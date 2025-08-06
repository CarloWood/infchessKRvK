cmake_minimum_required(VERSION 3.14...4.0.2)

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

add_subdirectory(version2)
