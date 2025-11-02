cmake_minimum_required(VERSION 3.16)

project(RaytracingTools LANGUAGES CXX)

# --- Only dependency: Embree (the library subdir also calls find_package for embree) ---
# You can point CMake to a custom Embree with: -Dembree_DIR=/path/to/embree/cmake
# or EMBREE_ROOT, etc., depending on your Embree install.

# ---- Common compiler flags (your raytracing library links against 'compiler_flags') ----
add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_20)
if(MSVC)
  target_compile_options(compiler_flags INTERFACE /W4)
else()
  target_compile_options(compiler_flags INTERFACE -Wall -Wextra -Wpedantic)
endif()

# ---- Stub out optional/unused externals referenced by the library CMake ----
# Keep these empty so the subproject's target_link_libraries lines remain valid.

# Some versions of your library CMake may unconditionally link 'libsixt'.
# Provide a dummy INTERFACE target so it doesn't pull real externals.
if(NOT TARGET libsixt)
  add_library(libsixt INTERFACE)
endif()

# ---- Add the library subdirectory ----
add_subdirectory(src)

# ---- Tool executable ----
add_executable(raytracing_tool tools_raytracing/raytracing.cpp)

# Link the tool against the library (which itself links to Embree)
target_link_libraries(raytracing_tool PRIVATE raytracing_objects)

# Add includes if your tool directly includes headers from the lib
target_include_directories(raytracing_tool PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---- Install rules ----
include(GNUInstallDirs)
install(TARGETS raytracing_tool
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

