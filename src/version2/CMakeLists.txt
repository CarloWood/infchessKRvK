cmake_minimum_required(VERSION 3.14...4.0.2)

add_executable(infchess2
  BlockIndex.cxx
  Board.cxx
  Classification.cxx
  Graph.cxx
  Info.cxx
  KingSquare.cxx
  Square.cxx
  infchess2.cxx
  ../Color.cxx
  ../parse_move.cxx
)

target_link_libraries(infchess2
  PRIVATE
    ${AICXX_OBJECTS_LIST}
    enchantum::enchantum
)
