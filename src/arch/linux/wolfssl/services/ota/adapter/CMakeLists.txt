aux_source_directory(. DIR_adapter_SRCS)

include_directories(../../platforms/include/)

add_library (ota_adapter ${DIR_adapter_SRCS})

