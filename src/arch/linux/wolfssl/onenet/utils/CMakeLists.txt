aux_source_directory(. DIR_onenet_utils_SRCS)

include_directories(../../platforms/include/)
include_directories(../../)
include_directories(../../tools/wolfssl/wolfssl-3.15.3/)

add_library (aio_onenet_utils ${DIR_onenet_utils_SRCS})

target_link_libraries(aio_onenet_utils aio_tools_wolfssl)
target_link_libraries(aio_onenet_utils aio_platforms)

if(${TM_SYS_TYPE} STREQUAL "32-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

elseif(${TM_SYS_TYPE} STREQUAL "64-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")

endif()
