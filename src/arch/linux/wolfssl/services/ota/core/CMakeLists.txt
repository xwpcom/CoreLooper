aux_source_directory(. DIR_core_SRCS)

include_directories(./http/)
include_directories(../adapter/)
include_directories(../../../tools/cJSON/)
include_directories(../../../onenet/utils/)
include_directories(../../../platforms/include/)

add_library (ota_core ${DIR_core_SRCS})
target_link_libraries(ota_core ota_adapter ota_http_parse aio_platforms)
