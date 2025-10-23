#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This script performs a build-operation in the ./build_unittest-buildsrv folder.
#
# Invocation:
# ./build_unittest-buildsrv.sh (clean | all | rebuild | dox)

set -e

# ensure that exactly one argument has been passed to the script
if [ $# -ne 1 ]; then
  echo "One argument expected: 'clean', 'all', 'rebuild', or 'dox'"
  exit 1
fi

# ensure that the build-folder exists
if [ ! -d "../build_unittest-buildsrv" ]; then
  echo "Build-folder does not exist."
  echo "Did you run any of the 'cmake_config_unittest-buildsrv_*.sh' scripts?"
  exit 1
fi

./build_x.sh build_unittest-buildsrv $1
