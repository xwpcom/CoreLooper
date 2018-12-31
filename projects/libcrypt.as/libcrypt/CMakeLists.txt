# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
SET(TARGET_NAME crypt)

SET(SRC_DIR ${PROJECT_SOURCE_DIR}/../../libcrypt/src)

SET(CORELOOPER_INC ${PROJECT_SOURCE_DIR}/../../../include)
SET(SRC_INC ${CORELOOPER_INC}/libcrypt)

include_directories(
        ${PROJECT_SOURCE_DIR}/../../libcrypt/
        ${SRC_INC}
        ${SRC_INC}/ssl
        ${CORELOOPER_INC}
        ${CORELOOPER_INC}/core
        ${CORELOOPER_INC}/core/base
)


add_library( # Sets the name of the library.
    ${TARGET_NAME}

    # Sets the library as a shared library.
    SHARED

    ${SRC_DIR}/ssl/arc4.cpp
    ${SRC_DIR}/ssl/base64.cpp
    ${SRC_DIR}/ssl/bignum.cpp
    ${SRC_DIR}/ssl/havege.cpp
    ${SRC_DIR}/ssl/rsa.cpp
    ${SRC_DIR}/ssl/sha1.cpp
    ${SRC_DIR}/ssl/ssl_cli.cpp
    ${SRC_DIR}/ssl/ssl_tls.cpp
    ${SRC_DIR}/ssl/x509parse.cpp
    ${SRC_DIR}/aes.cpp
    ${SRC_DIR}/base64ex.cpp
    ${SRC_DIR}/bcd.cpp
    ${SRC_DIR}/crc16.cpp
    ${SRC_DIR}/crypthelper.cpp
    ${SRC_DIR}/digestaccessauth.cpp
    ${SRC_DIR}/md5.cpp
    ${SRC_DIR}/md5ex.cpp
    ${SRC_DIR}/tea.cpp
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ${TARGET_NAME}
        
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${PROJECT_SOURCE_DIR}/../../libcorelooper.as/libcorelooper/build/intermediates/cmake/${CMAKE_BUILD_TYPE}/obj/${ANDROID_ABI}/libcorelooper.so
        )