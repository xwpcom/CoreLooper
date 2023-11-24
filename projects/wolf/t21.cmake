# cmake -DCMAKE_TOOLCHAIN_FILE=../t21.cmake  ..


# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Release")
SET(_CONFIG_INGENIC 1)

# where is the target environment
#XiongWanPing 2021.06.02,注意t21要用gcc5.4.0,和捷高保持一致
# t20要用gcc4.7.2
#SET(CMAKE_FIND_ROOT_PATH  /opt/jco/mips-gcc472-glibc216-64bit /opt/jco/mips-gcc472-glibc216-64bit)
#SET(COMPILER_DIR  /opt/jco/mips-gcc472-glibc216-64bit/bin)
SET(CMAKE_FIND_ROOT_PATH  /opt/jco/mips-gcc540-glibc222-64bit-r3.3.0 /opt/jco/mips-gcc540-glibc222-64bit-r3.3.0)
SET(COMPILER_DIR  /opt/jco/mips-gcc540-glibc222-64bit-r3.3.0/bin)
SET(COMPILER_CROSS ${COMPILER_DIR}/mips-linux-uclibc-gnu-)
SET(CMAKE_C_COMPILER   ${COMPILER_CROSS}gcc)
SET(CMAKE_CXX_COMPILER ${COMPILER_CROSS}g++)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_OPENSSLEXTRA=1")
