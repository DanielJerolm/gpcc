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

#ifdef COMPILER_GCC_ARM

#ifndef DEFINITIONS_HPP_202206112146
#define DEFINITIONS_HPP_202206112146

/**
 * @ingroup GPCC_COMPILER_SYSTEM_ENDIAN
 * @{
 */

#define GPCC_SYSTEMS_ENDIAN GPCC_LITTLE ///<System's endian

/**@}*/

/**
 * @ingroup GPCC_COMPILER_ATTRIBUTES
 * @{
 */

/**
 * \brief Keyword (1 of 2) for packet structs.
 *
 * Usage example:
 * ~~~{.cpp}
 * typedef PACKED1 struct x
 * {
 *   ..
 * } PACKED2 x;
 *
 * typedef PACKED1 struct
 * {
 *   ..
 * } PACKED2 y;
 *
 * PACKED1 struct
 * {
 *   ..
 * } PACKED2 z;
 * ~~~
 */
#define PACKED1

/**
 * \brief Keyword (2 of 2) for packet structs.
 *
 * Usage example:
 * ~~~{.cpp}
 * typedef PACKED1 struct x
 * {
 *   ..
 * } PACKED2 x;
 *
 * typedef PACKED1 struct
 * {
 *   ..
 * } PACKED2 y;
 *
 * PACKED1 struct
 * {
 *   ..
 * } PACKED2 z;
 * ~~~
 */
#define PACKED2 __attribute__((__packed__))

/**
 * \brief Keyword (1 of 2) for declaring no-return functions.
 *
 * Usage example:
 * ~~~{.cpp}
 * // declaration:
 * NORETURN1 void SomeFunc(void) NORETURN2;
 * NORETURN1 void SomeOtherFunc(void) noexcept NORETURN2;
 *
 * // definition:
 * void SomeFunc(void)
 * {
 *   ...
 * }
 * void SomeOtherFunc(void) noexcept
 * {
 *   ...
 * }
 * ~~~
 */
#define NORETURN1

/**
 * \brief Keyword (2 of 2) for declaring no-return functions.
 *
 * Usage example:
 * ~~~{.cpp}
 * // declaration:
 * NORETURN1 void SomeFunc(void) NORETURN2;
 * NORETURN1 void SomeOtherFunc(void) noexcept NORETURN2;
 *
 * // definition:
 * void SomeFunc(void)
 * {
 *   ...
 * }
 * void SomeOtherFunc(void) noexcept
 * {
 *   ...
 * }
 * ~~~
 */
#define NORETURN2 __attribute__((__noreturn__))

/**@}*/

#endif /* DEFINITIONS_HPP_202206112146 */
#endif /* #ifdef COMPILER_GCC_ARM */
