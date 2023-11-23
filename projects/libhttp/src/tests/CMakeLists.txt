add_executable(http_unittest http_unittest.cpp)
SET(SRC_DIR ${PROJECT_SOURCE_DIR})

#message(${CMAKE_CURRENT_LIST_DIR})

#SET(WOLFSSL_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../src/arch/linux/wolfssl/tools/wolfssl/wolfssl-3.15.3)
#message (${WOLFSSL_DIR})
#INCLUDE_DIRECTORIES(${WOLFSSL_DIR})
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/../../../../src/arch/linux/wolfssl/platforms/include)
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/../../../../src/arch/linux/wolfssl)
SET(WOLFSSL_DIR ${CMAKE_CURRENT_LIST_DIR}/../study/wolfssl.11.22)
INCLUDE_DIRECTORIES(${WOLFSSL_DIR})

target_link_libraries(http_unittest crypt http crypt corelooper 
#aio_tools_wolfssl aio_platforms
wolfssl
crypt
)

set_target_properties(http_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)
