/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef BUILTINS_HPP_202206112147
#define BUILTINS_HPP_202206112147

/**
 * @ingroup GPCC_COMPILER
 * @defgroup GPCC_COMPILER_BUILTINS Compiler-builtins and special CPU instructions
 * \brief Useful functions making use of compiler-builtins or special CPU instructions.
 */

#ifdef COMPILER_GCC_X64
#include "compiler/gcc_x64/builtins.hpp"
#endif

#ifdef COMPILER_GCC_ARM
#include "compiler/gcc_arm/builtins.hpp"
#endif

#endif /* BUILTINS_HPP_202206112147 */
