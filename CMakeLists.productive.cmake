# Sub-CMakeLists-file for GPCC, specific for the "productive" target environment
# ==============================================================================

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
