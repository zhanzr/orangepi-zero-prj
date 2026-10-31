# Orange Pi Zero cross toolchain environment
# source from anywhere:  . ~/opz-dev/env.sh
export OPZ_CROSS=arm-linux-gnueabihf-
export OPZ_BOARD=opz   # ssh alias, see ~/.ssh/config
echo "opz toolchain ready: ${OPZ_CROSS}gcc ($( ${OPZ_CROSS}gcc -dumpversion ))"
