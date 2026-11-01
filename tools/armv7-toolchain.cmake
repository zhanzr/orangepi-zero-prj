set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# -mfp16-format=none: Cortex-A7 has no FP16 storage ext; keeps gcc's
# __ARM_FP fp16-storage bit clear so NEON fp16 intrinsics stay out
set(CMAKE_C_FLAGS_INIT   "-march=armv7-a+neon-vfpv4 -mfloat-abi=hard -mfp16-format=none")
set(CMAKE_CXX_FLAGS_INIT "-march=armv7-a+neon-vfpv4 -mfloat-abi=hard -mfp16-format=none")

set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
