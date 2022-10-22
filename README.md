# GPCC
The swiss army knife for C++ cross-platform embedded software.

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

# Adoption to a specific platform
## Linux (ARM/ARM64/x64)
The underlying platform provides a standart C libarary, STL, and POSIX. Beyond these there is no need to provide any
special functions.

## ChibiOS/RT
Interoperability has been tested with ChibiOS/RT kernel version 6.0.3. Any later version should also be compliant.

The top-level project has to provide public linkage against a library or object-library containing ChibiOS/RT. The
integration of GPCC into the CMakeLists.txt file of the top level project may look like this:

```
# GPCC
set(GPCC_TargetEnvironment "productive" CACHE STRING "" FORCE)
set(GPCC_Compiler "gcc_arm" CACHE STRING "" FORCE)
set(GPCC_OS "chibios_arm" CACHE STRING "" FORCE)
add_subdirectory(gpcc)
target_link_libraries(gpcc PUBLIC <library containing ChibiOS/RT>)
```

GPCC will include the main ChibiOS/RT header this way:

```
#include <ch.h>
```

Alongside ChibiOS/RT, the user has to provide a few functions to allow GPCC to read the system's clocks. These functions
are currently not part of ChibiOS/RT, but the naming follows the ChibiOS/RT pattern for include files and functions:

```
#include <chClockAndTime.h>
```

This header has to provide the following functions:

```
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

  void ch_clock_getres_realtime_coarse(struct timespec* ts);
  void ch_clock_getres_realtime(struct timespec* ts);
  void ch_clock_getres_monotonic_coarse(struct timespec* ts);
  void ch_clock_getres_monotonic(struct timespec* ts);

  void ch_clock_gettime_realtime_coarse(struct timespec* ts);
  void ch_clock_gettime_realtime(struct timespec* ts);
  void ch_clock_gettime_monotonic_coarse(struct timespec* ts);
  void ch_clock_gettime_monotonic(struct timespec* ts);

#ifdef __cplusplus
}
#endif
```

The functions shall provide the resolution and a reading of the following clocks:

- A coarse realtime clock. It shall provide UTC time based on the system tick (e.g. 1ms).
- A precise realtime clock. It shall provide UTC time with the highest feasible precision. On some platforms, reading
  this clock may be more expensive than reading the coarse version of this clock.
- A coarse monotonic clock. It shall provide the time passed by since the system has booted based on the system tick
  (e.g. 1ms). __This shall be the same clock like the one used by ChibiOS/RT to specify timeouts for condition__
  __variables, semaphores etc.__
- A precise monotonic clock. It shall provide the time passed by since the system has booted with the highest feasible
  precision. On some platforms, reading this clock may be more expensive than reading the coarse version of this clock.

For an example, please refer to folder "fake_platform_chibios" in https://github.com/DanielJerolm/gpcc_dev.

The requried functions are inspired by POSIX functionality. Search the web for `clock_getres()`, `clock_gettime()`,
`CLOCK_REALTIME`, `CLOCK_REALTIME_COARSE`, `CLOCK_MONOTONIC` and `CLOCK_MONOTONIC_COARSE`.
