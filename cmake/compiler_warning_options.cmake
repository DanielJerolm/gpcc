# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm

# ------------------------------------------------------------------------
# Warning levels and settings applied to GPCC productive and unittest code
# ------------------------------------------------------------------------
list(APPEND GPCC_CXX_WARN_OPTIONS "-Wall"
                                  "-Wextra"
                                  "-Wdouble-promotion"
                                  "-Wformat=2"
                                  "-Wundef")

# Currently not applied, but planned:
# -Wshadow
#    Produces approx. 26 warnings, but appliance makes sense
#
# -Wpedantic
#   4 findings, easy to fix. Complains about using '#warning'

# Not applied by intention:
# -Wconversion
#    Produces ~376 findings. All reviewed findings are intentional. Appliance is not planned.
