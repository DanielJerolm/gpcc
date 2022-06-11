!/bin/bash

# General Purpose Class Collection (GPCC)
# Copyright (C) 2019, 2022 Daniel Jerolm
#
# This file is part of the General Purpose Class Collection (GPCC).
#
# The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#                                  ---
#
# A special exception to the GPL can be applied should you wish to distribute
# a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
# to provide the source code for any proprietary components. See the file
# license_exception.txt for full details of how and when the exception can be applied.
#


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
