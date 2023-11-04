set(TARGET_LAYER0 test_layer0)


# add test sources here ... 
add_executable(${TARGET_LAYER0}
    # add need lib and source code here
    layer0.cpp
)

# add test to activate ctest -VV
add_test(NAME ${TARGET_LAYER0} COMMAND sudo ./${TARGET_LAYER0})