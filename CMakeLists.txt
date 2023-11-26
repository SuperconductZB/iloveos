cmake_minimum_required (VERSION 3.1.0)
project(fischl)

set(CMAKE_CXX_STANDARD 14)

include_directories(
  # fischl include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
)

add_executable(fischl

  lib/fischl.cpp
  lib/main.cpp
  lib/rawdisk.cpp
  lib/fs/datablock_manager.cpp
  lib/fs/fs_data_types.cpp
  lib/fs/fs_resize.cpp
  lib/fs/fs_read_write.cpp
  lib/fs/fs.cpp
  lib/fs/inode_manager.cpp
  lib/files.cpp
  lib/direntry.cpp

)

enable_testing()
add_subdirectory(test)
add_subdirectory(googletest)