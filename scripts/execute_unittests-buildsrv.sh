#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script executes all unittests from the ./build_unittest-buildsrv folder.
#
# Invocation:
# ./execute_unittests-buildsrv.sh [args]
#
# <args> is an optional parameter that is passed to the unittest executable.
# It could be used to configure a filter:
# ./execute_unittests-buildsrv.sh --gtest_filter=Testsuite.Testcase
#

set -e

# ensure that the unittest executable is existing
if [ ! -f "../build_unittest-buildsrv/output/unittests" ]; then
  echo "The unittest executable is not existing."
  echo "Did you run 'cmake_config_unittest-buildsrv_*.sh' and 'build_unittest-buildsrv.sh' ?"
  exit 1
fi

if [ $# -eq 0 ]; then

  cd ../build_unittest-buildsrv/output

  # Run death tests without memcheck
  ./unittests --gtest_filter=*Death*

  # Run all non-death tests with memcheck
  valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,indirect,possible ./unittests --gtest_filter=-*Death*

else

  cd ../build_unittest-buildsrv/output
  valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,indirect,possible ./unittests $@

fi
