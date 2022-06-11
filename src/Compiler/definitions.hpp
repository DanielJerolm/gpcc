/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
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
