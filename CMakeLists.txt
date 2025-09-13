# FetchContent was added to 3.14.
cmake_minimum_required(VERSION 3.14...4.0.2)

# Compile all subdirectories with the same standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(infchessKRvK
  LANGUAGES C CXX
  DESCRIPTION "Analyse K+R vs K on a quarter-infinity chess board."
)

#==============================================================================
# Begin of gitache configuration.
set(GITACHE_PACKAGES libcwd_r)
include(cwm4/cmake/StableGitache)
# End of gitache configuration.

#==============================================================================
# This project uses aicxx modules.
include(cwm4/cmake/AICxxProject NO_POLICY_SCOPE)

include(AICxxSubmodules)

#==============================================================================
# OPTIONS

# End of OPTIONS section.

#==============================================================================

# Exit if someone tries to contaminate the source directory with an in-source build.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Please do out-of-source builds."
                      "Cleanup: \"rm -rf CMake CMakeCache.txt CMakeFiles/\"")
endif ()

#==============================================================================
# CONFIG FILES

# Tell cwds/sys.h that we have a config.h.
#add_definitions(-DHAVE_CONFIG_H)
#include_directories(${top_objdir})
# End of CONFIG FILES section.

#==============================================================================

# Declare aicxx submodules.
include(AICxxSubmodules)

#==============================================================================

add_subdirectory(src)

# We use utils/to_string.h
add_subdirectory(enchantum)
