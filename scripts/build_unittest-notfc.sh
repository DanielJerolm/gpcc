#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script performs a build-, rebuild-, or clean-operation in the ./build_unittest-notfc folder.
#
# Invocation:
# ./build_unittest-notfc.sh (clean | all | rebuild)

set -e

# ensure that exactly one argument has been passed to the script
if [ $# -ne 1 ]; then
  echo "One argument expected: 'clean' or 'all' or 'rebuild'"
  exit 1
fi

# ensure that the build-folder exists
if [ ! -d "../build_unittest-notfc" ]; then
  echo "Build-folder does not exist."
  echo "Did you run any of the 'cmake_config_unittest-notfc_*.sh' scripts?"
  exit 1
fi

./build_x.sh build_unittest-notfc $1
