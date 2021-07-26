add_executable(http_unittest http_unittest.cpp)
target_link_libraries(http_unittest http crypt corelooper )

set_target_properties(http_unittest PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/bear/work/nfs)
