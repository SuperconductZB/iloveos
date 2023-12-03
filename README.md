# iloveos

Simply copying libfuse.


## Branching Rules

1. Fork the main branch and work on your own branch.
2. Test before initiating a Pull Request.
3. Get approved to get merged.

# Quick Start

## configure and build code
make build directory 
```bash
mkdir -p build && cd build
cmake ..
make # cmake --build . is same
```

## mount and test
normal usage:
```bash
./fischl diskpath mountpoint
```
diskpath must be the provided following ./fischl

if the diskpath need to be accessed by root:
```bash
sudo ./fischl diskpath -o allow_other mountpoint
```

for debugging:
```bash
sudo ./fischl diskpath -o allow_other -d mountpoint
```

## run test
### add your own test file on test/CMakeList.txt
```
set(TARGET_NAME run_tests)
set(TEST_NAME test_test)

# add test sources here ... 
add_executable(${TARGET_NAME} 

    ../lib/fischl.cpp 
    testfischl.cpp
    
)
add_executable(${TEST_NAME} 

    ../lib/fischl.cpp 
    testtest.cpp
    
)
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
```

### ctest
```bash
ctest -VVV 
ctest -VV  #Displays the output from the tests (e.g., stdout or stderr) in addition to the test information.
```
Test Result will be like this
```bash
[cloud-user@ip-172-31-22-147 build]$ ctest -VVV
Test project /home/cloud-user/iloveos/build
    Start 1: run_tests
1/1 Test #1: run_tests ........................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```

Failed demonstration
```bash
[cloud-user@ip-172-31-22-147 build]$ ctest -VVV
Test project /home/cloud-user/iloveos/build
    Start 1: run_tests
1/2 Test #1: run_tests ........................   Passed    0.00 sec
    Start 2: test_test
2/2 Test #2: test_test ........................Subprocess aborted***Exception:   0.26 sec

50% tests passed, 1 tests failed out of 2

Total Test time (real) =   0.27 sec

The following tests FAILED:
          2 - test_test (Subprocess aborted)
Errors while running CTest
Output from these tests are in: /home/cloud-user/iloveos/build/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
```
