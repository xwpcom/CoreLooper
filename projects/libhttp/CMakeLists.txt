cmake_minimum_required(VERSION 3.16)

SET(TARGET_APP http)
# project(${TARGET_APP})

aux_source_directory(./src SRC)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./src)
INCLUDE_DIRECTORIES(../../include)
INCLUDE_DIRECTORIES(../../include/core)
INCLUDE_DIRECTORIES(../../include/core/base)
INCLUDE_DIRECTORIES(../../include/libhttp)

set(SRC_LIST
	${SRC}
	${SRC_INGENIC}
	)

add_library(${TARGET_APP} ${SRC_LIST})
set_target_properties(${TARGET_APP} PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)


add_subdirectory(src/tests)