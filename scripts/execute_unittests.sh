#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script executes all unittests from the ./build_unittest folder.
#
# Invocation:
# ./execute_unittests.sh [args]
#
# <args> is an optional parameter that is passed to the unittest executable.
# It could be used to configure a filter:
# ./execute_unittests.sh --gtest_filter=Testsuite.Testcase

set -e

# ensure that the unittest executable is existing
if [ ! -f "../build_unittest/output/unittests" ]; then
  echo "The unittest executable is not existing."
  echo "Did you run 'cmake_config_unittest_*.sh' and 'build_unittest.sh' ?"
  exit 1
fi

cd ../build_unittest/output
./unittests $@
