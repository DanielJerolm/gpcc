# GPCC main CMakeLists.txt file

## Host system requirements

- Linux
- GCC toolchain (min. 8.0) suitable for your target
- CMake 3.21 or newer
- Doxygen 1.9.x (optional)

## Intended use

GPCC is intended to be included as a git sub-module into an upper level project. Typically this file will be included from a top-level CMake project via `add_subdirectory(gpcc)`.

## Configuration options
There are three CMake cache variables that must be configured from the outside:
- GPCC_TargetEnvironment:  
  `productive` or `unittest`.

- GPCC_Compiler:  
  `gcc_arm` or `gcc_x64`.

- GPCC_OS:  
  `chibios_arm`, `epos_arm`, `linux_arm`, `linux_arm_tfc`, `linux_x64`, `linux_x64_tfc`

For details please inspect `CMakeLists.txt`.

Note: There are additional configuration options for the unittest environment. These options are dedicated to special use cases. All these options have default values, that fit normal use cases. For details, please refer to  [doc/special_unittest_configuration_options.md](doc/special_unittest_configuration_options.md).

## Build for productive use

__Integration in top-level CMakeLists.txt:__ (example)
```
set(GPCC_TargetEnvironment "productive" CACHE STRING "" FORCE)

# <-- Skip this for automatic guessing of settings, apply this for explicit configuration
set(GPCC_Compiler "gcc_arm" CACHE STRING "" FORCE)
set(GPCC_OS "epos_arm" CACHE STRING "" FORCE)
# -->

add_subdirectory(extern/gpcc)
```

__Dependencies/requirements to be fulfilled by top level project:__
GPCC_OS       | Dependency/requirement
------------- | ------------------------------------
chibios_arm   | target_link_libraries(gpcc PUBLIC <chibios_and_cpp_runtime>)
epos_arm      | Presence of library `epos_kernel`
linux_arm     | -
linux_x64     | -

__Artifacts build:__
- Static library `gpcc`
- Doxygen documentation (target `gpcc_doxygen`).  
  Building that target must be explicitly requested:  
  `cmake --build <build-folder> --target gpcc_doxygen`  
  A CMake variable `GPCC_DOXYGEN_OUT` will be injected into the scope of the parent project. It refers to the directory in the build-tree that contains the html doc-folder and the tag-file.

## Build for unittest environment

__Integration in top-level CMakeLists.txt:__ (example)
```
set(GPCC_BuildEmptyTestCaseLibrary ON CACHE BOOL "" FORCE)
set(GPCC_TargetEnvironment "unittest" CACHE STRING "" FORCE)

# <-- Skip this for automatic guessing of settings, apply this for explicit configuration
set(GPCC_Compiler "gcc_x64" CACHE STRING "" FORCE)
set(GPCC_OS "linux_x64_tfc" CACHE STRING "" FORCE)
# -->

add_subdirectory(extern/gpcc)
```

__Dependencies required from top level project:__
- gmock ([googletest](https://github.com/google/googletest)) (__only if__ there is a top-level project (GPCC not build "standalone"))

__Artifacts build:__
- Static library `gpcc`
- Object library `gpcc_testcases`
- Executable `output/unittests` (__only if__ there is no top-level project (GPCC is configured "standalone"))
- Doxygen documentation (target `gpcc_doxygen`).  
  Building that target must be explicitly requested:  
  `cmake --build <build-folder> --target gpcc_doxygen`  
  A CMake variable `GPCC_DOXYGEN_OUT` will be injected into the scope of the parent project. It refers to the directory in the build-tree that contains the html doc-folder and the tag-file.

## Compiler options and language standard

Targets linking against `gpcc` or `gpcc_testcases` will receive the following _transitive usage requirements_:
- minimum language standard at least C11 and C++17
- RTTI enabled
- C++ exceptions enabled
- Linking against platform libraries (e.g. `epos_kernel` or `pthread` depending on `GPCC_OS`)
- #defines indicating the compiler, OS, and some gpcc configuration options will be visible to users (e.g. `OS_LINUX_X64_TFC`)

All options will be bound to the build targets!
GPCC will not spread any build options or settings globally via e.g. `add_compile_options()` or `add_compile_definitions()`.
