add_library(aio_tools_wolfssl "")

include_directories(./port/)
include_directories(./wolfssl-3.15.3/)
include_directories(../../platforms/include/)
include_directories(../../)

LINK_LIBRARIES(-lm)

target_sources(aio_tools_wolfssl
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/hmac.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/hash.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/rsa.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/aes.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/sha.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/sha256.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/md5.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/asn.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/coding.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/random.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/wc_port.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/misc.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/integer.c
	${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/wc_encrypt.c
	${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/wolfcrypt/src/logging.c
  )

target_sources(aio_tools_wolfssl
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/src/wolfio.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/src/internal.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/src/tls.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/src/ssl.c
    ${CMAKE_CURRENT_LIST_DIR}/wolfssl-3.15.3/src/keys.c
  )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -D OPENSSL_EXTRA")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D OPENSSL_EXTRA")
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
