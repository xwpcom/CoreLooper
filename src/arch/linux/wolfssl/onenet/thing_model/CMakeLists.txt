aux_source_directory(./core DIR_onenet_tm_SRCS)
aux_source_directory(./user DIR_onenet_tm_SRCS)

include_directories(../../protocols/mqtt/)
include_directories(../../protocols/coap/er-coap-13/)
include_directories(../../protocols/http/)
include_directories(../../platforms/include/)
include_directories(../../tools/cJSON)
include_directories(../../common/)
include_directories(../../)
include_directories(../utils/)
include_directories(./core/)
include_directories(./user/)
include_directories(../../tools/wolfssl/wolfssl-3.15.3/)

add_library (aio_onenet_tm ${DIR_onenet_tm_SRCS})

target_link_libraries(aio_onenet_tm aio_platforms)

if(${TM_SYS_TYPE} STREQUAL "32-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

elseif(${TM_SYS_TYPE} STREQUAL "64-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")

endif()
