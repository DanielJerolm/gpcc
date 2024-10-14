/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019, 2024 Daniel Jerolm
*/

===========================
GPCC Configuration Switches
===========================

Note: This document applies to the bare source code.
      When using cmake, then the defines will be setup by GPCC's cmake project files.

Defines, which must be setup when compiling source code
-------------------------------------------------------
COMPILER-related: -DCOMPILER_GCC_ARM or -DCOMPILER_GCC_X64
OS-Related......: -DOS_CHIBIOS_ARM or -DOS_EPOS_ARM or
                  -DOS_LINUX_ARM or -DOS_LINUX_ARM_TFC or
                  -DOS_LINUX_X64 or -DOS_LINUX_X64_TFC

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
