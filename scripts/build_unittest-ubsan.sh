#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script builds, rebuilds, or cleans the unittest library and the unittest executable with UBSan.
#
# Invocation:
# ./build_unittest-ubsan (clean | all | rebuild)

set -e

# ensure that exactly one argument has been passed to the script
if [ $# -eq 0 ]; then
  echo "One argument expected: 'clean' or 'all' or 'rebuild'"
  exit 1
fi

# ensure that the build-folder exists
if [ ! -d "../build_unittest-ubsan" ]; then
  echo "Build-folder does not exist."
  echo "Did you run 'cmake_config_nativelinux-unittest-ubsan.sh'?"
  exit 1
fi

./build_x.sh build_unittest-ubsan $1
