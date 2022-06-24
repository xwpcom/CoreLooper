aux_source_directory(. DIR_paho_mqtt_SRCS)

include_directories(../../../platforms/include/)
include_directories(../../../security/)
include_directories(../../../common/)
include_directories(../../../)
include_directories(../)

add_library (aio_paho_mqtt ${DIR_paho_mqtt_SRCS})

target_link_libraries(aio_paho_mqtt aio_common)
target_link_libraries(aio_paho_mqtt aio_security)

if(${TM_SYS_TYPE} STREQUAL "32-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

elseif(${TM_SYS_TYPE} STREQUAL "64-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")

endif()