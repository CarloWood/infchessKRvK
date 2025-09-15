find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

add_library(gui_ObjLib OBJECT)

target_sources(gui_ObjLib
  PRIVATE
    "ChessApplication.cxx"
    "ChessWindow.cxx"
    "ChessMenuBar.cxx"
    "ChessIconFactory.cxx"
)

# Required include search-paths.
target_include_directories(gui_ObjLib
  PUBLIC
    "${CMAKE_SOURCE_DIR}"                       # For local sys.h and debug.h.
    "${CMAKE_SOURCE_DIR}/cwds"                  # For sys.h and debug.h fallbacks.
    "${CMAKE_CURRENT_SOURCE_DIR}/.."            # For version2 headers.
)

target_link_libraries(gui_ObjLib
  PUBLIC
    PkgConfig::GTKMM
)

# Create an ALIAS target.
add_library(gui::gui ALIAS gui_ObjLib)
