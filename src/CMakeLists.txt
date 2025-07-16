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
  BlackKing.cxx
  Board.cxx
  Color.cxx
  Data.cxx
  FEN.cxx
  Graph.cxx
  KingMoves.cxx
  Rook.cxx
  RookMoves.cxx
  Square.cxx
  WhiteKing.cxx
  infchess.cxx
)

target_link_libraries(infchess
  PRIVATE
    ${AICXX_OBJECTS_LIST}
)
