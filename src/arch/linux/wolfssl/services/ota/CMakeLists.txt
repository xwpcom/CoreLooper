include_directories(./http/)

add_subdirectory(./adapter)
add_subdirectory(./core)
add_subdirectory(./core/http)

add_library(aio_service_ota INTERFACE)
target_link_libraries(aio_service_ota INTERFACE ota_core ota_adapter ota_http_parse)