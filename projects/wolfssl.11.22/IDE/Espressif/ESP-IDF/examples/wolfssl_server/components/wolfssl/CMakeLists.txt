#
#  Copyright (C) 2006-2023 wolfSSL Inc.
#
#  This file is part of wolfSSL.
#
#  wolfSSL is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  wolfSSL is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
#
# cmake for wolfssl Espressif projects
#
# Version 5.6.3.001
#
# See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html
#

cmake_minimum_required(VERSION 3.16)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")
set(CMAKE_CURRENT_SOURCE_DIR ".")
set(COMPONENT_REQUIRES lwip) # we typically don't need lwip directly in wolfssl component

# COMPONENT_NAME = wolfssl
# The component name is the directory name. "No feature to change this".
# See https://github.com/espressif/esp-idf/issues/8978#issuecomment-1129892685

# set the root of wolfSSL:
#   set(WOLFSSL_ROOT  "C:/some path/with/spaces")
#   set(WOLFSSL_ROOT  "c:/workspace/wolfssl-gojimmypi")
#   set(WOLFSSL_ROOT  "/mnt/c/some path/with/spaces")
#   or use this logic to assign value from Environment Variable WOLFSSL_ROOT,
#   or assume this is an example 7 subdirectories below:

# We are typically in [root]/IDE/Espressif/ESP-IDF/examples/wolfssl_test/components/wolfssl
# The root of wolfSSL is 7 directories up from here:

if(CMAKE_BUILD_EARLY_EXPANSION)
    message(STATUS "wolfssl component CMAKE_BUILD_EARLY_EXPANSION:")
    idf_component_register(
                            REQUIRES "${COMPONENT_REQUIRES}"
                            PRIV_REQUIRES # esp_hw_support
                                          esp_timer
                                          driver # this will typically only be needed for wolfSSL benchmark
                           )

else()
    # not CMAKE_BUILD_EARLY_EXPANSION
    message(STATUS "************************************************************************************************")
    message(STATUS "wolfssl component config:")
    message(STATUS "************************************************************************************************")

    # TODO
    if(WIN32)
        # Windows-specific configuration here
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_CMAKE_SYSTEM_NAME_WINDOWS")
        message("Detected Windows")
    endif()
    if(CMAKE_HOST_UNIX)
        message("Detected UNIX")
    endif()
    if(APPLE)
        message("Detected APPLE")
    endif()
    if(CMAKE_HOST_UNIX AND (NOT APPLE) AND EXISTS "/proc/sys/fs/binfmt_misc/WSLInterop")
        # Windows-specific configuration here
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_CMAKE_SYSTEM_NAME_WSL")
        message("Detected WSL")
    endif()
    if(CMAKE_HOST_UNIX AND (NOT APPLE) AND (NOT WIN32))
        # Windows-specific configuration here
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_CMAKE_SYSTEM_NAME_LINUX")
        message("Detected Linux")
    endif()
    if(APPLE)
        # Windows-specific configuration here
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_CMAKE_SYSTEM_NAME_APPLE")
        message("Detected Apple")
    endif()

    # Check to see if we're already in wolfssl, and only if WOLFSSL_ROOT not specified
    if ("${WOLFSSL_ROOT}" STREQUAL "")
        # wolfssl examples are 7 directories deep from wolfssl repo root
        #                        1  2  3  4  5  6  7
        set(THIS_RELATIVE_PATH "../../../../../../..")
        get_filename_component(THIS_SEARCH_PATH  "${THIS_RELATIVE_PATH}" ABSOLUTE)
        message(STATUS "Searching in path = ${THIS_SEARCH_PATH}")

        if (EXISTS "${THIS_SEARCH_PATH}/wolfcrypt/src")
            # we're already in wolfssl examples!
            get_filename_component(WOLFSSL_ROOT  "${THIS_SEARCH_PATH}" ABSOLUTE)
            message(STATUS "Using wolfSSL example with root ${WOLFSSL_ROOT}")
        else()
            # We're in some other repo such as wolfssh, so we'll search for an
            # adjacent-level directory for wolfssl. (8 directories up, then down one)
            #
            # For example wolfSSL examples:
            #   C:\workspace\wolfssl-gojimmypi\IDE\Espressif\ESP-IDF\examples\wolfssl_benchmark\components\wolfssl
            #
            # For example wolfSSH examples:
            #   C:\workspace\wolfssh-gojimmypi\ide\Espressif\ESP-IDF\examples\wolfssh_benchmark\components\wolfssl
            #
            #                        1  2  3  4  5  6  7  8
            set(THIS_RELATIVE_PATH "../../../../../../../..")
            get_filename_component(THIS_SEARCH_PATH  "${THIS_RELATIVE_PATH}" ABSOLUTE)
            message(STATUS "Searching next in path = ${THIS_SEARCH_PATH}")
        endif()
    endif()

    # search other possible locations
    if ("${WOLFSSL_ROOT}" STREQUAL "")
        # there's not a hard-coded WOLFSSL_ROOT value above, so let's see if we can find it.
        if( "$ENV{WOLFSSL_ROOT}" STREQUAL "" )
            message(STATUS "Environment Variable WOLFSSL_ROOT not set. Will search common locations.")

            message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
            get_filename_component(THIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
            message(STATUS "THIS_DIR = ${THIS_DIR}")

            # find the user name to search for possible "wolfssl-username"
            message(STATUS "USERNAME = $ENV{USERNAME}")
            if(  "$ENV{USER}" STREQUAL "" ) # the bash user
                if(  "$ENV{USERNAME}" STREQUAL "" ) # the Windows user
                    message(STATUS "could not find USER or USERNAME")
                else()
                    # the bash user is not blank, so we'll use it.
                    set(THIS_USER "$ENV{USERNAME}")
                endif()
            else()
                # the bash user is not blank, so we'll use it.
               set(THIS_USER "$ENV{USER}")
            endif()
            message(STATUS "THIS_USER = ${THIS_USER}")

            # This same makefile is used for both the wolfssl component, and other
            # components that may depend on wolfssl, such as wolfssh. Therefore
            # we need to determine if this makefile is in the wolfssl repo, or
            # some other repo.

            if(  "{THIS_USER}" STREQUAL "" )
                # This is highly unusual to not find a user name.
                # In this case, we'll just search for a "wolfssl" directory:
                message(STATUS "No username found!")
                get_filename_component(WOLFSSL_ROOT  "${THIS_RELATIVE_PATH}/wolfssl" ABSOLUTE)
            else()
                # We found an environment USER name!
                # The first place to look for wolfssl will be in a user-clone called "wolfssl-[username]"
                message(STATUS "Using [THIS_USER = ${THIS_USER}] to see if there's a [relative path]/wolfssl-${THIS_USER} directory.")
                get_filename_component(WOLFSSL_ROOT  "${THIS_RELATIVE_PATH}/wolfssl-${THIS_USER}" ABSOLUTE)

                if( EXISTS "${WOLFSSL_ROOT}" )
                    message(STATUS "Found wolfssl in user-suffix ${WOLFSSL_ROOT}")
                else()
                    # If there's not a user-clone called "wolfssl-[username]",
                    # perhaps there's simply a git clone called "wolfssl"?
                    message(STATUS "Did not find wolfssl-${THIS_USER}; continuing search...")
                    get_filename_component(WOLFSSL_ROOT  "${THIS_RELATIVE_PATH}/wolfssl" ABSOLUTE)

                    if( EXISTS "${WOLFSSL_ROOT}" )
                        message(STATUS "Found wolfssl in standard ${WOLFSSL_ROOT}")
                    else()
                        # Things are looking pretty bleak. We'll likely not be able to compile.
                        message(STATUS "Did not find wolfssl in ${WOLFSSL_ROOT}")
                    endif()
                endif()
            endif()

        else()
            # there's an environment variable, so use it.
            set(WOLFSSL_ROOT "$ENV{WOLFSSL_ROOT}")

            if( EXISTS "${WOLFSSL_ROOT}" )
                get_filename_component(WOLFSSL_ROOT  "$ENV{WOLFSSL_ROOT}" ABSOLUTE)
                message(STATUS "Found WOLFSSL_ROOT via Environment Variable:")
            else()
                message(FATAL_ERROR "WOLFSSL_ROOT Environment Variable defined, but path not found:")
                message(STATUS "$ENV{WOLFSSL_ROOT}")
            endif()
        endif()
        # end of search for wolfssl component root
    else()
        # There's already a value assigned; we won't search for anything else.
        message(STATUS "Found user-specified WOLFSSL_ROOT value.")
    endif() # WOLFSSL_ROOT user defined

    # After all the logic above, does our WOLFSSL_ROOT actually exist?
    if( EXISTS "${WOLFSSL_ROOT}" )
        message(STATUS "WOLFSSL_ROOT = ${WOLFSSL_ROOT}")
    else()
        # Abort. We need wolfssl _somewhere_.
        message(FATAL_ERROR "Could not find wolfssl in ${WOLFSSL_ROOT}. Try setting environment variable or git clone.")
    endif()


    set(INCLUDE_PATH ${WOLFSSL_ROOT})

    set(COMPONENT_SRCDIRS "\"${WOLFSSL_ROOT}/src/\""
                          "\"${WOLFSSL_ROOT}/wolfcrypt/src\""
                          "\"${WOLFSSL_ROOT}/wolfcrypt/src/port/Espressif\""
                          "\"${WOLFSSL_ROOT}/wolfcrypt/src/port/atmel\""
                          # TODO: Make this a univeral makefile that detects if bechmark / test needed
                          # Sometimes problematic with SM; consider gating detection.
                          #"\"${WOLFSSL_ROOT}/wolfcrypt/benchmark\"" # the benchmark application
                          #"\"${WOLFSSL_ROOT}/wolfcrypt/test\"" # the test application
       ) # COMPONENT_SRCDIRS
    message(STATUS "This COMPONENT_SRCDIRS = ${COMPONENT_SRCDIRS}")

    set(WOLFSSL_PROJECT_DIR "${CMAKE_HOME_DIRECTORY}/components/wolfssl")
    add_definitions(-DWOLFSSL_USER_SETTINGS_DIR="${WOLFSSL_PROJECT_DIR}/include/user_settings.h")


    # Espressif may take several passes through this makefile. Check to see if we found IDF
    string(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "" WOLFSSL_FOUND_IDF)

    # get a list of all wolfcrypt assembly files; we'll exclude them as they don't target Xtensa
    file(GLOB EXCLUDE_ASM *.S)
    file(GLOB_RECURSE EXCLUDE_ASM ${CMAKE_SOURCE_DIR} "${WOLFSSL_ROOT}/wolfcrypt/src/*.S")

    message(STATUS "IDF_PATH = $ENV{IDF_PATH}")
    message(STATUS "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
    message(STATUS "EXCLUDE_ASM = ${EXCLUDE_ASM}")

    #
    # Check to see if there's both a local copy and EDP-IDF copy of the wolfssl and/or wolfssh components.
    #
    if( EXISTS "${WOLFSSL_PROJECT_DIR}" AND EXISTS "$ENV{IDF_PATH}/components/wolfssl/" )
        #
        # wolfSSL found in both ESP-IDF and local project - needs to be resolved by user
        #
        message(STATUS "")
        message(STATUS "**************************************************************************************")
        message(STATUS "")
        message(STATUS "Error: Found components/wolfssl in both local project and IDF_PATH")
        message(STATUS "")
        message(STATUS "To proceed: ")
        message(STATUS "")
        message(STATUS "Remove either the local project component: ${WOLFSSL_PROJECT_DIR} ")
        message(STATUS "or the Espressif shared component installed at: $ENV{IDF_PATH}/components/wolfssl/ ")
        message(STATUS "")
        message(FATAL_ERROR "Please use wolfSSL in either local project or Espressif components, but not both.")
        message(STATUS "")
        message(STATUS "**************************************************************************************")
        message(STATUS "")

        # Optional: if you change the above FATAL_ERROR to STATUS you can warn at runtime with this macro definition:
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_MULTI_INSTALL_WARNING")

    else()
        if( EXISTS "$ENV{IDF_PATH}/components/wolfssl/" )
            #
            # wolfSSL found in ESP-IDF components and is assumed to be already configured in user_settings.h via setup.
            #
            message(STATUS "")
            message(STATUS "Using components/wolfssl in IDF_PATH = $ENV{IDF_PATH}")
            message(STATUS "")
        else()
            #
            # wolfSSL is not an ESP-IDF component.
            # We need to now determine if it is local and if so if it is part of the wolfSSL repo,
            # or if  wolfSSL is simply installed as a local component.
            #

            if( EXISTS "${WOLFSSL_PROJECT_DIR}" )
                #
                # wolfSSL found in local project.
                #
                if( EXISTS "${WOLFSSL_PROJECT_DIR}/wolfcrypt/" )
                    message(STATUS "")
                    message(STATUS "Using installed project ./components/wolfssl in CMAKE_HOME_DIRECTORY = ${CMAKE_HOME_DIRECTORY}")
                    message(STATUS "")
                    #
                    # Note we already checked above and confirmed there's not another wolfSSL installed in the ESP-IDF components.
                    #
                    # We won't do anything else here, as it will be assumed the original install completed successfully.
                    #
                else() # full wolfSSL not installed in local project
                    #
                    # This is the developer repo mode. wolfSSL will be assumed to be not installed to ESP-IDF nor local project
                    # In this configuration, we are likely running a wolfSSL example found directly in the repo.
                    #
                    message(STATUS "")
                    message(STATUS "Using developer repo ./components/wolfssl in CMAKE_HOME_DIRECTORY = ${CMAKE_HOME_DIRECTORY}")
                    message(STATUS "")

                    message(STATUS "************************************************************************************************")
                    # When in developer mode, we are typically running wolfSSL examples such as benchmark or test directories.
                    # However, the as-cloned or distributed wolfSSL does not have the ./include/ directory, so we'll add it as needed.
                    #
                    # first check if there's a [root]/include/user_settings.h
                    if( EXISTS "${WOLFSSL_ROOT}/include/user_settings.h" )
                        message(FATAL_ERROR "Found stray wolfSSL user_settings.h in "
                                            "${WOLFSSL_ROOT}/include/user_settings.h "
                                            " (please move it to ${WOLFSSL_PROJECT_DIR}/include/user_settings.h )")
                    else()
                        # we won't overwrite an existing user settings file, just note that we already have one:
                        if( EXISTS "${WOLFSSL_PROJECT_DIR}/include/user_settings.h" )
                            message(STATUS "Using existing wolfSSL user_settings.h in "
                                           "${WOLFSSL_PROJECT_DIR}/include/user_settings.h")
                        else()
                            message(STATUS "Installing wolfSSL user_settings.h to "
                                           "${WOLFSSL_PROJECT_DIR}/include/user_settings.h")
                            file(COPY "${WOLFSSL_ROOT}/IDE/Espressif/ESP-IDF/user_settings.h"
                            DESTINATION "${CMAKE_HOME_DIRECTORY}/wolfssl/include/")
                        endif()
                    endif() # user_settings.h

                    # next check if there's a [root]/include/config.h
                    if( EXISTS "${WOLFSSL_ROOT}/include/config.h" )
                        message(FATAL_ERROR "Found stray wolfSSL config.h in "
                                            "${WOLFSSL_ROOT}/include/config.h "
                                            " (please move it to ${WOLFSSL_PROJECT_DIR}/include/config.h )")
                    else()
                        # we won't overwrite an existing user settings file, just note that we already have one:
                        if( EXISTS "${WOLFSSL_PROJECT_DIR}/include/config.h" )
                            message(STATUS "Using existing wolfSSL config.h           ${WOLFSSL_PROJECT_DIR}/include/config.h")
                        else()
                            message(STATUS "Installing wolfSSL config.h to            ${WOLFSSL_PROJECT_DIR}/include/config.h")
                            file(COPY   "${WOLFSSL_ROOT}/IDE/Espressif/ESP-IDF/dummy_config_h" DESTINATION "${WOLFSSL_PROJECT_DIR}/include/")
                            file(RENAME "${WOLFSSL_PROJECT_DIR}/include/dummy_config_h" "${WOLFSSL_PROJECT_DIR}/include/config.h")
                        endif() # Project config.h
                    endif() # WOLFSSL_ROOT config.h
                    message(STATUS "************************************************************************************************")
                    message(STATUS "")
                endif()

            else()
                # we did not find a ./components/wolfssl/include/ directory from this pass of cmake.
                if($WOLFSSL_FOUND_IDF)
                    message(STATUS "")
                    message(STATUS "WARNING: wolfSSL not found.")
                    message(STATUS "")
                else()
                    # probably needs to be re-parsed by Espressif
                    message(STATUS "wolfSSL found IDF. Project Source:${PROJECT_SOURCE_DIR}")
                endif() # else we have not found ESP-IDF yet
            endif() # else not a local wolfSSL component

        endif() #else not an ESP-IDF component
    endif() # else not local copy and EDP-IDF wolfSSL


    # RTOS_IDF_PATH is typically:
    # "/Users/{username}/Desktop/esp-idf/components/freertos/include/freertos"
    # depending on the environment, we may need to swap backslashes with forward slashes
    string(REPLACE "\\" "/" RTOS_IDF_PATH "$ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/include/freertos")

    string(REPLACE "\\" "/" WOLFSSL_ROOT ${WOLFSSL_ROOT})

    if(IS_DIRECTORY "${RTOS_IDF_PATH}")
        message(STATUS "Found current RTOS path: ${RTOS_IDF_PATH}")
    else()
        # ESP-IDF prior version 4.4x has a different RTOS directory structure
        string(REPLACE "\\" "/" RTOS_IDF_PATH "$ENV{IDF_PATH}/components/freertos/include/freertos")
        if(IS_DIRECTORY "${RTOS_IDF_PATH}")
            message(STATUS "Found legacy RTOS path: ${RTOS_IDF_PATH}")
        else()
            message(STATUS "Could not find RTOS path")
        endif()
    endif()


    set(COMPONENT_ADD_INCLUDEDIRS
        "./include" # this is the location of wolfssl user_settings.h
        "\"${WOLFSSL_ROOT}/\""
        "\"${WOLFSSL_ROOT}/wolfssl/\""
        "\"${WOLFSSL_ROOT}/wolfssl/wolfcrypt/\""
        "\"${RTOS_IDF_PATH}/\""
        )


    if(IS_DIRECTORY ${IDF_PATH}/components/cryptoauthlib)
        list(APPEND COMPONENT_ADD_INCLUDEDIRS "../cryptoauthlib/lib")
    endif()

    list(APPEND COMPONENT_ADD_INCLUDEDIRS "\"${WOLFSSL_ROOT}/wolfssl/\"")
    list(APPEND COMPONENT_ADD_INCLUDEDIRS "\"${WOLFSSL_ROOT}/wolfssl/wolfcrypt/\"")



    set(COMPONENT_SRCEXCLUDE
        "\"${WOLFSSL_ROOT}/src/bio.c\""
        "\"${WOLFSSL_ROOT}/src/conf.c\""
        "\"${WOLFSSL_ROOT}/src/misc.c\""
        "\"${WOLFSSL_ROOT}/src/pk.c\""
        "\"${WOLFSSL_ROOT}/src/ssl_asn1.c\""    # included by ssl.c
        "\"${WOLFSSL_ROOT}/src/ssl_bn.c\""      # included by ssl.c
        "\"${WOLFSSL_ROOT}/src/ssl_certman.c\"" # included by ssl.c
        "\"${WOLFSSL_ROOT}/src/ssl_crypto.c\""  # included by ssl.c
        "\"${WOLFSSL_ROOT}/src/ssl_misc.c\""    # included by ssl.c
        "\"${WOLFSSL_ROOT}/src/x509.c\""
        "\"${WOLFSSL_ROOT}/src/x509_str.c\""
        "\"${WOLFSSL_ROOT}/wolfcrypt/src/evp.c\""
        "\"${WOLFSSL_ROOT}/wolfcrypt/src/misc.c\""
        "\"${EXCLUDE_ASM}\""
        )

    spaces2list(COMPONENT_REQUIRES)

    separate_arguments(COMPONENT_SRCDIRS NATIVE_COMMAND "${COMPONENT_SRCDIRS}")
    separate_arguments(COMPONENT_SRCEXCLUDE NATIVE_COMMAND "${COMPONENT_SRCEXCLUDE}")
    separate_arguments(COMPONENT_ADD_INCLUDEDIRS NATIVE_COMMAND "${COMPONENT_ADD_INCLUDEDIRS}")

    #
    # See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html#example-component-requirements
    #
    message(STATUS "COMPONENT_SRCDIRS = ${COMPONENT_SRCDIRS}")
    message(STATUS "COMPONENT_ADD_INCLUDEDIRS = ${COMPONENT_ADD_INCLUDEDIRS}")
    message(STATUS "COMPONENT_REQUIRES = ${COMPONENT_REQUIRES}")
    message(STATUS "COMPONENT_SRCEXCLUDE = ${COMPONENT_SRCEXCLUDE}")

    #
    # see https://docs.espressif.com/projects/esp-idf/en/stable/esp32/migration-guides/release-5.x/build-system.html?highlight=space%20path
    #
    set(EXTRA_COMPONENT_DIRS "${COMPONENT_SRCDIRS}")
    idf_component_register(
                            SRC_DIRS "${COMPONENT_SRCDIRS}"
                            INCLUDE_DIRS "${COMPONENT_ADD_INCLUDEDIRS}"
                            REQUIRES "${COMPONENT_REQUIRES}"
                            EXCLUDE_SRCS "${COMPONENT_SRCEXCLUDE}"
                            PRIV_REQUIRES esp_timer driver # this will typically only be needed for wolfSSL benchmark
                           )
    # some optional diagnostics
    if (1)
        get_cmake_property(_variableNames VARIABLES)
        list (SORT _variableNames)
        message(STATUS "")
        message(STATUS "ALL VARIABLES BEGIN")
        message(STATUS "")
        foreach (_variableName ${_variableNames})
            message(STATUS "${_variableName}=${${_variableName}}")
        endforeach()
        message(STATUS "")
        message(STATUS "ALL VARIABLES END")
        message(STATUS "")
    endif()

    # target_sources(wolfssl PRIVATE  "\"${WOLFSSL_ROOT}/wolfssl/\""  "\"${WOLFSSL_ROOT}/wolfssl/wolfcrypt\"")
endif() # CMAKE_BUILD_EARLY_EXPANSION



# check to see if there's both a local copy and EDP-IDF copy of the wolfssl components
if( EXISTS "${WOLFSSL_PROJECT_DIR}" AND EXISTS "$ENV{IDF_PATH}/components/wolfssl/" )
    message(STATUS "")
    message(STATUS "")
    message(STATUS "********************************************************************")
    message(STATUS "WARNING: Found components/wolfssl in both local project and IDF_PATH")
    message(STATUS "********************************************************************")
    message(STATUS "")
endif()
# end multiple component check


#
# LIBWOLFSSL_SAVE_INFO(VAR_OUPUT THIS_VAR VAR_RESULT)
#
# Save the THIS_VAR as a string in a macro called VAR_OUPUT
#
# VAR_OUPUT:  the name of the macro to define
# THIS_VAR:   the OUTPUT_VARIABLE result from a execute_process()
# VAR_RESULT: the RESULT_VARIABLE from a execute_process(); "0" if successful.
#
function ( LIBWOLFSSL_SAVE_INFO VAR_OUPUT THIS_VAR VAR_RESULT )
    # is the RESULT_VARIABLE output value 0? If so, IS_VALID_VALUE is true.
    string(COMPARE EQUAL "${VAR_RESULT}" "0" IS_VALID_VALUE)

    # if we had a successful operation, save the THIS_VAR in VAR_OUPUT
    if(${IS_VALID_VALUE})
        # strip newline chars in THIS_VAR parameter and save in VAR_VALUE
        string(REPLACE "\n" ""  VAR_VALUE  ${THIS_VAR})

        # we'll could percolate the value to the parent for possible later use
        # set(${VAR_OUPUT} ${VAR_VALUE} PARENT_SCOPE)

        # but we're only using it here in this function
        set(${VAR_OUPUT} ${VAR_VALUE})

        # we'll print what we found to the console
        message(STATUS "Found ${VAR_OUPUT}=${VAR_VALUE}")

        # the interesting part is defining the VAR_OUPUT name a value to use in the app
        add_definitions(-D${VAR_OUPUT}=\"${VAR_VALUE}\")
    else()
        # if we get here, check the execute_process command and parameters.
        message(STATUS "LIBWOLFSSL_SAVE_INFO encountered a non-zero VAR_RESULT")
        set(${VAR_OUPUT} "Unknown")
    endif()
endfunction() # LIBWOLFSSL_SAVE_INFO

# create some programmatic #define values that will be used by ShowExtendedSystemInfo().
# see wolfcrypt\src\port\Espressif\esp32_utl.c
if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    set (git_cmd "git")
    message(STATUS "Adding macro definitions:")

    # LIBWOLFSSL_VERSION_GIT_ORIGIN: git config --get remote.origin.url
    execute_process(WORKING_DIRECTORY ${WOLFSSL_ROOT} COMMAND ${git_cmd} "config" "--get" "remote.origin.url" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES ERROR_QUIET  )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_ORIGIN "${TMP_OUT}" "${TMP_RES}")

    # LIBWOLFSSL_VERSION_GIT_BRANCH: git rev-parse --abbrev-ref HEAD
    execute_process(WORKING_DIRECTORY ${WOLFSSL_ROOT} COMMAND ${git_cmd} "rev-parse" "--abbrev-ref" "HEAD" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES ERROR_QUIET  )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_BRANCH "${TMP_OUT}" "${TMP_RES}")

    # LIBWOLFSSL_VERSION_GIT_HASH: git rev-parse HEAD
    execute_process(WORKING_DIRECTORY ${WOLFSSL_ROOT} COMMAND ${git_cmd} "rev-parse" "HEAD" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES ERROR_QUIET  )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_HASH "${TMP_OUT}" "${TMP_RES}")

    # LIBWOLFSSL_VERSION_GIT_SHORT_HASH: git rev-parse --short HEAD
    execute_process(WORKING_DIRECTORY ${WOLFSSL_ROOT} COMMAND ${git_cmd} "rev-parse" "--short" "HEAD" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES ERROR_QUIET )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_SHORT_HASH "${TMP_OUT}" "${TMP_RES}")

    # LIBWOLFSSL_VERSION_GIT_HASH_DATE git show --no-patch --no-notes --pretty=\'\%cd\'
    execute_process(WORKING_DIRECTORY ${WOLFSSL_ROOT} COMMAND ${git_cmd} "show" "--no-patch" "--no-notes" "--pretty=\'\%cd\'" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES  )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_HASH_DATE "${TMP_OUT}" "${TMP_RES}")

    message(STATUS "************************************************************************************************")
    message(STATUS "wolfssl component config complete!")
    message(STATUS "************************************************************************************************")
endif()
