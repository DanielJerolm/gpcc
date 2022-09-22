/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef DEFINITIONS_HPP_202206112147
#define DEFINITIONS_HPP_202206112147

/**
 * @ingroup GPCC_COMPILER
 * @defgroup GPCC_COMPILER_SYSTEM_ENDIAN System's endian
 * \brief Definitions indicating the system's endian at compile-time.
 * @{
 */
#define GPCC_BIG     1 ///<Value for GPCC_SYSTEMS_ENDIAN: Big endian
#define GPCC_LITTLE  2 ///<Value for GPCC_SYSTEMS_ENDIAN: Little endian
/** @} */

/**
 * @ingroup GPCC_COMPILER
 * @defgroup GPCC_COMPILER_ATTRIBUTES Attributes
 * \brief Attributes for alignment of variables, packing structs etc.
 */

#ifdef COMPILER_GCC_X64
#include "compiler/gcc_x64/definitions.hpp"
#endif

#ifdef COMPILER_GCC_ARM
#include "compiler/gcc_arm/definitions.hpp"
#endif

#endif /* DEFINITIONS_HPP_202206112147 */
