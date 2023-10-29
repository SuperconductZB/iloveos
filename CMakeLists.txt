cmake_minimum_required (VERSION 3.1.0)
project(fischl)

set(CMAKE_CXX_STANDARD 14)

include_directories(
  # fischl include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(fischl

  # Header filesc
  lib/fischl.cpp
  lib/main.cpp

)

enable_testing()
add_subdirectory(test)
