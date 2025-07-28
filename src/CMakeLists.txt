cmake_minimum_required(VERSION 3.14...4.0.2)

project(chess3
  LANGUAGES C CXX
  DESCRIPTION "C++ formatter."
)

# Compile all subdirectories with the same standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(infchess
  Board0.cxx
  Board1.cxx
  Classification0.cxx
  Classification1.cxx
  Color.cxx
  BlackToMoveData0.cxx
  BlackToMoveData1.cxx
  WhiteToMoveData0.cxx
  WhiteToMoveData1.cxx
  Graph0.cxx
  Graph1.cxx
  Square0.cxx
  Square1.cxx
  infchess.cxx
  parse_move.cxx
)

target_link_libraries(infchess
  PRIVATE
    ${AICXX_OBJECTS_LIST}
)
