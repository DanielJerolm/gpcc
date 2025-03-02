#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script builds, rebuilds, or cleans the unittest library and the unittest executable.
#
# Invocation:
# ./build_unittest (clean | all | rebuild)

set -e

# ensure that exactly one argument has been passed to the script
if [ $# -eq 0 ]; then
  echo "One argument expected: 'clean' or 'all' or 'rebuild'"
  exit 1
fi

# ensure that the build-folder exists
if [ ! -d "../build_unittest" ]; then
  echo "Build-folder does not exist."
  echo "Did you run 'cmake_config_linux-unittest-debug.sh' or 'cmake_config_linux-unittest-release.sh'?"
  exit 1
fi

./build_x.sh build_unittest $1
