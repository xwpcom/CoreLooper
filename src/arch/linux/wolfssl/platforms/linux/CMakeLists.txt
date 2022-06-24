aux_source_directory(. DIR_platforms_SRCS)

include_directories(../include/)
include_directories(../../common/)
include_directories(../../)

LINK_LIBRARIES(-lpthread)

add_library (aio_platforms ${DIR_platforms_SRCS})
target_link_libraries(aio_platforms aio_common)

if(_CONFIG_INGENIC)
else(_CONFIG_INGENIC)

if(${TM_SYS_TYPE} STREQUAL "32-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

elseif(${TM_SYS_TYPE} STREQUAL "64-bit")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")

endif()
endif(_CONFIG_INGENIC)