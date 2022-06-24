# CMake版本最低要求
cmake_minimum_required(VERSION 3.0.0)

# 工程信息
project(onenet_thing_model VERSION 1.0.0)

# 指定编译器
#set(CMAKE_C_COMPILER "gcc")
#cmake_policy(SET CMP0065 NEW)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_VERBOSE_MAKEFILE OFF)

# 指定系统位数
set(TM_SYS_TYPE "32-bit" CACHE INTERNAL "System Type")


set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLCMAKE_C_FLAGS_DEBUGAGS} -g3")



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

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


# 包含头文件
include_directories(./ ./platforms/include ./onenet/thing_model/core ./onenet/thing_model/user)
include_directories(./commom ./services/ota/adapter ./services/ota/core ./services/ota/core/http)

#find_package(Threads REQUIRED)

# 添加子目录
add_subdirectory(platforms/linux)
#add_subdirectory(onenet/thing_model)
#add_subdirectory(onenet/utils)
#add_subdirectory(tools/cJSON)
add_subdirectory(tools/wolfssl)
#add_subdirectory(protocols/coap/er-coap-13)
#add_subdirectory(protocols/mqtt/paho-mqtt)
#add_subdirectory(protocols/http)
#add_subdirectory(security)
add_subdirectory(common)
#add_subdirectory(services/ota)

LINK_LIBRARIES(-lpthread -lm)

# 生成可执行文件
add_executable(one ./main.c)

# 添加链接库
#target_link_libraries(one Threads::Threads)
#target_link_libraries(one aio_platforms aio_onenet_tm aio_tools_cjson aio_onenet_utils aio_er_coap_13 aio_paho_mqtt aio_http)
#target_link_libraries(one aio_security aio_service_ota)




