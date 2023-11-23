# CMake generated Testfile for 
# Source directory: /home/bear/work/os/wolfssl.11.22
# Build directory: /home/bear/work/os/wolfssl.11.22/bin.pclinux
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unit_test "/home/bear/work/os/wolfssl.11.22/bin.pclinux/tests/unit.test")
set_tests_properties(unit_test PROPERTIES  WORKING_DIRECTORY "/home/bear/work/os/wolfssl.11.22" _BACKTRACE_TRIPLES "/home/bear/work/os/wolfssl.11.22/CMakeLists.txt;2237;add_test;/home/bear/work/os/wolfssl.11.22/CMakeLists.txt;0;")
add_test(wolfcrypttest "/home/bear/work/os/wolfssl.11.22/bin.pclinux/wolfcrypt/test/testwolfcrypt")
set_tests_properties(wolfcrypttest PROPERTIES  WORKING_DIRECTORY "/home/bear/work/os/wolfssl.11.22" _BACKTRACE_TRIPLES "/home/bear/work/os/wolfssl.11.22/CMakeLists.txt;2279;add_test;/home/bear/work/os/wolfssl.11.22/CMakeLists.txt;0;")
