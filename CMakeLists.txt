# Main CMakeLists.txt-file for GPCC
# =================================
#
# Intended use
# ------------
#
# GPCC is intended to be treated as a sub-module in an upper level project. So the typical use case is, that this file
# will be included from a top-level cmake project via "add_subdirectory(gpcc)". However, this file may also be used
# stand-alone to build GPCC's artifacts.
#
# Configuration
# -------------
#
# GPCC strictly distinguishes between two target environments. You have to select the proper environment for your build
# and set the cache variable "GPCC_TargetEnvironment" accordingly:
# - productive : A library "gpcc" will be build for productive use.
# - unittest   : A library "gpcc" and an object-library "gpcc_testcases" will be build for usage in a unittest
#                environment. The unittest environment is intended for executing GPCC's unit tests as well as
#                unit tests from the upper level project.
#
# Further you have to provide the user settings for "GPCC_Compiler" and "GPCC_OS".
#
# There are three approaches to provide the user settings:
# a) Via the cmake gui tool.
#
# b) During configuration. Example:
#    -DGPCC_TargetEnvironment:STRING=productive
#    -DGPCC_Compiler:STRING=gcc_arm
#    -DGPCC_OS:STRING=chibios_arm
#
# c) By the upper-level cmake project file. Example:
#    set(GPCC_TargetEnvironment "unittest" CACHE STRING "")
#    set(GPCC_Compiler "gcc_arm" CACHE STRING "")
#    set(GPCC_OS "chibios_arm" CACHE STRING "")
#
# Valid values and the help text can be retrieved using the cmake gui tool.
#
# After configuration it is strongly recommended that you check your settings:
# 1. cd to you build directory
# 2. Type: cmake . -LA | grep GPCC_
#    To include help text, type: cmake . -LAH | grep -B 1 GPCC_
#
# In addition to the three settings mentioned before, grep will show some more options depending on the selected target
# environment. There are reasonable defaults for these settings, but you should have a look at them and confirm them.
#
# Compiler options and language standard
# --------------------------------------
#
# GPCC requires for itself and for its users:
# - at least C++17
# - presence of RTTI
# - presence of C++ exceptions
#
# GPCC will impose the required compiler features and compiler options for itself and for its users. See 'common.cmake',
# function 'SetRequiredCompilerFeatures' for details.
#
# Dependencies
# ------------
#
# In unittest environment:
# - The top level project has to provide googletest (gmock library).
#
# In productive environment:
# For GPCC_OS=chibios_arm, the top level project has to provide:
# - target_link_libraries(gpcc PRIVATE <library containing ChibiOS/RT kernel>)
#
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(gpcc LANGUAGES CXX)

include(common.cmake)

# ---------------------------------------------------------------------------------------------------------------------
# User setting "GPCC_TargetEnvironment"
# ---------------------------------------------------------------------------------------------------------------------
set(GPCC_TargetEnvironment CACHE STRING "Selects between productive use and unit test environment.")
set(GPCC_TargetEnvironmentValues "productive;unittest")

string(REPLACE ";"  ", " GPCC_TargetEnvironmentValues_str "${GPCC_TargetEnvironmentValues}")
set_property(CACHE GPCC_TargetEnvironment PROPERTY STRINGS ${GPCC_TargetEnvironmentValues})

# ---------------------------------------------------------------------------------------------------------------------
# User setting "GPCC_Compiler"
# ---------------------------------------------------------------------------------------------------------------------
set(GPCC_Compiler CACHE STRING "Compiler in use.")
set(GPCC_CompilerValues "gcc_arm;gcc_x64")

string(REPLACE ";"  ", " compilerValues_str "${GPCC_CompilerValues}")
set_property(CACHE GPCC_Compiler PROPERTY STRINGS ${GPCC_CompilerValues})

# ---------------------------------------------------------------------------------------------------------------------
# User setting "GPCC_OS"
# ---------------------------------------------------------------------------------------------------------------------
set(GPCC_OS CACHE STRING "Operating system present on the target where GPCC will be used.")
set(GPCC_OSValues "chibios_arm;linux_arm;linux_arm_tfc;linux_x64;linux_x64_tfc")

string(REPLACE ";"  ", " osValues_str "${GPCC_OSValues}")
set_property(CACHE GPCC_OS PROPERTY STRINGS ${GPCC_OSValues})



# ---------------------------------------------------------------------------------------------------------------------
# Ensure user settings have valid values
# ---------------------------------------------------------------------------------------------------------------------
if(NOT GPCC_TargetEnvironment)
  message(FATAL_ERROR "Error: 'GPCC_TargetEnvironment' is not set.\n"
                      "Allowed values: ${GPCC_TargetEnvironmentValues_str}")
endif()
if(NOT (${GPCC_TargetEnvironment} IN_LIST GPCC_TargetEnvironmentValues))
  message(FATAL_ERROR "Error: 'GPCC_TargetEnvironment' has invalid value.\n"
                      "Allowed values: ${GPCC_TargetEnvironmentValues_str}")
endif()

if(NOT GPCC_Compiler)
  message(FATAL_ERROR "Error: 'GPCC_Compiler' is not set.\n"
                      "Allowed values: ${GPCC_CompilerValues}")
endif()
if(NOT (${GPCC_Compiler} IN_LIST GPCC_CompilerValues))
  message(FATAL_ERROR "Error: 'GPCC_Compiler' has invalid value.\n"
                      "Allowed values: ${GPCC_CompilerValues}")
endif()

Validate_GPCC_Compiler()

if(NOT GPCC_OS)
  message(FATAL_ERROR "Error: 'GPCC_OS' is not set.\n"
                      "Allowed values: ${GPCC_OSValues}")
endif()
if(NOT (${GPCC_OS} IN_LIST GPCC_OSValues))
  message(FATAL_ERROR "Error: 'GPCC_OS' has invalid value.\n"
                      "Allowed values: ${GPCC_OSValues}")
endif()


# ---------------------------------------------------------------------------------------------------------------------
# Continue in cmake files specific to the target environment
# ---------------------------------------------------------------------------------------------------------------------
include(CMakeLists.${GPCC_TargetEnvironment}.cmake)
