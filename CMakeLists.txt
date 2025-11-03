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

# ---- Add the library subdirectory ----
add_subdirectory(src)

# ---- Tool executable ----
add_executable(raytracing tools_raytracing/raytracing.cpp)

# Link the tool against the library (which itself links to Embree)
target_link_libraries(raytracing PRIVATE raytracing_objects)

# Add includes if your tool directly includes headers from the lib
target_include_directories(raytracing PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(raytracing PROPERTIES
    BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
)
message(PROJECT_SOURCE_DIR="${CMAKE_INSTALL_BINDIR}")
# ---- Install rules ----
include(GNUInstallDirs)
install(TARGETS raytracing
	RUNTIME DESTINATION bin
)
