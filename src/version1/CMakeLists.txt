cmake_minimum_required(VERSION 3.14...4.0.2)

add_executable(infchess1
  Board.cxx
  Classification.cxx
  BlackToMoveData.cxx
  WhiteToMoveData.cxx
  Graph.cxx
  Square.cxx
  ../Color.cxx
  ../parse_move.cxx
  ../print_formula_table.cxx
  infchess1.cxx
)

target_link_libraries(infchess1
  PRIVATE
    ${AICXX_OBJECTS_LIST}
)
