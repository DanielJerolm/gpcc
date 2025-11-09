# How to integrate GPCC's doxygen documentation into another project

GPCC is documented using doxygen.

Typically GPCC is a subproject in a top project. In some cases the top project may have multiple subprojects. If these projects are documented using doxygen, then they may want to refer to classes from GPCC in their documentation (e.g. via @ref).

This document explains how to link a project's documentation to the documentation of GPCC using doxygen tag-files.

Note that there are two approaches.

## Approach 1 (Top project does not use CMake to build its documentation)

__Prerequisites:__  
- A doxygen documentation for GPCC matching the CPU and OS configuration used by the top level project has been build within the gpcc subproject.  
This can be accomplished e.g. using the scripts in gpcc/scripts. See [Build doxygen documentation](../README.md#Build-doxygen-documentation) in GPCC's README.md for details.

__Implementation:__  
A reference to the tag-file generated together with GPCC's HTML documentation is added to the `TAGFILES` entry in the top project's doxygen control file.  

Example:  
Assume the following directory structure:
~~~
top_project
  +-- doc
  |     +-- html  (B1)
  +-- extern
  |     +-- gpcc
  |     |     +-- build_doxygen_linux_x64
  |    ...    |     +-- html      (B2)
  |          ...    +-- gpcc.tag  (A2)
  +-- scripts (A1)
  |     +-- doxyfile
 ...
~~~

The `TAGFILES` entry in the top project's doxygen file must be composed as follows:  
`TAGFILES = <path from A1 to A2>=<path from B1 to B2>`  

Example matching the directory structure shown above:
~~~
TAGFILES = ../extern/gpcc/build_doxygen_linux_x64/gpcc.tag=../../extern/gpcc/build_doxygen_linux_x64/html
~~~

## Approach 2 (Top project uses CMake to build its documentation)

GPCC provides a build target `gpcc_doxygen` for its doxygen documentation and GPCC exposes a CMake variable `GPCC_DOXYGEN_OUT` to the parent project, which contains the location of the generated documentation.

Example of top project's CMakeLists.txt file:  
~~~
find_package(doxygen)

# [...]

add_subdirectory(extern/gpcc)

# [...]

set(DOXYGEN_TAGFILES "${GPCC_DOXYGEN_OUT}/gpcc.tag=${GPCC_DOXYGEN_OUT}/html")
doxygen_add_docs(top_project_dox
                 include
                 src)

# ensure GPCC's documentation is build before top_project_dox
add_dependencies(top_project_dox gpcc_doxygen)
~~~
