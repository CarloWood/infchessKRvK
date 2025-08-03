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
  Board.cxx
  Classification.cxx
  Color.cxx
  BlackToMoveData.cxx
  WhiteToMoveData.cxx
  Graph.cxx
  Square.cxx
  infchess.cxx
  parse_move.cxx
  print_formula_table.cxx
)

target_link_libraries(infchess
  PRIVATE
    ${AICXX_OBJECTS_LIST}
)
