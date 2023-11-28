cmake_minimum_required(VERSION 3.22)
project(bearWolfssl)

include_directories(./wolfssl/)
include_directories(./)
LINK_LIBRARIES(-lm)

set(CRYPT_SRC ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src)
aux_source_directory(${CRYPT_SRC} SRC)

set(SRC_LIST
	${SRC}
	)

add_library(wolfssl ${SRC_LIST})

target_sources(wolfssl
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/src/wolfio.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/src/internal.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/src/tls.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/src/ssl.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/src/keys.c
  )

#在user_settings.h定义-D OPENSSL_EXTRA 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D WOLFSSL_LIB -D WOLFSSL_CALLBACKS -D WOLFSSL_USER_SETTINGS -D CYASSL_USER_SETTINGS")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -D WOLFSSL_LIB -D WOLFSSL_CALLBACKS -D WOLFSSL_USER_SETTINGS -D CYASSL_USER_SETTINGS -fdata-sections -ffunction-sections -Wl,--gc-sections -s")


if(_CONFIG_INGENIC)
else(_CONFIG_INGENIC)

set(TM_SYS_TYPE "64-bit" CACHE INTERNAL "System Type")
if(${TM_SYS_TYPE} STREQUAL "32-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

elseif(${TM_SYS_TYPE} STREQUAL "64-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")

endif()
endif(_CONFIG_INGENIC)

add_executable(testclient
    ${CMAKE_CURRENT_SOURCE_DIR}/testclient.c)
target_link_libraries(testclient wolfssl -lm)
set_target_properties(testclient PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)

#没有生效，要手工复制
set_target_properties(wolfssl PROPERTIES LIBRARY_OUTPUT_DIRECTORY /mnt/corelooper/projects/master/t21.sdk/lib-5.4.0/uclibc/)
