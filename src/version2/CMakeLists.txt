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

add_executable(mmap_server32
  BlockIndex.cxx
  Board.cxx
  Classification.cxx
  Graph.cxx
  Info.cxx
  KingSquare.cxx
  Square.cxx
  mmap_server.cxx
  ../Color.cxx
)

target_compile_definitions(mmap_server32
  PUBLIC
    SIZE_BX=8 SIZE_BY=8 SIZE_PX=4 SIZE_PY=4
)

target_link_libraries(mmap_server32
  PRIVATE
    ${AICXX_OBJECTS_LIST}
    enchantum::enchantum
)

add_executable(mmap_server64
  BlockIndex.cxx
  Board.cxx
  Classification.cxx
  Graph.cxx
  Info.cxx
  KingSquare.cxx
  Square.cxx
  mmap_server.cxx
  ../Color.cxx
)

target_compile_definitions(mmap_server64
  PUBLIC
    SIZE_BX=8 SIZE_BY=8 SIZE_PX=8 SIZE_PY=8
)

target_link_libraries(mmap_server64
  PRIVATE
    ${AICXX_OBJECTS_LIST}
    enchantum::enchantum
)

add_executable(compare
  compare.cxx
  ../Color.cxx
)

target_link_libraries(compare
  PRIVATE
    ${AICXX_OBJECTS_LIST}
    enchantum::enchantum
)

add_executable(play
  BlockIndex.cxx
  Board.cxx
  Classification.cxx
  Graph.cxx
  Info.cxx
  KingSquare.cxx
  Square.cxx
  play.cxx
  ../Color.cxx
)

target_link_libraries(play
  PRIVATE
    ${AICXX_OBJECTS_LIST}
    gui::gui
    enchantum::enchantum
)

add_subdirectory(GUI)
