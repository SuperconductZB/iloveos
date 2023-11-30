set(TARGET_LAYER0 test_layer0)
set(TARGET_LAYER1_API test_layer1_API)
# set(TARGET_LAYER1_TEST1 test_layer1_test1)
set(DIR_PLACE /dev/vdb)

# add test sources here ... 
add_executable(${TARGET_LAYER0}
    # add need lib and source code here
    layer0.cpp

    ../lib/rawdisk.cpp

)
add_executable(${TARGET_LAYER1_API}
    # add need lib and source code here
    layer1_API.cpp

    ../lib/fischl.cpp
    ../lib/rawdisk.cpp
    ../lib/fs/datablock_manager.cpp
    ../lib/fs/fs_data_types.cpp
    ../lib/fs/fs_file_io.cpp
    ../lib/fs/fs.cpp
    ../lib/fs/inode_manager.cpp
)
# add_executable(${TARGET_LAYER1_TEST1}
#     # add need lib and source code here
#     layer1_test1.cpp
# )

# add test to activate ctest -VV
add_test(NAME ${TARGET_LAYER0} COMMAND sudo ./${TARGET_LAYER0} ${DIR_PLACE})
add_test(NAME ${TARGET_LAYER1_API} COMMAND sudo ./${TARGET_LAYER1_API} ${DIR_PLACE})
# add_test(NAME ${TARGET_LAYER1_TEST1} COMMAND sudo ./${TARGET_LAYER1_TEST1} ${DIR_PLACE})