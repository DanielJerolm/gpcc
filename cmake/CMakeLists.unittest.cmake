# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2022, 2024 Daniel Jerolm


# Sub-CMakeLists-file for GPCC, specific for the "unittest" target environment
# ============================================================================
# Note: Current working directory is GPCC's root directory

# ---------------------------------------------------------------------------------------------------------------------
# Option "GPCC_CliNoFontStyles" (always ON in unittest environment)
# ---------------------------------------------------------------------------------------------------------------------
set(GPCC_CliNoFontStyles ON CACHE BOOL "" FORCE)

# ---------------------------------------------------------------------------------------------------------------------
# Option "GPCC_BuildEmptyTestCaseLibrary"
# ---------------------------------------------------------------------------------------------------------------------
option(GPCC_BuildEmptyTestCaseLibrary "Builds an empty test case library 'gpcc_testcases'." OFF)

# ---------------------------------------------------------------------------------------------------------------------
# Options "GPCC_Skip*Tests"
# ---------------------------------------------------------------------------------------------------------------------
option(GPCC_SkipTFCBasedTests "Excludes unit-tests from compilation, that require presence of TFC." OFF)
option(GPCC_SkipLoadDependentTests "Excludes unit-tests from compilation that depend on machine performance and that cannot make use of TFC." ON)
option(GPCC_SkipVeryBigMemTests "Excludes unit-tests from compilation, that require a large amount of RAM." ON)
option(GPCC_SkipSpecialRightsBasedTests "Excludes unit-tests from compilation that require special user-rights." ON)

ValidateSkipOptions()



# ---------------------------------------------------------------------------------------------------------------------
# Artifact: gpcc library for use in unittest environment
# ---------------------------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME})

add_subdirectory(src)
add_subdirectory(test_src)

target_include_directories(${PROJECT_NAME} PRIVATE . PUBLIC include)

SetupBasicDefines(${PROJECT_NAME})
SetRequiredCompilerOptionsAndFeatures(${PROJECT_NAME})
SetupLinkLibraries(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE gmock)

# ---------------------------------------------------------------------------------------------------------------------
# Artifact: gpcc_testcases object library containing unit test cases for library "gpcc"
# ---------------------------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME}_testcases OBJECT)

add_subdirectory(testcases)

target_include_directories(${PROJECT_NAME}_testcases PRIVATE .)

if(${GPCC_OS} STREQUAL "chibios_arm")
  message(FATAL_ERROR "Error: 'GPCC_OS=chibios_arm' not supported for unit test environment.")
elseif(${GPCC_OS} STREQUAL "epos_arm")
  message(FATAL_ERROR "Error: 'GPCC_OS=epos_arm' not supported for unit test environment.")
endif()

# SetupBasicDefines() is not required. All defines made there are public and will be pulled in from library "gpcc".

SetupDefinesForSkippingUnitTests(${PROJECT_NAME}_testcases)
SetRequiredCompilerOptionsAndFeatures(${PROJECT_NAME}_testcases)
SetupLinkLibraries(${PROJECT_NAME}_testcases)

target_link_libraries(${PROJECT_NAME}_testcases PRIVATE ${PROJECT_NAME} gmock)
