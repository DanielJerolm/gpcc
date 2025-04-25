# How to integrate GPCC's documentation into a top level project's doxygen
GPCC is documented using doxygen.

Typically GPCC is a subproject in a top project and having access to GPCC's documentation from within the top project is very useful.

## Generate GPCC-only documentation
You can generate the HTML documentation for GPCC using one of the scripts located in `/scripts`. There are different scripts for the different CPU and OS configurations available.

The generated HTML documentation and a doxygen tag-file will be located in `/build_doxygen_*` , * indicating the CPU and OS configuration.

## Integrate into top level project
If the top project imports the tag-file generated together with the GPCC documentation, then the top level project can reference elements from GPCC directly in its doxygen documentation:
~~~
/**
 * \brief   This is a function in the top level project.
 *
 * \param   wq
 * Reference to a [work queue](@ref gpcc::execution::async::WorkQueue) that shall be used as execution context.
 */
~~~

__Prerequisites:__  
- A doxygen documentation for the CPU and OS configuration used by the top level project has been build within the gpcc subproject.  
See chapter "Generate GPCC-only documentation" above.
- The top project's doxygen control file contains a reference to the tag-file generated together with the GPCC HTML documentation.

__Example:__  
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

The `TAGFILES` entry in the top project's doxygen file must be composed as following:  
`TAGFILES = <path from A1 to A2>=<path from B1 to B2>`  

Example matching the directory structure shown above:
~~~
TAGFILES = ../extern/gpcc/build_doxygen_linux_x64/gpcc.tag=../../extern/gpcc/build_doxygen_linux_x64/html
~~~
