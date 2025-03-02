# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2022, 2024, 2025 Daniel Jerolm

# =================================
# Main CMakeLists.txt-file for GPCC
# =================================

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(gpcc LANGUAGES CXX)

include(cmake/common.cmake)

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
set(GPCC_OSValues "chibios_arm;epos_arm;linux_arm;linux_arm_tfc;linux_x64;linux_x64_tfc")

string(REPLACE ";"  ", " osValues_str "${GPCC_OSValues}")
set_property(CACHE GPCC_OS PROPERTY STRINGS ${GPCC_OSValues})


CheckUserSettings()


# ---------------------------------------------------------------------------------------------------------------------
# Option "GPCC_CliNoFontStyles"
# This is always on, if GPCC is configured for the unittest environment.
# ---------------------------------------------------------------------------------------------------------------------
if(${GPCC_TargetEnvironment} STREQUAL "productive")
  option(GPCC_CliNoFontStyles "Disables gpcc::cli::CLI font style control." OFF)
else()
  set(GPCC_CliNoFontStyles ON CACHE BOOL "" FORCE)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# Option "GPCC_BuildEmptyTestCaseLibrary"
# This is only available, if GPCC if configured for the unittest environment.
# This is always OFF, if GPCC is configured stand-alone.
# ---------------------------------------------------------------------------------------------------------------------
if (PROJECT_IS_TOP_LEVEL)
  set(GPCC_BuildEmptyTestCaseLibrary OFF CACHE BOOL "" FORCE)
else()
  option(GPCC_BuildEmptyTestCaseLibrary "Builds an empty test case library 'gpcc_testcases'." ON)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# Options "GPCC_Skip*Tests"
# These are only available, if GPCC is configured for the unittest environment.
# ---------------------------------------------------------------------------------------------------------------------
if(${GPCC_TargetEnvironment} STREQUAL "unittest")
  option(GPCC_SkipTFCBasedTests
         "Excludes unit-tests from compilation, that require presence of TFC."
         OFF)

  option(GPCC_SkipLoadDependentTests
         "Excludes unit-tests from compilation that depend on machine performance and that cannot make use of TFC."
         ON)

  option(GPCC_SkipVeryBigMemTests
         "Excludes unit-tests from compilation, that require a large amount of RAM."
         ON)

  option(GPCC_SkipSpecialRightsBasedTests
         "Excludes unit-tests from compilation that require special user-rights."
         ON)

  ValidateSkipTestOptions()
endif()


# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------
# End of user settings. Let's build...
# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------

if (PROJECT_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# Fetch externals, if GPCC is build stand-alone
# ---------------------------------------------------------------------------------------------------------------------
if (PROJECT_IS_TOP_LEVEL)
  if(${GPCC_TargetEnvironment} STREQUAL "unittest")
    include(FetchContent)

    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      # release 1.12.1
      GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850
    )

    message(STATUS "Fetching googletest from www.github.com")
    FetchContent_MakeAvailable(googletest)
  endif()
endif()



# ---------------------------------------------------------------------------------------------------------------------
# Library "gpcc"
# ---------------------------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME})

if(${GPCC_TargetEnvironment} STREQUAL "productive")
  add_subdirectory(src)
else()
  add_subdirectory(src)
  add_subdirectory(test_src)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE . PUBLIC include)

SetupDefines(${PROJECT_NAME})
SetCompilerAndLanguageOptions(${PROJECT_NAME})
SetupLinkLibraries(${PROJECT_NAME})

# ---------------------------------------------------------------------------------------------------------------------
# Library "gpcc_testcases"
# ---------------------------------------------------------------------------------------------------------------------
if(${GPCC_TargetEnvironment} STREQUAL "unittest")

  add_library(${PROJECT_NAME}_testcases OBJECT)

  add_subdirectory(testcases)

  target_include_directories(${PROJECT_NAME}_testcases PRIVATE .)

  SetupDefines(${PROJECT_NAME}_testcases)
  SetupDefinesForSkippingUnitTests(${PROJECT_NAME}_testcases)
  SetCompilerAndLanguageOptions(${PROJECT_NAME}_testcases)
  SetupLinkLibraries(${PROJECT_NAME}_testcases)

  target_link_libraries(${PROJECT_NAME}_testcases PRIVATE ${PROJECT_NAME} gmock)

endif()

# ---------------------------------------------------------------------------------------------------------------------
# Unittest executable "gpcc_unittests"
# (only available if gpcc is configured for the unittest environment AND if GPCC is configured stand-alone)
# ---------------------------------------------------------------------------------------------------------------------
if((${GPCC_TargetEnvironment} STREQUAL "unittest") AND (PROJECT_IS_TOP_LEVEL))

  add_executable(${PROJECT_NAME}_unittests testcases/empty.cpp)

  target_link_libraries(${PROJECT_NAME}_unittests PRIVATE gpcc_testcases gmock_main)

  set_target_properties(${PROJECT_NAME}_unittests PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output"
                         OUTPUT_NAME "unittests"
                        )
endif()
