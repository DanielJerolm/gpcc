#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm

set -e

cd ../doc/doxygen
doxygen doxyfile_chibios_arm_extract_all

echo "warnings.log:"
cat ../../build_doxygen_chibios_arm_extract_all/warnings.log
