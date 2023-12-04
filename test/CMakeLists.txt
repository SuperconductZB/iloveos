# set(TARGET_LAYER0 test_layer0)
# set(TARGET_LAYER1_API test_layer1_API)
# set(TARGET_LAYER2_API test_layer2_API)
# set(TARGET_DIR_API test_dir_API)
# set(DIR_PLACE /dev/vdb)
# # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")

# # add test sources here ... 
# add_executable(${TARGET_LAYER0}
#     # add need lib and source code here
#     layer0.cpp

#     ../lib/rawdisk.cpp

# )
# add_executable(${TARGET_LAYER1_API}
#     # add need lib and source code here
#     layer1_API.cpp
#     ../lib/rawdisk.cpp
#     ../lib/fs/datablock_manager.cpp
#     ../lib/fs/fs_data_types.cpp
#     ../lib/fs/fs_file_io.cpp
#     ../lib/fs/fs.cpp
#     ../lib/fs/inode_manager.cpp
# )
# add_executable(${TARGET_LAYER2_API}
#     ../lib/direntry.cpp
#     ../lib/rawdisk.cpp
#     ../lib/fs/datablock_manager.cpp
#     ../lib/fs/fs_data_types.cpp
#     ../lib/fs/fs_file_io.cpp
#     ../lib/fs/fs.cpp
#     ../lib/fs/inode_manager.cpp
#     ../lib/files.cpp
#     layer2_API_dir.cpp
# )
# add_executable(${TARGET_DIR_API}
#     ../lib/direntry.cpp
#     ../lib/rawdisk.cpp
#     ../lib/fs/datablock_manager.cpp
#     ../lib/fs/fs_data_types.cpp
#     ../lib/fs/fs_file_io.cpp
#     ../lib/fs/fs.cpp
#     ../lib/fs/inode_manager.cpp
#     dir_API.cpp
# )

# # Link Google Test to your test executables
# target_link_libraries(${TARGET_LAYER0} gtest gtest_main)
# target_link_libraries(${TARGET_LAYER1_API} gtest gtest_main)
# target_link_libraries(${TARGET_DIR_API} gtest gtest_main)

# # add test to activate ctest -VV
# add_test(NAME ${TARGET_LAYER0} COMMAND sudo ./${TARGET_LAYER0} ${DIR_PLACE})
# add_test(NAME ${TARGET_LAYER1_API} COMMAND sudo ./${TARGET_LAYER1_API} ${DIR_PLACE})
# add_test(NAME ${TARGET_LAYER2_API} COMMAND sudo ./${TARGET_LAYER2_API} ${DIR_PLACE})
# add_test(NAME ${TARGET_DIR_API} COMMAND sudo ./${TARGET_DIR_API} ${DIR_PLACE})


# # Add the -Wall flag
# target_compile_options(${TARGET_LAYER2_API} PRIVATE -Wall)

# # Use pkg-config to get flags for fuse3
# find_package(PkgConfig REQUIRED)
# pkg_search_module(FUSE3 REQUIRED fuse3)

# # Add the flags from pkg-config for fuse3
# target_include_directories(${TARGET_LAYER2_API} PRIVATE ${FUSE3_INCLUDE_DIRS})
# target_link_libraries(${TARGET_LAYER2_API} PRIVATE ${FUSE3_LIBRARIES} gtest gtest_main)