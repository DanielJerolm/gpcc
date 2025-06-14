# Configuration options for to the unittest environment

## Background
Most unittests from GPCC are single-threaded and their results do not depend on scheduling or CPU load. A couple of unittests is multithreaded and requires an OSAL with TFC to produce reproducible results. Using an OSAL with TFC in a unittest build is recommended and is the default when building GPCC standalone.

However, there are a few additional "special" testcases:
- Testcases for synchronization primitives from the OSAL that cannot make use of TFC or that require absence of TFC.  
Some of these tests fail, if execution is delayed by the activity of other processes present on the machine.  
Unfortunately this is inevitable for testing a few corner cases and special scenarios.
- Testcases that require a significant amount of RAM.
- Testcases that require special user permissions.

The share of these test cases among all test cases is approx. 1.1%.  
GPCC offers a set of options to disable the "special" test cases selectively.

The "special" tests shall be executed from time to time manually on an idle machine, e.g. before pushing a new release version to the master branch. A build folder for the "special" tests can be setup using one of the `cmake_config_unittest-notfc_*.sh` scripts from the scripts-folder.

For more details on testcase classification, please refer to the _GPCC coding style_ embedded in the doxygen documentation, chapter "Unittests with googletest".

## Options
__GPCC_SkipTFCBasedTests__  
Excludes unit-tests from compilation, that require presence of TFC for reproducible results.  
Some of these tests will only be excluded, if `GPCC_SkipLoadDependentTests` is also set.

__GPCC_SkipLoadDependentTests__  
Excludes unit-tests from compilation that depend on machine load and scheduling and that cannot make use of TFC.

__GPCC_SkipVeryBigMemTests__  
Excludes unit-tests from compilation, that require a significant amount of RAM.  
These tests sometimes have issues in conjunction with valgrind/memcheck or limited system resources.

__GPCC_SkipSpecialRightsBasedTests__  
Excludes unit-tests from compilation that require special user permissions.

## Recommended configuration for reproducible results
The following settings will always produce reproducible results independent of the machine load:

Option                           | TFC present *) | TFC not present
-------------------------------- | -------------- | ---------------
GPCC_SkipTFCBasedTests           | OFF *)         | ON
GPCC_SkipLoadDependentTests      | ON *)          | ON
GPCC_SkipVeryBigMemTests         | ON *)          | ON
GPCC_SkipSpecialRightsBasedTests | ON *)          | ON

*) These are the defaults.

## Recommended configuration for the additional "special" tests
The following settings will enable the additional "special" tests and disable some tests that require TFC:

Option                           | Value
-------------------------------- | -----
TFC present                      | no
GPCC_SkipTFCBasedTests           | ON
GPCC_SkipLoadDependentTests      | OFF
GPCC_SkipVeryBigMemTests         | OFF
GPCC_SkipSpecialRightsBasedTests | OFF

__Note:__
- During the build process, warnings indicating that testcases have been skipped due to absence of TFC may be encountered.
- The execution time of the unittests is significantly larger due to absence of TFC (approx. x100).
- Some testcases may fail due to missing user permissions (e.g. `gpcc_osal_Thread_TestsF.Start_Policy_SP_*`)
- Some testcases may fail if they are delayed by the activity of other processes on the machine.

There is a separate build-folder and configuration script for this configuration. See `scripts/cmake_config_unittest-notfc_*.sh`.

## Other options
__GPCC_CliNoFontStyles__  
Disables CLI font style control. This is mandatory when building for the unittest environment.

__GPCC_BuildEmptyTestCaseLibrary__  
Builds an empty test case library `gpcc_testcases`.  
This is useful to exclude GPCC's unittest cases if GPCC is build as a sub-project and if the top-project links `gpcc_testcases` into its unittest executable. The better approach may be not to link with `gpcc_testcases`, but unfortunately this does not always suppress building `gpcc_testcases`.
