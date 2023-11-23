# CMake generated Testfile for 
# Source directory: /mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22
# Build directory: /mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/bin.t21
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unit_test "/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/bin.t21/tests/unit.test")
set_tests_properties(unit_test PROPERTIES  WORKING_DIRECTORY "/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22" _BACKTRACE_TRIPLES "/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/CMakeLists.txt;2239;add_test;/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/CMakeLists.txt;0;")
add_test(wolfcrypttest "/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/bin.t21/wolfcrypt/test/testwolfcrypt")
set_tests_properties(wolfcrypttest PROPERTIES  WORKING_DIRECTORY "/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22" _BACKTRACE_TRIPLES "/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/CMakeLists.txt;2281;add_test;/mnt/corelooper/CoreLooper.git/projects/wolfssl.11.22/CMakeLists.txt;0;")
