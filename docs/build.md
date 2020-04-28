# Overview of the build system
`datalog-cpp` uses the [CMake](www.cmake.org) build tool to generate build files for other build systems. Currently, building using clang and gcc with Makefiles are supported.

# Building with Makefiles
From the command line inside a git clone, run the following:
```
mkdir build
cd build
cmake ../src
make
```
# Building directly with CMake in Visual Studio 2019
Visual Studio 2019 supports using CMake to manage the build directly by selecting File -> Open -> Cmake... and opening `src/CMakeLists.txt`. Then Visual Studio's normal build shortcuts will update the CMake configuration as well as building the project.

CMake options are configured using the `CMakeSettings.json` file, which Visual Studio will generate when `CMakeLists.txt` is opened.

# Build options
Additional configuration variables can be provided in the `cmake` invocation.

For Makefile builds, a build type can be specified by passing `-DCMAKE_BUILD_TYPE=Debug`, `-DCMAKE_BUILD_TYPE=MinSizeRel`, `-DCMAKE_BUILD_TYPE=Release`, or `-DCMAKE_BUILD_TYPE=RelWithDebInfo`. By default, Makefile builds will use `RelWithDebInfo`.
