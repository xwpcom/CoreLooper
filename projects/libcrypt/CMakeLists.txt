cmake_minimum_required(VERSION 3.5)

SET(TARGET_APP crypt)
# project(${TARGET_APP})

aux_source_directory(./src SRC)
aux_source_directory(./src/ssl SRC)

# message(STATUS " ${PROJECT_SOURCE_DIR} ")

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./src)
INCLUDE_DIRECTORIES(../../include)
INCLUDE_DIRECTORIES(../../include/core)
INCLUDE_DIRECTORIES(../../include/core/base)
INCLUDE_DIRECTORIES(../../include/libcrypt)
INCLUDE_DIRECTORIES(../../include/libcrypt/ssl)

INCLUDE_DIRECTORIES(/mnt/os/openssl/openssl-1.1.1k/include)

set(SRC_LIST
	${SRC}
	${SRC_INGENIC}
	)

add_library(${TARGET_APP} ${SRC_LIST})
set_target_properties(${TARGET_APP} PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)


