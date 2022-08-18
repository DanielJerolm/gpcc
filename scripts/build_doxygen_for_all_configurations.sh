!/bin/bash

# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2019 Daniel Jerolm


# This script will generate the doxygen documentation for GPCC for all operating system
# configurations with and without private members extracted.
#
# The script must be executed from gpcc/scripts/
#
# The output will be a set of files and folders for each OS configuration:
# gpcc/doc/doxygen/html_${CONFIGURATION}/
# gpcc/doc/doxygen/warnings_${CONFIGURATION}.log
# gpcc/doc/doxygen/html_${CONFIGURATION}_extract_all/
# gpcc/doc/doxygen/warnings_${CONFIGURATION}_extract_all.log
#
# The current documentation gpcc/doc/doxygen/html/ and the current warnings.log will be deleted.

CONFIGURATIONS="chibios_arm linux_arm linux_arm_tfc linux_x64 linux_x64_tfc"

echo "Remove existing documentation"
echo "================================================================================"
rm -fr ../doc/doxygen/html/
rm -fr ../doc/doxygen/html_*
rm ../doc/doxygen/warnings.log
rm ../doc/doxygen/warnings_*

echo

for CONFIGURATION in ${CONFIGURATIONS}
do
  echo "Building doxygen for \"${CONFIGURATION}\""
  echo "================================================================================"
  doxygen doxyfile_${CONFIGURATION}
  mv ../doc/doxygen/html/ ../doc/doxygen/html_${CONFIGURATION}/
  mv ../doc/doxygen/warnings.log ../doc/doxygen/warnings_${CONFIGURATION}.log
  echo

  echo "Building doxygen for \"${CONFIGURATION}\" (extract all)"
  echo "================================================================================"
  doxygen doxyfile_${CONFIGURATION}_extract_all
  mv ../doc/doxygen/html/ ../doc/doxygen/html_${CONFIGURATION}_extract_all/
  mv ../doc/doxygen/warnings.log ../doc/doxygen/warnings_${CONFIGURATION}_extract_all.log
  echo
done

echo "Summary of warnings:"
echo "================================================================================"

cd ../doc/doxygen
for filename in warnings_*.log
do
  echo "${filename}:"
  echo "---------------------------------------------------"
  cat ${filename}
  echo
done

cd ../../scripts

echo DONE
