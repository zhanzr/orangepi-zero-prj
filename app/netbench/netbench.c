/*
  netbench - TCP throughput benchmark for the Orange Pi Zero's two
  network interfaces (Ethernet `end0`, WiFi `wld1`).

  The server runs ON THE BOARD and accepts multiple sequential
  connections; the client runs on the host and picks the interface by
  destination address (eth IP vs wifi IP). Connections are always
  initiated by the client (works through WSL2's NAT); the measured
  direction is selected per connection:

    default     client -> server  (host transmits, board receives)
    -r          server -> client  (board transmits, host receives)

  Server:  netbench -s [-p port]
  Client:  netbench -H <ip> [-p port] [-t seconds] [-r]

  Output: bytes transferred, elapsed seconds, Mbit/s.
*/
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_PORT 9001
#define BUF_SIZE     256 * 1024

static double now_sec(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void report(const char *dir, double bytes, double elapsed)
{
    printf("%-16s %10.0f bytes in %6.2f s  =  %8.2f Mbit/s\n",
           dir, bytes, elapsed, bytes * 8.0 / elapsed / 1e6);
}

/* pump `sec` seconds of data; mode: 1 = write, 0 = read. Returns bytes */
static double pump(int fd, int write_mode, double sec)
{
    static char buf[BUF_SIZE];
    double bytes = 0, t_end;
    ssize_t n;

    memset(buf, 0xA5, sizeof(buf));
    t_end = now_sec() + sec;

    while (now_sec() < t_end)
    {
        if (write_mode)
        {
            n = send(fd, buf, sizeof(buf), 0);
            if (n <= 0)
            {
                if (errno == EINTR)
                    continue;
                break;
            }
            bytes += n;
        }
        else
        {
            n = recv(fd, buf, sizeof(buf), 0);
            if (n < 0)
            {
                if (errno == EINTR)
                    continue;
                break;
            }
            if (n == 0)
                break;
            bytes += n;
        }
    }
    return bytes;
}

static int server(int port)
{
    int ls = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    struct sockaddr_in a;

    if (ls < 0)
    {
        perror("socket");
        return 1;
    }
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons((uint16_t)port);
    if (bind(ls, (struct sockaddr *)&a, sizeof(a)) < 0 ||
        listen(ls, 2) < 0)
    {
        perror("bind/listen");
        return 1;
    }
    printf("netbench server on 0.0.0.0:%d\n", port);

    for (;;)
    {
        int fd = accept(ls, NULL, NULL);
        char cmd;
        uint32_t sec_n;
        double sec, bytes, elapsed, t0;

        if (fd < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept");
            return 1;
        }
        if (read(fd, &cmd, 1) != 1 || read(fd, &sec_n, 4) != 4)
        {
            close(fd);
            continue;
        }
        sec = (double)ntohl(sec_n);
        t0 = now_sec();

        if (cmd == 'S')
        {
            /* server -> client (client measures) */
            bytes = pump(fd, 1, sec);
            elapsed = now_sec() - t0;
            report("server sent", bytes, elapsed);
        }
        else
        {
            /* client -> server (client measures; server also reports) */
            bytes = pump(fd, 0, sec);
            elapsed = now_sec() - t0;
            report("server recv", bytes, elapsed);
        }
        shutdown(fd, SHUT_WR);
        close(fd);
    }
}

static int client(const char *host, int port, double sec, int recv_mode)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a;
    char cmd = recv_mode ? 'S' : 'R';
    uint32_t sec_n = htonl((uint32_t)sec);
    double bytes, elapsed, t0;

    if (fd < 0)
    {
        perror("socket");
        return 1;
    }
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &a.sin_addr) != 1)
    {
        fprintf(stderr, "netbench: bad host %s\n", host);
        return 1;
    }
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0)
    {
        fprintf(stderr, "netbench: connect %s:%d: %s\n",
                host, port, strerror(errno));
        return 1;
    }

    if (write(fd, &cmd, 1) != 1 || write(fd, &sec_n, 4) != 4)
    {
        perror("handshake");
        return 1;
    }

    t0 = now_sec();
    /* recv_mode: server sends ('S' to server), client reads */
    bytes = pump(fd, recv_mode ? 0 : 1, sec);
    elapsed = now_sec() - t0;
    shutdown(fd, SHUT_WR);
    close(fd);

    report(recv_mode ? "board -> host" : "host -> board", bytes, elapsed);
    return 0;
}

static void usage(void)
{
    printf("usage: netbench -s [-p port]\n"
           "       netbench -H <ip> [-p port] [-t seconds] [-r]\n"
           "  -r: board -> host (server sends); default host -> board\n");
}

int main(int argc, char **argv)
{
    int port = DEFAULT_PORT, server_mode = 0, recv_mode = 0;
    double sec = 10;
    const char *host = NULL;

    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGPIPE, SIG_IGN);

    for (int i = 1; i < argc; i++)
    {
        int arg_err = 0;

        if (argv[i][0] == '-' && argv[i][1] && !argv[i][2])
        {
            switch (argv[i][1])
            {
            case 's': server_mode = 1; break;
            case 'r': recv_mode = 1; break;
            case 'p':
                if (i + 1 < argc)
                    port = atoi(argv[++i]);
                else
                    arg_err = 1;
                break;
            case 'H':
                if (i + 1 < argc)
                    host = argv[++i];
                else
                    arg_err = 1;
                break;
            case 't':
                if (i + 1 < argc)
                    sec = atof(argv[++i]);
                else
                    arg_err = 1;
                break;
            default: arg_err = 1;
            }
        }
        else
            arg_err = 1;

        if (arg_err)
        {
            usage();
            return 1;
        }
    }

    if (server_mode)
        return server(port);
    if (!host)
    {
        usage();
        return 1;
    }
    return client(host, port, sec, recv_mode);
}
