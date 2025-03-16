#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script executes the all non-death unittests with valgrind/memcheck.
#
# Invocation:
# ./execute_unittests_memcheck.sh [args]
#
# <args> is an optional parameter that is passed to the unittest executable.
# It could be used to configure a filter:
# ./execute_unittests_memcheck.sh --gtest_filter=Testsuite.Testcase
#
# If <args> is not present, then a filter excluding all death-tests will be passed to the executable.

set -e

if [ $# -eq 0 ]; then

  cd ../build_unittest/output
  valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,indirect,possible ./unittests --gtest_filter=-*Death*

else

  cd ../build_unittest/output
  valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,indirect,possible ./unittests "$@"

fi
