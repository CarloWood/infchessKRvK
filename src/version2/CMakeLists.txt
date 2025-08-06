cmake_minimum_required(VERSION 3.14...4.0.2)

add_executable(infchess2
  Board.cxx
  Classification.cxx
  Color.cxx
  Info.cxx
  Square.cxx
  infchess2.cxx
)

target_link_libraries(infchess2
  PRIVATE
    ${AICXX_OBJECTS_LIST}
)
