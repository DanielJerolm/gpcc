/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifndef BUILTINS_HPP_202206112147
#define BUILTINS_HPP_202206112147

#ifdef COMPILER_GCC_X64
#include "compiler/gcc_x64/builtins.hpp"
#endif

#ifdef COMPILER_GCC_ARM
#include "compiler/gcc_arm/builtins.hpp"
#endif

#endif /* BUILTINS_HPP_202206112147 */
