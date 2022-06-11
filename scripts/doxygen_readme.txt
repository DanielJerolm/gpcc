=============================================================================
Creating new doxygen control files for GPCC, or for a project containing GPCC
=============================================================================

The following counts:

- Section "INPUT"
  The following must be included: ?/gpcc/src \
                                  ?/gpcc/test_src/fakes \
                                  ?/gpcc/test_src/test_src.dox

- Section "IMAGE_PATH"
  The following must be included: ?/gpcc/doc/figures

- Section "PREDEFINED"
  See file "readme.txt" for a list of configuration switches. The configuration switches used
  to build your GPCC OS/platform port must be visible to doxygen. Depending on the switches,
  the following DEFINES must be listed in "PREDEFINED":
  - COMPILER_GCC_ARM or COMPILER_GCC_X64
  - OS_LINUX_X64 or OS_LINUX_X64_TFC or OS_LINUX_ARM or OS_LINUX_ARM_TFC or OS_CHIBIOS_ARM
  - __DOXYGEN__
