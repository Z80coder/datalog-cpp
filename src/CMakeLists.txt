# CMakeLists.txt

# CMake setup
cmake_minimum_required (VERSION 3.8)

# Set a default build type if none was specified
set(default_build_type "RelWithDebInfo")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Project initialisation
project("datalog-cpp")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)

# types_test target
add_executable(types_test ../tests/types_test.cpp)
target_include_directories(types_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(types_test PUBLIC UNIX)