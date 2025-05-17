cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(lzip VERSION 0.0.1 LANGUAGES CXX)

# Default to Release for maximum optimization if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release flags for speed
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Optionally build documentation
option(BUILD_DOCS "Enable building of documentation" OFF)

# Header-only interface library
add_library(lzip INTERFACE)
target_include_directories(lzip INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(lzip INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Application
add_subdirectory(app)

# Testing
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(ext/Catch2)
  include(ext/Catch2/contrib/Catch.cmake)
  add_subdirectory(tests)
endif()

# Documentation
if(BUILD_DOCS)
  add_subdirectory(doc)
endif()

# Alias for embedding
add_library(lzip::lzip ALIAS lzip)

# Installation
install(
  TARGETS lzip
  EXPORT lzip-config
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  EXPORT lzip-config
  NAMESPACE lzip::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lzip
)

install(
  DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Summary
include(FeatureSummary)
feature_summary(WHAT ALL)
