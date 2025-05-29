# General Purpose Class Collection (GPCC)
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2025 Daniel Jerolm


# This is a toolchain file for the native GCC. If passed to CMake during initial configuration,
# then all libs and executables will be build with GCC's Undefined Behaviour Sanitizer (UBSan)
# enabled. Execution will stop on first encountered error.

add_compile_options("-fsanitize=undefined"
                    "-fno-sanitize-recover=all")
add_link_options("-fsanitize=undefined"
                 "-fno-sanitize-recover=all")
