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
set(GPCC_TargetEnvironment CACHE STRING "Selects between productive use and unittest environment.")
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
set(GPCC_OSValues "chibios_arm;epos_arm;linux_arm;linux_arm_tfc;linux_x64;linux_x64_tfc;zephyr")

string(REPLACE ";"  ", " osValues_str "${GPCC_OSValues}")
set_property(CACHE GPCC_OS PROPERTY STRINGS ${GPCC_OSValues})


# guess if target environment is specified, but compiler and OS are not
if((GPCC_TargetEnvironment) AND
   (NOT GPCC_Compiler) AND
   (NOT GPCC_OS))
  message(NOTICE "GPCC: GPCC_Compiler and GPCC_OS are not set. Guessing...")
  GuessUserSettings()
endif()

CheckUserSettings()

# ---------------------------------------------------------------------------------------------------------------------
# User option "GPCC_CliNoFontStyles"
# This is always on, if GPCC is configured for the unittest environment.
# ---------------------------------------------------------------------------------------------------------------------
if(${GPCC_TargetEnvironment} STREQUAL "productive")
  option(GPCC_CliNoFontStyles "Disables gpcc::cli::CLI font style control." OFF)
else()
  set(GPCC_CliNoFontStyles ON CACHE BOOL "" FORCE)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# User option "GPCC_BuildEmptyTestCaseLibrary"
# This is only available, if GPCC if configured for the unittest environment.
# This is always OFF, if GPCC is configured stand-alone.
# ---------------------------------------------------------------------------------------------------------------------
if (PROJECT_IS_TOP_LEVEL)
  set(GPCC_BuildEmptyTestCaseLibrary OFF CACHE BOOL "" FORCE)
else()
  option(GPCC_BuildEmptyTestCaseLibrary "Builds an empty test case library 'gpcc_testcases'." ON)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# User options "GPCC_Skip*Tests"
# These are only available, if GPCC is configured for the unittest environment.
# ---------------------------------------------------------------------------------------------------------------------
if(${GPCC_TargetEnvironment} STREQUAL "unittest")
  option(GPCC_SkipTFCBasedTests
         "Excludes unittests from compilation, that require presence of TFC."
         OFF)

  option(GPCC_SkipLoadDependentTests
         "Excludes unittests from compilation that depend on machine performance and that cannot make use of TFC."
         ON)

  option(GPCC_SkipVeryBigMemTests
         "Excludes unittests from compilation, that require a large amount of RAM."
         ON)

  option(GPCC_SkipSpecialRightsBasedTests
         "Excludes unittests from compilation that require special user-rights."
         ON)

  ValidateSkipTestOptions()
endif()


# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------
# End of user settings and options. Let's build...
# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------------------------------------

message(STATUS "GPCC config: ${GPCC_TargetEnvironment}, ${GPCC_Compiler}, ${GPCC_OS}")

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
      # release 1.16.0
      GIT_TAG 6910c9d9165801d8827d628cb72eb7ea9dd538c5
    )

    message(STATUS "Fetching googletest from www.github.com")
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

# ---------------------------------------------------------------------------------------------------------------------
# Library "gpcc"
# ---------------------------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME})

add_subdirectory(src)

if(${GPCC_TargetEnvironment} STREQUAL "unittest")
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

  target_link_libraries(${PROJECT_NAME}_testcases PRIVATE ${PROJECT_NAME})

endif()

# ---------------------------------------------------------------------------------------------------------------------
# Unittest executable "gpcc_unittests"
# (only available if GPCC is configured for the unittest environment AND if GPCC is configured stand-alone)
# ---------------------------------------------------------------------------------------------------------------------
if((${GPCC_TargetEnvironment} STREQUAL "unittest") AND (PROJECT_IS_TOP_LEVEL))

  add_executable(${PROJECT_NAME}_unittests testcases/empty.cpp)

  target_link_libraries(${PROJECT_NAME}_unittests PRIVATE gpcc_testcases gmock_main)

  set_target_properties(${PROJECT_NAME}_unittests PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output"
                         OUTPUT_NAME "unittests"
                        )
endif()

# ---------------------------------------------------------------------------------------------------------------------
# Documentation (doxygen)
# ---------------------------------------------------------------------------------------------------------------------
find_package(Doxygen 1.9)

if(Doxygen_FOUND)
  set(GPCC_BASE ${CMAKE_CURRENT_LIST_DIR})
  set(GPCC_DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
  if (NOT PROJECT_IS_TOP_LEVEL)
    set(GPCC_DOXYGEN_OUT ${GPCC_DOXYGEN_OUT} PARENT_SCOPE)
  endif()

  SetupDoxygenPredefinedTag(DOXYGEN_PREDEFINED_LIST)
  string(REPLACE ";"  ", " DOXYGEN_PREDEFINED "${DOXYGEN_PREDEFINED_LIST}")

  list(APPEND DOXYGEN_INPUT_LIST "include/gpcc")
  list(APPEND DOXYGEN_INPUT_LIST "src")
  if(${GPCC_TargetEnvironment} STREQUAL "unittest")
    list(APPEND DOXYGEN_INPUT_LIST "include/gpcc_test")
    list(APPEND DOXYGEN_INPUT_LIST "test_src")
    list(APPEND DOXYGEN_INPUT_LIST "testcases")
  endif()
  string(REPLACE ";"  " \\\n                         " DOXYGEN_INPUT "${DOXYGEN_INPUT_LIST}")

  configure_file(doc/doxygen/doxyfile.in ${GPCC_DOXYGEN_OUT}/doxyfile @ONLY)

  add_custom_target(gpcc_doxygen
                    COMMAND doxygen ${GPCC_DOXYGEN_OUT}/doxyfile
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                    COMMENT "Generating doxygen documentation for GPCC\nOutput location: ${GPCC_DOXYGEN_OUT}/html")

  add_custom_command(TARGET gpcc_doxygen
                     POST_BUILD
                     COMMAND cat ${GPCC_DOXYGEN_OUT}/warnings.log)
else()
  message(WARNING "Doxygen not found! Target 'gpcc_doxygen' will not be available.")
endif()
