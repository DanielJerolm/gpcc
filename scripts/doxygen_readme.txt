=================================================
Hints for creating doxygen documentation for GPCC
=================================================

GPCC is documented using doxygen.

Typically GPCC is a subproject in a top project.

You can create the documentation for GPCC only using one of the control files located in this folder. There are
different files for the different CPU and OS configurations available. All scripts should result in zero errors and
zero warnings.

Alternatively you can integrate GPCC's documentation seamlessly into the doxygen documentation of the top project.
To achieve this, bear the following in mind regarding the doxygen file of the top-level project:

- Section "INPUT"
  The following must be included: gpcc/src \
                                  gpcc/test_src

- Section "IMAGE_PATH"
  The following must be included: gpcc/doc/figures

- Section "PREDEFINED"
  See file "readme_gpcc_configuration.txt" for a list of configuration switches. The configuration switches used
  to build your GPCC OS/platform port must be visible to doxygen. Depending on the switches, the following DEFINES must
  be listed in "PREDEFINED":
  - COMPILER_GCC_ARM or COMPILER_GCC_X64
  - OS_LINUX_X64 or OS_LINUX_X64_TFC or OS_LINUX_ARM or OS_LINUX_ARM_TFC or OS_CHIBIOS_ARM
  - __DOXYGEN__

- Section "FILE_PATTERNS":
  The following should be included: *.cpp \
                                    *.hpp \
                                    *.dox \
                                    *.tcc

Further, the following settings are recommended:
- MULTILINE_CPP_IS_BRIEF = YES
- TAB_SIZE = 2
- AUTOLINK_SUPPORT = NO
- BUILTIN_STL_SUPPORT = YES
- IDL_PROPERTY_SUPPORT = NO
- EXTRACT_ALL = YES
- HIDE_UNDOC_MEMBERS = YES
- HIDE_UNDOC_CLASSES = YES
- HIDE_IN_BODY_DOCS = YES
- FORCE_LOCAL_INCLUDES = YES
- SORT_MEMBERS_CTORS_1ST = YES
- SORT_BY_SCOPE_NAME = YES
- WARN_NO_PARAMDOC = YES
- RECURSIVE = YES
- COLLABORATION_GRAPH = NO
- GROUP_GRAPHS = NO
- INCLUDE_GRAPH = NO
- INCLUDED_BY_GRAPH = NO
- GRAPHICAL_HIERARCHY = NO
- DIRECTORY_GRAPH = NO
