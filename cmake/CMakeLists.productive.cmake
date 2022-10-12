# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2022 Daniel Jerolm


# Sub-CMakeLists-file for GPCC, specific for the "productive" target environment
# ==============================================================================
# Note: Current working directory is GPCC's root directory

# ---------------------------------------------------------------------------------------------------------------------
# Option "GPCC_CliNoFontStyles"
# ---------------------------------------------------------------------------------------------------------------------
option(GPCC_CliNoFontStyles "Disables gpcc::cli::CLI font style control." OFF)



# ---------------------------------------------------------------------------------------------------------------------
# Artifact: gpcc library for productive use (src and src_notest included)
# ---------------------------------------------------------------------------------------------------------------------
add_library(${PROJECT_NAME})

add_subdirectory(src)
add_subdirectory(src_notest)

target_include_directories(${PROJECT_NAME} PRIVATE . PUBLIC include)

SetupBasicDefines(${PROJECT_NAME})
SetRequiredCompilerOptionsAndFeatures(${PROJECT_NAME})
SetupLinkLibraries(${PROJECT_NAME})
