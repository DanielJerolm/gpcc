# GPCC
The swiss army knife for C++ cross-platform portable embedded software.

# About
GPCC is a C++17 class library, that is in particular useful for development of cross-platform portable embedded software. The classes offered by GPCC are meant to be an addendum to the STL. Some classes, in particluar those relevant for cross-platform portability, are meant to be an alternative to some classes from STL.

The term "embedded" refers to devices starting at Cortex-M CPUs.

## Features
- __Abstraction for threading (OSAL), time, compiler, and file system__  
  Write platform independent multithreaded software in C++, that can be reused among a large range of platforms, starting at Cortex-M devices running an RTOS with C++ support up to Cortex-A and x64 platforms running Linux.
- __Time flow control__  
  Some implementations of GPCC's OSAL are instrumented ("TFC") and allow you to write unittests for software components that have an intended timing behavior. If properly used, then these unittest cases allow for testing the timing behaviour and will provide 100% reproducible results, independent of the host's workload.  
  TFC also allows you to write hardware models with defined timing behaviour and to test your software with these hardware fakes.  
  However, if you want to use TFC, you will have to use GPCC functionality instead of STL's `<thread>` and instead of any of the `now()`-methods offered by the clocks from STL's `<chrono>`.
- __Command line interface (CLI)__  
  Allows developers to interact with the device: Run commands, query information, see logs etc.
- __Log system__  
  See what's going on. Filter by log-source name and message severity via CLI.
- __CANopen object dictionary__  
  Compatible with EtherCAT. Rich set of supported data types, incl. variable length data types (e.g. VISIBLE_STRING and OCTET_STRING). Pre-write, post-write and pre-read callbacks. Access to objects via synchronous interface, via asynchronous messages, and via command line interface.
- __Stream composition and decomposition__
- __RAII__  
  Write exception-safe and cancellation-safe code with ease.
- __String__  
  String conversion, efficient string composition, and reference counted shared strings.  
  Using `gpcc::string::StringComposer` instead of STL's `std::ostringstream` will safe you up to ~180kB of machine code! (with gcc 10.3 on Cortex-M4 CPU)
- __CRC__
- __Standard interfaces__  
  Abstract base classes for drivers. These allow for instance seamless interoperation of drivers for arbitrary I2C master IPs and drivers for I2C peripherals.
- __Miscellaneous__  
  Additional types of containers, workqueues, resource management, power-fail-safe filesystem for EEPROM devices, etc.

## Quality
The following measures are applied:
- Unittest line coverage >90%
- Appliance of valgrind (memcheck) and gcc's undefined behaviour sanitizer
- Applicance of RAII: Avoidance of `new` and raw pointers, 100% exception-safe and thread-cancellation-safe code
- Zero warnings from all tools and tests.
- Appliance of "Modern CMake"
- Reproducible builds: Build steps are controlled by scripts which are under version control.
- "Issues welcome" attitude.

## Host (development system) requirements
- Linux
- GCC toolchain
- CMake 3.21 or newer
- doxygen 1.9.8 or newer
- vscode (recommended) with C/C++ Extension Pack

# Getting started
GPCC is typically integrated as a git submodule into a git superproject.

GPCC distinguishes two environments: _Productive environment_ and _unittest environment_.  
In the superproject's CMakeLists.txt file, GPCC can be integrated like this:
```
if(BUILD_PRODUCTIVE)
  set(GPCC_TargetEnvironment "productive" CACHE STRING "" FORCE)
  set(GPCC_Compiler "gcc_arm" CACHE STRING "" FORCE)
  set(GPCC_OS "epos_arm" CACHE STRING "" FORCE)
else()
  set(GPCC_TargetEnvironment "unittest" CACHE STRING "" FORCE)
  set(GPCC_Compiler "gcc_x64" CACHE STRING "" FORCE)
  set(GPCC_OS "linux_x64_tfc" CACHE STRING "" FORCE)
endif()
add_subdirectory(extern/gpcc)
```

Alternatively, GPCC may guess values for `GPCC_Compiler` and `GPCC_OS` automatically.  
For automatic guessing, integrate GPCC like this:
```
if(BUILD_PRODUCTIVE)
  set(GPCC_TargetEnvironment "productive" CACHE STRING "" FORCE)
else()
  set(GPCC_TargetEnvironment "unittest" CACHE STRING "" FORCE)
  # Note: For the unittest environment, guessing will always select an OSAL with TFC.
endif()
add_subdirectory(extern/gpcc)
```

For development of GPCC and for purposes of automated testing (e.g. github actions), GPCC can also be directly cloned and build "standalone".

The way GPCC is used determines which artifacts are build:
| Artifact                                    | CMake sub-project                 | stand alone
| ------------------------------------------- | --------------------------------- | ---------------------------------
| static library `gpcc`                       | productive + unittest environment | productive + unittest environment
| object library `gpcc_testcases`             | unittest environment only         | unittest environment only
| executable `build_unittest/output/unittest` | no                                | unittest environment only

Note that libarary `gpcc` contains additional classes if GPCC is configured for the unittest environment. These additional classes may be useful for unittesting components of the top level project. They are of no use (or not even applicable) in the productive environment.

## Build standalone
### Generate doxygen documentation
There are multiple configuration files for doxygen, each tuned to a specific configuration of GPCC:  
_operating system_ + _TFC (y/n)_ + _extract all (y/n)_

The following example generates the documentation for GPCC, if GPCC is configured for Linux with TFC-instrumented OSAL:
```
# We start in GPCC's root folder
$ cd scripts
$ ./build_doxygen_linux_x64.sh
$ cd ../build_doxygen_linux_x64
$ firefox html/index.html &
```

Then:
- At the landing page, click on "Topics" (former "Modules" in previous doxygen versions) in the top of the HTML page.
- In the top right corner, select __detail level 1__.
- From there you can explore the documentation.

### Build and run unittests from shell
Note that GPCC uses two separate build-folders for the _productive_ environment and for the _unittest_ environment.  
The following example configures the build-folder for the unittest environment (./build_unittest) for building the unittests for the native linux host applying _debug_ configuration settings:
```
# We start in GPCC's root folder
$ cd scripts
$ ./cmake_config_nativelinux-unittest-debug.sh
$ ./build_unittest.sh all
$ ./execute_unittests.sh
```

### Build productive library for native Linux
Note that GPCC uses two separate build-folders for the _productive_ environment and for the _unittest_ environment.  
The following example configures the build-folder for the productive environment (./build_productive) for building the productive library for the native linux host applying _release_ configuration settings:
```
# We start in GPCC's root folder
$ cd scripts
$ ./cmake_config_nativelinux-productive-release.sh
$ ./build_productive.sh all
```

### Cleanup
It is safe to delete the `build_*`-folders. They can be recreated by running the respective build steps.

There is also a script to remove all build-folders:
```
# We start in GPCC's root folder
$ cd scripts
$ ./full_clean.sh
```

### Open, edit and build using VSCODE
In GPCC's root folder invoke vscode:
```
$ vscode .
```

GPCC uses two separate build-folders for the _productive_ environment and for the _unittest_ environment. During CMake initialization of a build-folder, a file `compile_commands.json` will be created in the build-folder. The file is required by Intellisense to understand the code.

In the previous chapters, the CMake initialization of the build-folders has been accomplished via the shell-scripts in the scripts-folder. Most actions that can be accomplished via the scripts can also be triggered via vscode tasks. Let's configure the build folders. Run the following tasks:
- Main menue: Terminal --> Run Task... --> CMake configure: Native Linux productive (release)
- Main menue: Terminal --> Run Task... --> CMake configure: Native Linux unittest (debug)

Now open a file, e.g. `src/cli/CLI.cpp`.

In the bottom right of the vscode window there should be a bell and a label `Linux-Productive` or `Linux-Unittest`. The label is only visible, if a cpp- or hpp-file is currently open. For instance it will vanish, if you open this markdown file you are currently reading.

The label indicates the currently active _C/C++ configuration_. If you click on the label, then a menue appears at the top of the vscode window and you can select the configuration. By changing between `Linux-Productive` or `Linux-Unittest` Intellisense instantly switches between the _productive_ environment and the _unittest_ environment. In both environments #defines and include-paths may differ and thus Intellisense will evaluate preprocessor directives and #includes differently.

For now, you should not see any "problems" and there should be no errors highlighted in the code.

If #include statements are marked with a red underline the most likely error is that the build-folder that corresponds to the selected C/C++ configuration does not contain a `compile_commands.json` file.
