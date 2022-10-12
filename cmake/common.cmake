# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2022 Daniel Jerolm


function(Validate_GPCC_Compiler)
  # This checks that the used compiler matches the user setting in "GPCC_Compiler".

  if(${GPCC_Compiler} STREQUAL "gcc_arm")
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
      message(FATAL_ERROR "Error: 'GPCC_Compiler' is 'gcc_arm', but the current compiler is not the gnu compiler!")
    endif()
  elseif(${GPCC_Compiler} STREQUAL "gcc_x64")
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
      message(FATAL_ERROR "Error: 'GPCC_Compiler' is 'gcc_x64', but the current compiler is not the gnu compiler!")
    endif()
  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_Compiler' is not supported by function 'Validate_GPCC_Compiler'.")
  endif()
endfunction()

function(ValidateSkipOptions)
  # This checks the GPCC_Skip*Tests options against user settings.

  if((NOT GPCC_SkipTFCBasedTests) AND
     (NOT (${GPCC_OS} STREQUAL "linux_arm_tfc")) AND
     (NOT (${GPCC_OS} STREQUAL "linux_x64_tfc")))
    message(WARNING "TFC is not present and unit tests that rely on TFC are not excluded!\n"
                    "Check options 'GPCC_SkipTFCBasedTests' and 'GPCC_OS'.")
  endif()
endfunction()

function(SetupBasicDefines target)
  # This sets up the basic #defines indicating the compiler in use, the operating system in use and configuration
  # options.

  if(${GPCC_Compiler} STREQUAL "gcc_arm")
    target_compile_definitions(${target} PUBLIC COMPILER_GCC_ARM)
  elseif(${GPCC_Compiler} STREQUAL "gcc_x64")
    target_compile_definitions(${target} PUBLIC COMPILER_GCC_X64)
  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_Compiler' is not supported by function 'SetupBasicDefines'.")
  endif()

  if(${GPCC_OS} STREQUAL "chibios_arm")
    target_compile_definitions(${target} PUBLIC OS_CHIBIOS_ARM)
  elseif(${GPCC_OS} STREQUAL "linux_arm")
    target_compile_definitions(${target} PUBLIC OS_LINUX_ARM)
  elseif(${GPCC_OS} STREQUAL "linux_arm_tfc")
    target_compile_definitions(${target} PUBLIC OS_LINUX_ARM_TFC)
  elseif(${GPCC_OS} STREQUAL "linux_x64")
    target_compile_definitions(${target} PUBLIC OS_LINUX_X64)
  elseif(${GPCC_OS} STREQUAL "linux_x64_tfc")
    target_compile_definitions(${target} PUBLIC OS_LINUX_X64_TFC)
  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_OS' is not supported by function 'SetupBasicDefines'.")
  endif()

  if(GPCC_CliNoFontStyles)
    target_compile_definitions(${target} PUBLIC GPCC_CLI_NO_FONT_STYLES)
  endif()
endfunction()

function(SetupDefinesForSkippingUnitTests target)
  # This sets up the #defines indicating which types of unit tests shall be excluded from the build.

  if(GPCC_SkipTFCBasedTests)
    target_compile_definitions(${target} PRIVATE SKIP_TFC_BASED_TESTS)
  endif()

  if(GPCC_SkipLoadDependentTests)
    target_compile_definitions(${target} PRIVATE SKIP_LOAD_DEPENDENT_TESTS)
  endif()

  if(GPCC_SkipVeryBigMemTests)
    target_compile_definitions(${target} PRIVATE SKIP_VERYBIGMEM_TESTS)
  endif()

  if(GPCC_SkipSpecialRightsBasedTests)
    target_compile_definitions(${target} PRIVATE SKIP_SPECIAL_RIGHTS_BASED_TESTS)
  endif()
endfunction()

function(SetupLinkLibraries target)
  # This sets up the linkage to other libraries required by GPCC in some configurations.

  if(${GPCC_OS} STREQUAL "chibios_arm")
    # The top-level project will provide the required linkage to a library containing ChibiOS/RT.

  elseif((${GPCC_OS} STREQUAL "linux_arm") OR
         (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
         (${GPCC_OS} STREQUAL "linux_x64") OR
         (${GPCC_OS} STREQUAL "linux_x64_tfc"))

    find_package(Threads)
    if (NOT CMAKE_USE_PTHREADS_INIT)
      message(FATAL_ERROR "Error: Couldn't find package 'Threads' (pthreads)!")
    endif()

    target_link_libraries(${target} PRIVATE Threads::Threads rt)

  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_OS' is not supported by function 'SetupLinkLibraries'.")
  endif()
endfunction()

function(SetRequiredCompilerOptionsAndFeatures target)
  # This sets up the required compiler features and settings.

  target_compile_features(${target} PUBLIC cxx_std_17)

  if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${target} PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>")
    target_compile_options(${target} PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:-frtti>")

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
      # reduce length of file-names in debug messages contained in the libs
      target_compile_options(${target} PRIVATE -fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=gpcc/)
    endif()
  else()
    message(FATAL_ERROR "Error: Used compiler is not supported by function 'SetRequiredCompilerOptionsAndFeatures'.")
  endif()
endfunction()
