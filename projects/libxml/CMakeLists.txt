cmake_minimum_required(VERSION 3.16)

SET(TARGET_APP xml)
# project(${TARGET_APP})

aux_source_directory(./src SRC)
aux_source_directory(../../depends/pugixml-1.7/src SRC)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./src)
INCLUDE_DIRECTORIES(../../include)
INCLUDE_DIRECTORIES(../../include/core)
INCLUDE_DIRECTORIES(../../include/core/base)
INCLUDE_DIRECTORIES(../../include/libxml)
INCLUDE_DIRECTORIES(../../include/libxml/ajax)

set(SRC_LIST
	${SRC}
	${SRC_INGENIC}
	)

add_library(${TARGET_APP} ${SRC_LIST})
set_target_properties(${TARGET_APP} PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)


