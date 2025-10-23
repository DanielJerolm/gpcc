#!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2019, 2024, 2025 Daniel Jerolm


# This script will rebuild the doxygen documentation for GPCC for all operating system configurations with and without
# private members extracted.
#
# The script must be executed from gpcc/scripts/
#
# The output will be:
# gpcc/build_doxygen_${CONFIGURATION}/html
# gpcc/build_doxygen_${CONFIGURATION}/warnings.log

set -e

CONFIGURATIONS="chibios_arm chibios_arm_extract_all
                epos_arm epos_arm_extract_all
                linux_arm linux_arm_extract_all
                linux_arm_tfc linux_arm_tfc_extract_all
                linux_x64 linux_x64_extract_all
                linux_x64_tfc linux_x64_tfc_extract_all"

echo "Remove existing documentation"
echo "================================================================================"
rm -fr ../build_doxygen_*

echo

cd ../doc/doxygen
for CONFIGURATION in ${CONFIGURATIONS}
do
  echo "Building doxygen for \"${CONFIGURATION}\""
  echo "================================================================================"
  doxygen doxyfile_${CONFIGURATION}
  echo
done

echo "Summary of warnings:"
echo "================================================================================"

cd ../..
for CONFIGURATION in ${CONFIGURATIONS}
do
  echo "build_doxygen_${CONFIGURATION}/warnings.log:"
  echo "---------------------------------------------------"
  cat build_doxygen_${CONFIGURATION}/warnings.log
  echo
done

echo DONE
