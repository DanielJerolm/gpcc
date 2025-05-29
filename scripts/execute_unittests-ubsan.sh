#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script executes all unittests that have been build using the CMake configuration with
# GCC's Undefined Behaviour Sanitizer (UBSan).
#
# Invocation:
# ./execute_unittests-ubsan.sh [args]
#
# <args> is an optional parameter that is passed to the unittest executable.
# It could be used to configure a filter:
# ./execute_unittests-ubsan.sh --gtest_filter=Testsuite.Testcase


set -e

cd ../build_unittest-ubsan/output
./unittests "$@"
