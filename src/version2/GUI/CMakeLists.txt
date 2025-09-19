#==============================================================================
# POLICIES
#
if (POLICY CMP0156)
  cmake_policy(SET CMP0156 NEW)
  set(QT_FORCE_CMP0156_TO_VALUE NEW)
endif ()

set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

file(GLOB_RECURSE UiFiles CONFIGURE_DEPENDS "*.ui")
message(STATUS "UiFiles = \"${UiFiles}\"")

add_library(gui_ObjLib OBJECT)

target_sources(gui_ObjLib
  PRIVATE
    "ChessApplication.cxx"
    "qtcreator01/ChessWindowBase.cxx"
    "ChessWindow.cxx"
    "ChessMenuBar.cxx"
    "ChessIconFactory.cxx"
)

qt_add_ui(gui_ObjLib SOURCES ${UiFiles})

target_link_libraries(gui_ObjLib
  PUBLIC
    ${AICXX_OBJECTS_LIST}
    Qt6::Widgets
)

# Create an ALIAS target.
add_library(gui::gui ALIAS gui_ObjLib)
