/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

===========================
GPCC Configuration Switches
===========================

Note: This document applies to the bare source code.
      When using cmake, then the defines will be setup by GPCC's cmake project files.

Defines, which must be setup when compiling source code
-------------------------------------------------------
COMPILER-related: -DCOMPILER_GCC_ARM or -DCOMPILER_GCC_X64
OS-Related......: -DOS_LINUX_X64 or -DOS_LINUX_X64_TFC or -DOS_LINUX_ARM or -DOS_LINUX_ARM_TFC or -DOS_CHIBIOS_ARM

Defines, which can be used to disable certain types of unit tests
-----------------------------------------------------------------
-DSKIP_VERYBIGMEM_TESTS
 Excludes unit-tests from compilation, that require a large amount of RAM.
 These tests sometimes have issues in conjunction with valgrind/memcheck.

-DSKIP_TFC_BASED_TESTS
 Excludes unit-tests from compilation, that require presence of TFC.
 The affected tests may be executed without TFC, but they will likely fail and they will likely not provide reproducible
 results.

-DSKIP_LOAD_DEPENDENT_TESTS
 Excludes unit-tests from compilation that depend on machine performance and that cannot make use of TFC.

-DSKIP_SPECIAL_RIGHTS_BASED_TESTS
 Excludes unit-tests from compilation that require special user-rights.

Recommended configuration for reproducible results:
                             | TFC present    | TFC not present
  ---------------------------------------------------------------
  SKIP_TFC_BASED_TESTS       | not defined    | defined
  SKIP_LOAD_DEPENDENT_TESTS  | defined        | defined

Recommended configuration for additional tests of low-level functionality that require a light-loaded machine:
                             | TFC present    | TFC not present
  ---------------------------------------------------------------
  SKIP_TFC_BASED_TESTS       | not defined    | defined
  SKIP_LOAD_DEPENDENT_TESTS  | not defined    | not defined


Other defines
-------------
-DGPCC_CLI_NO_FONT_STYLES
 Disables CLI font style control. This is recommended when building a unit test executable or if your terminal does not
 support the color and font style control patterns defined in gpcc/src/cli/CLIColors.hpp
