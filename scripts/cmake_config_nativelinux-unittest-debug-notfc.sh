#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm

set -e

cd ..

FontReset='\033[0m'
FontBoldRed='\033[1;91m'

cmake -S . \
      -B build_unittest \
      -DGPCC_TargetEnvironment:STRING=unittest \
      -DGPCC_Compiler:STRING=gcc_x64 \
      -DGPCC_OS:STRING=linux_x64 \
      -DGPCC_SkipTFCBasedTests=OFF \
      -DGPCC_SkipLoadDependentTests=OFF \
      -DGPCC_SkipVeryBigMemTests=OFF \
      -DGPCC_SkipSpecialRightsBasedTests=OFF \
      -DCMAKE_BUILD_TYPE=Debug

echo "Done"
echo
echo -e "${FontBoldRed}=================================================================="
echo                  "= Note: GPCC has been configured to be build with load-dependent ="
echo                  "= unittests. This configuration requires an idle machine and is  ="
echo                  "= intended for manual use only. It is not suitablable for        ="
echo                  "= automated CI/CD pipelines.                                     ="
echo -e               "==================================================================${FontReset}"
