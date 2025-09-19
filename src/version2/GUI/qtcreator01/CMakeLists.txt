cmake_minimum_required(VERSION 3.16)

project(qtcreator01 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

qt_add_executable(qtcreator01
  MANUAL_FINALIZATION
    main.cpp
    ChessWindowBase.h
    ChessWindow.ui
    ChessWindowBase.cxx
)

target_link_libraries(qtcreator01 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(qtcreator01)
endif()
