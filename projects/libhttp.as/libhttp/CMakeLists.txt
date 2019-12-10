# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

SET(TARGET_NAME http)

SET(SRC_DIR ${PROJECT_SOURCE_DIR}/../../libhttp/src)

SET(CORELOOPER_INC ${PROJECT_SOURCE_DIR}/../../../include)
SET(SRC_INC ${CORELOOPER_INC}/libhttp)

include_directories(
        ${PROJECT_SOURCE_DIR}/../../../include/libcrypt/
        ${PROJECT_SOURCE_DIR}/../../libhttp/
        ${PROJECT_SOURCE_DIR}/../../
        ${SRC_INC}
        ${CORELOOPER_INC}
        ${CORELOOPER_INC}/core
        ${CORELOOPER_INC}/core/base
)

add_library( # Sets the name of the library.
        ${TARGET_NAME}

         # Sets the library as a shared library.
         SHARED

         # Provides a relative path to your source file(s).
        ${SRC_DIR}/ajaxcommandhandler.cpp
        ${SRC_DIR}/ajaxhandler.cpp
        ${SRC_DIR}/ajax_listajaxcommand.cpp
        ${SRC_DIR}/ajax_listajaxcommand.h
        ${SRC_DIR}/ajax_proc.cpp
        ${SRC_DIR}/ajax_proc.h
        ${SRC_DIR}/httpackheader.cpp
        ${SRC_DIR}/httpackparser.cpp
        ${SRC_DIR}/httpformfield.cpp
        ${SRC_DIR}/httpformfield_file.cpp
        ${SRC_DIR}/httphandler.cpp
        ${SRC_DIR}/httpheader.cpp
        ${SRC_DIR}/httppostcommandhandler.cpp
        ${SRC_DIR}/httpposthandler.cpp
        ${SRC_DIR}/httppostparser.cpp
        ${SRC_DIR}/httprequest.cpp
        ${SRC_DIR}/httprequesthandler.cpp
        ${SRC_DIR}/httprequesthandler_ajax.cpp
        ${SRC_DIR}/httprequesthandler_cgi.cpp
        ${SRC_DIR}/httprequesthandler_file.cpp
        ${SRC_DIR}/httpserver.cpp
        ${SRC_DIR}/httptool.cpp
        ${SRC_DIR}/posthandler_uploadpicture.cpp
        ${SRC_DIR}/posthandler_uploadvideo.cpp
        ${SRC_DIR}/procmanager.cpp
        ${SRC_DIR}/textparser.cpp
        ${SRC_DIR}/userinfo.cpp
        ${SRC_DIR}/userman.cpp
        ${SRC_DIR}/websockethandler.cpp
        ${SRC_DIR}/websocketsplitter.cpp
        ${SRC_DIR}/wshandler.cpp
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
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ${TARGET_NAME}

       # Links the target library to the log library
       # included in the NDK.
       ${log-lib}

        ${PROJECT_SOURCE_DIR}/../../libcrypt.as/libcrypt/build/intermediates/cmake/${CMAKE_BUILD_TYPE}/obj/${ANDROID_ABI}/libcrypt.so
        ${PROJECT_SOURCE_DIR}/../../libcorelooper.as/libcorelooper/build/intermediates/cmake/${CMAKE_BUILD_TYPE}/obj/${ANDROID_ABI}/libcorelooper.so
        )