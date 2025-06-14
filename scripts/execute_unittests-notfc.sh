#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script executes all unittests from the ./build_unittest-notfc folder.
#
# Invocation:
# ./execute_unittests-notfc.sh [args]
#
# <args> is an optional parameter that is passed to the unittest executable.
# It could be used to configure a filter:
# ./execute_unittests-notfc.sh --gtest_filter=Testsuite.Testcase

set -e

FontReset='\033[0m'
FontBoldRed='\033[1;91m'

# ensure that the unittest executable is existing
if [ ! -f "../build_unittest-notfc/output/unittests" ]; then
  echo "The unittest executable is not existing."
  echo "Did you run 'cmake_config_unittest-notfc_*.sh' and 'build_unittest-notfc.sh' ?"
  exit 1
fi

cd ../build_unittest-notfc/output

set +e
./unittests $@

if [ $? -ne 0 ]; then
  echo -e "${FontBoldRed}====================================================================="
  echo                  "= There is at least one failed test. This is not completely         ="
  echo                  "= unanticipated since additional \"special\" tests are enabled.       ="
  echo                  "= Check the output carefully.                                       ="
  echo                  "= See docs/special_unittest_configuration_options.md for details.   ="
  echo -e               "=====================================================================${FontReset}"
  exit 1
fi
