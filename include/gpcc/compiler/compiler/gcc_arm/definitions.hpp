/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
