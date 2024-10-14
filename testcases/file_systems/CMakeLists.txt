# -------------------------------------
# gpcc/testcases/file_systems
# -------------------------------------
add_subdirectory(cli)
add_subdirectory(eeprom_section_system)

if((${GPCC_OS} STREQUAL "linux_arm") OR
   (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  add_subdirectory(linux_fs)
endif()
