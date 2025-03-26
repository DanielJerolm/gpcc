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
                                  "-Wextra")

# Currently not applied, but planned:
# -Wformat=2
# -Wundef
# -Wshadow
#    Produces approx. 26 warnings, but appliance makes sense
#
# -Wdouble-promotion
#    Produces a few warnings. Appliance makes sense.
#    snprintf can be fixed via (std::is_same<T, float>::value == false)
#
# -Wpedantic
#   4 findings, easy to fix. Complains about using '#warning'

# Not applied by intention:
# -Wconversion
#    Produces ~376 findings. All reviewed findings are intentional. Appliance is not planned.
