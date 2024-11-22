/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifndef DEFINITIONS_HPP_202206112147
#define DEFINITIONS_HPP_202206112147

/**
 * @ingroup GPCC_COMPILER_SYSTEM_ENDIAN
 * @{
 */
#define GPCC_BIG     1 ///<Value for GPCC_SYSTEMS_ENDIAN: Big endian
#define GPCC_LITTLE  2 ///<Value for GPCC_SYSTEMS_ENDIAN: Little endian
/** @} */

#ifdef COMPILER_GCC_X64
#include "compiler/gcc_x64/definitions.hpp"
#endif

#ifdef COMPILER_GCC_ARM
#include "compiler/gcc_arm/definitions.hpp"
#endif

#endif /* DEFINITIONS_HPP_202206112147 */
