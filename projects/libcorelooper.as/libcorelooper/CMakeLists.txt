# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

SET(TARGET_NAME corelooper)
SET(SRC_DIR ${PROJECT_SOURCE_DIR}/../../../src)
SET(CORE_DIR ${SRC_DIR}/core)
SET(SRC_INC ${PROJECT_SOURCE_DIR}/../../../include)
SET(CORE_INC ${SRC_INC}/core)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../libcorelooper
    ${SRC_INC}
    ${SRC_INC}/arch/android
    ${SRC_INC}/arch/linux
    ${CORE_INC}
    ${CORE_INC}/base
    ${CORE_INC}/file
    ${CORE_INC}/looper
    ${CORE_INC}/net
    ${CORE_INC}/string
    ${CORE_INC}/thread
)

add_library( # Sets the name of the library.
             ${TARGET_NAME}

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp
${SRC_DIR}/arch/android/jni_entry.cpp
${SRC_DIR}/arch/android/jnihelper.cpp
${SRC_DIR}/arch/android/jnilooper.cpp
${SRC_DIR}/arch/android/jnimainlooper.cpp
${SRC_DIR}/arch/linux/linuxsignal.cpp
${SRC_DIR}/arch/linux/looper_linux.cpp
${SRC_DIR}/arch/linux/serialport_linux.cpp
${SRC_DIR}/arch/linux/tcpclient_linux.cpp
${SRC_DIR}/arch/linux/tcpserver_linux.cpp
${SRC_DIR}/arch/linux/udpclient_linux.cpp
${SRC_DIR}/arch/linux/udpserver_linux.cpp

${CORE_DIR}/base/bytebuffer.cpp
${CORE_DIR}/base/bytetool.cpp
${CORE_DIR}/base/dt.cpp
${CORE_DIR}/base/log.cpp
${CORE_DIR}/base/namevalue.cpp
${CORE_DIR}/base/object.cpp
${CORE_DIR}/base/shelltool.cpp
${CORE_DIR}/base/stringtool.cpp
${CORE_DIR}/base/tickdumper.cpp

${CORE_DIR}/file/dumpfile.cpp
${CORE_DIR}/file/file.cpp
${CORE_DIR}/file/filefinder.cpp
${CORE_DIR}/file/inifile.cpp
${CORE_DIR}/file/virtualfolder.cpp

${CORE_DIR}/looper/asynctask.cpp
${CORE_DIR}/looper/asynctasklooper.cpp
${CORE_DIR}/looper/handler.cpp
${CORE_DIR}/looper/handlerinternaldata.cpp
${CORE_DIR}/looper/looper.cpp
${CORE_DIR}/looper/looperimpl.cpp
${CORE_DIR}/looper/looperinternaldata.cpp
${CORE_DIR}/looper/procnode.cpp
${CORE_DIR}/looper/timermanager.cpp

${CORE_DIR}/net/channel.cpp
${CORE_DIR}/net/dnslooper.cpp
${CORE_DIR}/net/nettool.cpp
${CORE_DIR}/net/simpleconnect.cpp
${CORE_DIR}/net/socktool.cpp
${CORE_DIR}/net/speedcounter.cpp
${CORE_DIR}/net/tcpclient.cpp
${CORE_DIR}/net/tcpserver.cpp
${CORE_DIR}/net/udpserver.cpp
${CORE_DIR}/net/urltool.cpp
${CORE_DIR}/protocol/ctp/commontextprotocol.cpp
${CORE_DIR}/protocol/ctp/commontextprotocol_impl.cpp
${CORE_DIR}/protocol/ctp/ctp.template/ctpclient.cpp
${CORE_DIR}/protocol/ctp/ctp.template/ctphandler.cpp
${CORE_DIR}/protocol/ctp/ctp.template/ctpserver.cpp
${CORE_DIR}/protocol/ctp/commontextprotocol2.cpp
${CORE_DIR}/protocol/ctp/commontextprotocol_impl2.cpp
${CORE_DIR}/protocol/ctp/ctp.template/ctpclient2.cpp
${CORE_DIR}/protocol/ctp/ctp.template/ctphandler2.cpp
${CORE_DIR}/protocol/ctp/ctp.template/ctpserver2.cpp
${CORE_DIR}/string/textprotocol.cpp
${CORE_DIR}/string/textseparator.cpp
${CORE_DIR}/string/xmlstring.cpp
${CORE_DIR}/string/stringparam.cpp
${CORE_DIR}/thread/autolock.cpp
${CORE_DIR}/thread/criticalsection.cpp
${CORE_DIR}/thread/event.cpp

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
message(WARNING "libcorelooper.as CMAKE_BUILD_TYPE =" ${CMAKE_BUILD_TYPE} )
target_link_libraries( # Specifies the target library.
                       ${TARGET_NAME}

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )