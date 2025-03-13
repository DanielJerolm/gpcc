# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2022, 2024, 2025 Daniel Jerolm

function(GuessUserSettings)
  # This function guesses GPCC_Compiler and GPCC_OS

  # Guess compiler
  if(CMAKE_COMPILER_IS_GNUCXX)
    if((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR
       (CMAKE_SYSTEM_PROCESSOR STREQUAL "x64"))
      set(GPCC_Compiler "gcc_x64" CACHE STRING "" FORCE)
    elseif((CMAKE_SYSTEM_PROCESSOR STREQUAL "arm") OR
           (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64") OR
           (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))
      set(GPCC_Compiler "gcc_arm" CACHE STRING "" FORCE)
    else()
      message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported processor")
    endif()
  else()
    message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported compiler")
  endif()

  # Guess OS
  if(GPCC_TargetEnvironment STREQUAL "productive")

    if(CMAKE_SYSTEM_NAME STREQUAL "chibios")
      if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
        set(GPCC_OS "chibios_arm" CACHE STRING "" FORCE)
      else()
        message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported processor")
      endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "epos")
      if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
        set(GPCC_OS "epos_arm" CACHE STRING "" FORCE)
      else()
        message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported processor")
      endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      if((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR
         (CMAKE_SYSTEM_PROCESSOR STREQUAL "x64"))
        set(GPCC_OS "linux_x64" CACHE STRING "" FORCE)
      elseif((CMAKE_SYSTEM_PROCESSOR STREQUAL "arm") OR
             (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64") OR
             (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))
        set(GPCC_Compiler "linux_arm" CACHE STRING "" FORCE)
      else()
        message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported processor")
      endif()
    else()
      message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported OS")
    endif()

  else()

    if(CMAKE_SYSTEM_NAME STREQUAL "chibios")
      message(FATAL_ERROR "Cannot guess settings: OS 'chibios' is not supported in unittest environment")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "epos")
      message(FATAL_ERROR "Cannot guess settings: OS 'epos' is not supported in unittest environment")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      if((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR
         (CMAKE_SYSTEM_PROCESSOR STREQUAL "x64"))
        set(GPCC_OS "linux_x64_tfc" CACHE STRING "" FORCE)
      elseif((CMAKE_SYSTEM_PROCESSOR STREQUAL "arm") OR
             (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64") OR
             (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64"))
        set(GPCC_Compiler "linux_arm_tfc" CACHE STRING "" FORCE)
      else()
        message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported processor")
      endif()
    else()
      message(FATAL_ERROR "Cannot guess settings: Unknown/unsupported OS")
    endif()

  endif()

endfunction()


function(CheckUserSettings)
  # This checks that the user settings...
  # - GPCC_TargetEnvironment
  # - GPCC_Compiler
  # - GPCC_OS
  #... are set and that they have valid values.

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

  ValidateCompilerInUse()

  if(NOT GPCC_OS)
    message(FATAL_ERROR "Error: 'GPCC_OS' is not set.\n"
                        "Allowed values: ${GPCC_OSValues}")
  endif()
  if(NOT (${GPCC_OS} IN_LIST GPCC_OSValues))
    message(FATAL_ERROR "Error: 'GPCC_OS' has invalid value.\n"
                        "Allowed values: ${GPCC_OSValues}")
  endif()

  # exclude operating systems that are not supported in the productive/unittest environment
  if(${GPCC_TargetEnvironment} STREQUAL "productive")

    if(${GPCC_OS} STREQUAL "linux_arm_tfc")
      message(FATAL_ERROR "Error: 'GPCC_OS=linux_arm_tfc' is not supported for the productive environment.")
    elseif(${GPCC_OS} STREQUAL "linux_x64_tfc")
      message(FATAL_ERROR "Error: 'GPCC_OS=linux_x64_tfc' is not supported for the productive environment.")
    endif()

  else()

    if(${GPCC_OS} STREQUAL "chibios_arm")
      message(FATAL_ERROR "Error: 'GPCC_OS=chibios_arm' is not supported for the unittest environment.")
    elseif(${GPCC_OS} STREQUAL "epos_arm")
      message(FATAL_ERROR "Error: 'GPCC_OS=epos_arm' is not supported for the unittest environment.")
    endif()

  endif()

endfunction()


function(ValidateCompilerInUse)
  # This checks that the compiler in use matches the user setting "GPCC_Compiler".

  if(${GPCC_Compiler} STREQUAL "gcc_arm")
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
      message(FATAL_ERROR "Error: 'GPCC_Compiler' is 'gcc_arm', but the current compiler is not the gnu compiler!")
    endif()
  elseif(${GPCC_Compiler} STREQUAL "gcc_x64")
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
      message(FATAL_ERROR "Error: 'GPCC_Compiler' is 'gcc_x64', but the current compiler is not the gnu compiler!")
    endif()
  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_Compiler' is not supported by function 'ValidateCompilerInUse'.")
  endif()

endfunction()


function(ValidateSkipTestOptions)
  # This checks the GPCC_Skip*Tests user options

  # If TFC-based tests are not skipped, then an OSAL with TFC is mandatory
  if((NOT GPCC_SkipTFCBasedTests) AND
     (NOT (${GPCC_OS} STREQUAL "linux_arm_tfc")) AND
     (NOT (${GPCC_OS} STREQUAL "linux_x64_tfc")))
    message(WARNING "TFC is not present and unittests that rely on TFC are not excluded!\n"
                    "Check options 'GPCC_SkipTFCBasedTests' and 'GPCC_OS'.")
  endif()

endfunction()


function(SetupDefines target)
  # This sets up the #defines that specify the environment and selected configuration options.

  if(${GPCC_Compiler} STREQUAL "gcc_arm")
    target_compile_definitions(${target} PUBLIC COMPILER_GCC_ARM)
  elseif(${GPCC_Compiler} STREQUAL "gcc_x64")
    target_compile_definitions(${target} PUBLIC COMPILER_GCC_X64)
  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_Compiler' is not supported by function 'SetupDefines'.")
  endif()

  if(${GPCC_OS} STREQUAL "chibios_arm")
    target_compile_definitions(${target} PUBLIC OS_CHIBIOS_ARM)
  elseif(${GPCC_OS} STREQUAL "epos_arm")
    target_compile_definitions(${target} PUBLIC OS_EPOS_ARM)
  elseif(${GPCC_OS} STREQUAL "linux_arm")
    target_compile_definitions(${target} PUBLIC OS_LINUX_ARM)
  elseif(${GPCC_OS} STREQUAL "linux_arm_tfc")
    target_compile_definitions(${target} PUBLIC OS_LINUX_ARM_TFC)
  elseif(${GPCC_OS} STREQUAL "linux_x64")
    target_compile_definitions(${target} PUBLIC OS_LINUX_X64)
  elseif(${GPCC_OS} STREQUAL "linux_x64_tfc")
    target_compile_definitions(${target} PUBLIC OS_LINUX_X64_TFC)
  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_OS' is not supported by function 'SetupDefines'.")
  endif()

  if(GPCC_CliNoFontStyles)
    target_compile_definitions(${target} PUBLIC GPCC_CLI_NO_FONT_STYLES)
  endif()

endfunction()


function(SetupDefinesForSkippingUnitTests target)
  # This sets up the #defines indicating which types of unittests shall be excluded from the build.

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
    # The top-level project will provide the required linkage to a library containing ChibiOS/RT
    # and a suitable C++ runtime.

  elseif(${GPCC_OS} STREQUAL "epos_arm")
    target_link_libraries(${target} PUBLIC epos_kernel)

  elseif((${GPCC_OS} STREQUAL "linux_arm") OR
         (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
         (${GPCC_OS} STREQUAL "linux_x64") OR
         (${GPCC_OS} STREQUAL "linux_x64_tfc"))

    find_package(Threads)
    if (NOT CMAKE_USE_PTHREADS_INIT)
      message(FATAL_ERROR "Error: Couldn't find package 'Threads' (pthreads)!")
    endif()

    target_link_libraries(${target} PUBLIC Threads::Threads PRIVATE rt)

  else()
    message(FATAL_ERROR "Error: Value of 'GPCC_OS' is not supported by function 'SetupLinkLibraries'.")
  endif()

  if(${GPCC_TargetEnvironment} STREQUAL "unittest")
    # gpcc and gpcc_testcases both need gmock in the unittest environment
    target_link_libraries(${target} PRIVATE gmock)
  endif()

endfunction()


function(SetCompilerAndLanguageOptions target)
  # This sets up the compiler and C/C++ language options used by GPCC.

  target_compile_features(${target} PUBLIC cxx_std_17)

  if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${target} PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>")
    target_compile_options(${target} PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:-frtti>")

    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:C>:-Wall>")
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:C>:-Wextra>")
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>")

    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-Wall>")
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-Wextra>")

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
      # reduce length of filenames in debug messages contained in the libs
      target_compile_options(${target} PRIVATE -fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=gpcc/)
    endif()
  else()
    message(FATAL_ERROR "Error: Compiler in use is not supported by function 'SetCompilerAndLanguageOptions'.")
  endif()

endfunction()
