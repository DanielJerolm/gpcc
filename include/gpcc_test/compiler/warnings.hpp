/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifndef WARNINGS_HPP_202410092043
#define WARNINGS_HPP_202410092043

#ifdef COMPILER_GCC_ARM
#include "compiler/gcc_arm/warnings.hpp"
#endif

#ifdef COMPILER_GCC_X64
#include "compiler/gcc_x64/warnings.hpp"
#endif

#endif /* WARNINGS_HPP_202410092043 */
