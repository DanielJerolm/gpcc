# Configuration options specific to the unittest environment

## Background
Most unittests from GPCC are single-threaded and their results do not depend on scheduling or CPU load. A couple of unittests is multithreaded and requires an OSAL with TFC to produce reproducible results.

Last but not least, there are a few "special" testcases:
- Testcases that cannot make use of TFC and which depend on CPU load of the machine executing the tests. These tests do not produce stable results on a build-server.
- Testcases that require a significant amount of RAM.
- Testcases that require special user permissions.

The share of these test cases among all test cases is approx. 1.1%.  
GPCC offers a set of options to disable the "special" test cases selectively.

The "special" tests shall be executed from time to time manually on an idle machine, e.g. before pushing a new release version to the master branch.

For more details, please refer to the _GPCC coding style_ embedded in the doxygen documentation, chapter "Unittests with googletest".

## Options
__GPCC_SkipTFCBasedTests__  
Excludes unit-tests from compilation, that require presence of TFC.  
The affected tests may be executed without TFC, but they will likely show a load and scheduling dependency and will not
produce stable results. Some of these tests will only be excluded, if `GPCC_SkipLoadDependentTests` is also set.

__GPCC_SkipLoadDependentTests__  
Excludes unit-tests from compilation that depend on machine load and scheduling and that cannot make use of TFC.

__GPCC_SkipVeryBigMemTests__  
Excludes unit-tests from compilation, that require a significant amount of RAM.  
These tests sometimes have issues in conjunction with valgrind/memcheck or limited system resources.

__GPCC_SkipSpecialRightsBasedTests__  
Excludes unit-tests from compilation that require special user permissions.

## Recommended configuration for reproducible results
Option                           | TFC present *) | TFC not present
-------------------------------- | -------------- | ---------------
GPCC_SkipTFCBasedTests           | OFF *)         | ON
GPCC_SkipLoadDependentTests      | ON *)          | ON
GPCC_SkipVeryBigMemTests         | ON *)          | ON
GPCC_SkipSpecialRightsBasedTests | ON *)          | ON

*) These are the defaults.

## Recommended configuration for additional tests of low-level functionality that require a light-loaded machine
Option                           | value
-------------------------------- | -----
TFC present                      | no
GPCC_SkipTFCBasedTests           | ON
GPCC_SkipLoadDependentTests      | OFF
GPCC_SkipVeryBigMemTests         | OFF
GPCC_SkipSpecialRightsBasedTests | OFF

Note:
- During the build process, warnings indicating that testcases have been skipped due to absence of TFC may be encountered.
- The execution time of the unittests is significantly larger due to absence of TFC (approx. x100).
- Testcases may fail due to missing user permissions.

## Other options
__GPCC_CliNoFontStyles__  
Disables CLI font style control. This is mandatory when building for the unittest environment.

__GPCC_BuildEmptyTestCaseLibrary__  
Builds an empty test case library `gpcc_testcases`.  
This is useful to exclude GPCC's unittest cases if GPCC is build as a sub-project and if the top-project links `gpcc_testcases` into its unittest executable. The better approach may be not to link with `gpcc_testcases`, but this does not always suppress building the library.
