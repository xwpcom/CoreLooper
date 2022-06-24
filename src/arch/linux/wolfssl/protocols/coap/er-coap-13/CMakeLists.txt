aux_source_directory(. DIR_er_coap_13_SRCS)

include_directories(../../../platforms/include/)
include_directories(../../../)

add_library (aio_er_coap_13 ${DIR_er_coap_13_SRCS})


if(${TM_SYS_TYPE} STREQUAL "32-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

elseif(${TM_SYS_TYPE} STREQUAL "64-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")

endif()