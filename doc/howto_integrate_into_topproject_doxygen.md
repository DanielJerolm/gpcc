# How to integrate GPCC's documentation into a top level project's doxygen
GPCC is documented using doxygen.
Typically GPCC is a subproject in a top project.

## Generate GPCC-only documentation
You can create the documentation for GPCC using one of the scripts located in `./gpcc/scripts`. There are different
scripts for the different CPU and OS configurations available. All scripts should result in zero errors and zero
warnings.

## Integrate into top level project
Alternatively you can integrate GPCC's documentation seamlessly into the doxygen documentation of the top project.
To accomplish this, bear the following in mind regarding the doxygen file of the top-level project:

__Section "INPUT"__  
The following must be included:  
```
INPUT = some_other_dir \
        gpcc/include \
        gpcc/src \
        gpcc/test_src \
        gpcc/testcases
```
Note that the pathes may need a prefix depending on the structure of the top level project.

__Section "IMAGE_PATH"__  
The following must be included:  
```
IMAGE_PATH = some_other_dir \
             gpcc/doc/figures
```
Note that the pathes may need a prefix depending on the structure of the top level project.

__Section "PREDEFINED"__  
Depending on the compiler, one of the following must be included:  
`COMPILER_GCC_ARM` or `COMPILER_GCC_X64`

Depending on the operating system, one of the following must be included:  
`OS_CHIBIOS_ARM`, `OS_EPOS_ARM`, `OS_LINUX_X64`, `OS_LINUX_X64_TFC`, `OS_LINUX_ARM` or `OS_LINUX_ARM_TFC`.

The following should always be included:  
`__DOXYGEN__`

Example:  
```
PREDEFINED = COMPILER_GCC_ARM \
             OS_LINUX_ARM_TFC \
             __DOXYGEN__
```

__Section "FILE_PATTERNS"__  
The following patterns should be included:  
*.cpp *.hpp *.dox *.tcc

Example:  
```
FILE_PATTERNS = *.cpp \
                *.hpp \
                *.dox \
                *.tcc
```

__Other settings__  
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
