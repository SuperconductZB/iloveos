set(TARGET_LAYER0 test_layer0)
set(TARGET_LAYER1_API test_layer1_API)
set(TARGET_LAYER2_API test_layer2_API)
set(TARGET_DIR_API test_dir_API)
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
add_executable(${TARGET_LAYER2_API}
    ../lib/files.cpp
    layer2_API.cpp
)
add_executable(${TARGET_DIR_API}
    ../lib/direntry.cpp
    dir_API.cpp
)

# Link Google Test to your test executables
target_link_libraries(${TARGET_LAYER0} gtest gtest_main)
target_link_libraries(${TARGET_LAYER1_API} gtest gtest_main)
target_link_libraries(${TARGET_DIR_API} gtest gtest_main)

# add test to activate ctest -VV
add_test(NAME ${TARGET_LAYER0} COMMAND sudo ./${TARGET_LAYER0} ${DIR_PLACE})
add_test(NAME ${TARGET_LAYER1_API} COMMAND sudo ./${TARGET_LAYER1_API} ${DIR_PLACE})
add_test(NAME ${TARGET_LAYER2_API} COMMAND sudo ./${TARGET_LAYER2_API} ${DIR_PLACE})
add_test(NAME ${TARGET_DIR_API} COMMAND sudo ./${TARGET_DIR_API} ${DIR_PLACE})