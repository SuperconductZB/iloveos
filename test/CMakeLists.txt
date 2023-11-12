set(TARGET_LAYER0 test_layer0)
set(TARGET_LAYER1_API test_layer1_API)
set(TARGET_LAYER1_TEST1 test_layer1_test1)
set(DIR_PLACE /dev/vdb)

# add test sources here ... 
add_executable(${TARGET_LAYER0}
    # add need lib and source code here
    layer0.cpp
)
add_executable(${TARGET_LAYER1_API}
    # add need lib and source code here
    layer1_API.cpp
)
add_executable(${TARGET_LAYER1_TEST1}
    # add need lib and source code here
    layer1_test1.cpp
)

# add test to activate ctest -VV
add_test(NAME ${TARGET_LAYER0} COMMAND sudo ./${TARGET_LAYER0} ${DIR_PLACE})
add_test(NAME ${TARGET_LAYER1_API} COMMAND sudo ./${TARGET_LAYER1_API} ${DIR_PLACE})
add_test(NAME ${TARGET_LAYER1_TEST1} COMMAND sudo ./${TARGET_LAYER1_TEST1} ${DIR_PLACE})