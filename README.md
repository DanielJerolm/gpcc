# GPCC
The swiss army knife for C++ cross-platform portable embedded software.

# Before cloning...
GPCC is intended to be used as a git submodule in a git superproject. Cloning the GPCC repo alone makes little sense
and is of limited use in most scenarios. It may even be confusing for new users.

To evaluate or develop GPCC on a Linux host, please clone __gpcc_dev__, which contains GPCC as a git submodule and
provides a fully configured VSCODE workspace and CMake files for all build configurations:

HTTPS:

`git clone -b master --recurse-submodules https://github.com/DanielJerolm/gpcc_dev.git`

SSH:

`git clone -b master --recurse-submodules git@github.com:DanielJerolm/gpcc_dev.git`

# About
## What is it
GPCC is a C++17 class library, that is particulary useful in the field of "embedded devices". The term "embedded
devices" scales from Cortex-M3 devices running an RTOS up to Cortex-A series or x64 devices running Linux. However, it
originally aimed towards small RTOS based embedded devices.

GPCC is intended to be used as an addition to the STL and brings stuff like an OSAL, a log system, a file system
abstraction, a filesystem, a command line interface, a CANopen object dictionary and __many more__ to you. However, if
you want to use GPCC's TFC feature, it will require that you use GPCC functionality instead of STL's `<thread>` and
any of the `now()`-methods offered by the clocks offered by STL's `<chrono>`.

## Main Benefits
- GPCC's OSAL and file system abstraction allows you to write __platform independent software__ in C++, that can be
  __reused__ among a large range of platforms, starting at Cortex-M devices running an RTOS up to Cortex-A and x64
  platforms running Linux.

- Some implementations of GPCC's OSAL are instrumented ("TFC") and allow you to write unittests for software components
  __that have an intended timing behavior__. If properly used, these unit test cases allow for __testing the timing__
  __behaviour__ and will provide __100% reproducible results__ and often a shorter test duration, __independent__ of the
  host and its workload.

## Target applications
Firmware for embedded devices starting at Cortex-M series MCUs with no or limited user interface.

## Origin
I started programming C++ applications in the early 2000s on Windows. In the mid 2000s I started to switch from C to C++
on small MCUs after years of programming in C. I quickly recognized that an object oriented approach combined with C++
has the potential to increase reusability of embedded software tremendously, even across different platforms.

In the end of 2000s I started to collect some of the classes I wrote for different projects and reused them in other
projects. GPCC was born.

# Getting started
## Explore the documentation
Clone gpcc_dev (see above) and build GPCC's doxygen documentation:

```
$ cd gpcc_dev

$ cd gpcc/scripts
$ doxygen doxyfile_chibios_arm

$ cd ../doc/doxygen
$ firefox html/index.html &
```

Click on "Modules" in the top of the HTML page.

## Build the unit tests
Clone gpcc_dev (see above) and build and run GPCC's unit tests:

```
$ cd gpcc_dev/scripts

$ ./cmake_init_unittest_release.sh
$ ./build_unittest.sh all

$ ./execute_unittests.sh
```

## Explore the build system

- Examine `gpcc_dev/scripts/cmake_init_unittest_release.sh`
- Examine `gpcc_dev/CMakeLists.txt`
- Examine `gpcc_dev/gpcc/CMakeLists.txt`

## Explore the code
Clone gpcc_dev (see above) and explore the code:

gpcc_dev contains a VSCODE workspace file (`gpcc_dev-Workspace.code-workspace`). Open it in VSCODE. gpcc_dev contains
all relevant settings for VSCODE to build GPCC (Terminal -> Run Task... -> ...) and to debug the unit tests.

Intellisense requires a file `compile_commands.json` from CMake to resolve includes and symbols properly. After opening
the workspace for the first time, you should take the following actions:

1. Run `Terminal->Run Task...->cmake_init_unittest_release` from the main menu.

2. Open any CPP/HPP-file.

3. Click on the drop-down-box in the bottom right of VSCODE that displays the current configuration.

4. Choose the configuration "Linux-unittests".
