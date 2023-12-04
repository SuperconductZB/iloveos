cmake_minimum_required (VERSION 3.1.0)
project(fischl)

set(CMAKE_CXX_STANDARD 14)

include_directories(
  # fischl include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  # ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
)

add_executable(fischl

  lib/fischl.cpp
  lib/main.cpp
  lib/rawdisk.cpp
  lib/fs/datablock_manager.cpp
  lib/fs/fs_data_types.cpp
  lib/fs/fs_file_io.cpp
  lib/fs/fs.cpp
  lib/fs/inode_manager.cpp
  lib/files.cpp
  lib/direntry.cpp

)

enable_testing()
add_subdirectory(test)
add_subdirectory(googletest)

# Add the -Wall flag
target_compile_options(fischl PRIVATE -Wall)

# Use pkg-config to get flags for fuse3
find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE3 REQUIRED fuse3)

# Add the flags from pkg-config for fuse3
target_include_directories(fischl PRIVATE ${FUSE3_INCLUDE_DIRS})
target_link_libraries(fischl PRIVATE ${FUSE3_LIBRARIES})