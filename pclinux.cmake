# cmake -DCMAKE_TOOLCHAIN_FILE=../pclinux.cmake  ..

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
#SET(COMPILER_DIR  /opt/jco/mips-gcc472-glibc216-64bit/bin)
#SET(COMPILER_CROSS ${COMPILER_DIR}/mips-linux-uclibc-gnu-)
SET(CMAKE_C_COMPILER   ${COMPILER_CROSS}gcc)
SET(CMAKE_CXX_COMPILER ${COMPILER_CROSS}g++)
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D _CONFIG_INGENIC") 

#2022.06.27,为t21 linux引入wolfssl
SET(_ENABLE_WOLFSSL 1) #启用WOLFSSL
#SET(_ENABLE_WOLFSSL 0) #禁用WOLFSSL

if(_ENABLE_WOLFSSL)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D _CONFIG_WOLFSSL") 
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D _CONFIG_WOLFSSL") 
endif(_ENABLE_WOLFSSL)


# where is the target environment
#SET(CMAKE_FIND_ROOT_PATH  /opt/jco/mips-gcc472-glibc216-64bit /opt/jco/mips-gcc472-glibc216-64bit)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
