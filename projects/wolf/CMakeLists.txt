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

add_library(mini_wolfssl ${SRC_LIST})

#target_sources(mini_wolfssl
#  PRIVATE
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/hmac.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/hash.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/rsa.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/aes.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/sha.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/sha256.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/md5.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/asn.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/coding.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/random.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/wc_port.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/misc.c
#    ${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/integer.c
#	${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/wc_encrypt.c
#	${CMAKE_CURRENT_LIST_DIR}/wolfssl/wolfcrypt/src/logging.c
#
#	${CRYPT_SRC}/memory.c
#	${CRYPT_SRC}/dh.c
#	${CRYPT_SRC}/des3.c
#	${CRYPT_SRC}/arc4.c
#	${CRYPT_SRC}/pwdbased.c
#	${CRYPT_SRC}/md4.c
#	${CRYPT_SRC}/pkcs12.c
#	${CRYPT_SRC}/dsa.c
#	${CRYPT_SRC}/error.c
#	${CRYPT_SRC}/wolfmath.c
#  )

target_sources(mini_wolfssl
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
target_link_libraries(testclient mini_wolfssl -lm)
set_target_properties(testclient PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)
