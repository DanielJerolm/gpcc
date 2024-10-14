/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef COMPILER_GCC_ARM

#ifndef WARNINGS_HPP_202410092046
#define WARNINGS_HPP_202410092046

/**
 * @ingroup GPCC_TESTS_COMPILER_WARNINGS
 * @{
 */

/**
 * \brief Disables compiler warnings related to move-to-self.
 *
 * This is intended to be used in unit test cases testing a UUT's move-assignment operator for self-assignment.
 * Afterwards, the previous warning level shall be restored via @ref GPCC_RESTORE_WARN_SELFMOVE().
 *
 * Example:
 * ~~~{.cpp}
 * // Test move-assignment to self.
 * GPCC_DISABLE_WARN_SELFMOVE();
 * uut = std::move(uut);
 * GPCC_RESTORE_WARN_SELFMOVE();
 * ~~~
 */
#define GPCC_DISABLE_WARN_SELFMOVE()                 \
  _Pragma("GCC diagnostic push")                     \
  _Pragma("GCC diagnostic ignored \"-Wself-move\"")  \

/**
 * \brief Restores the previous configuration for compiler warnings related to move-to-self before
 *        @ref GPCC_DISABLE_WARN_SELFMOVE() was called.
 */
#define GPCC_RESTORE_WARN_SELFMOVE()                 \
  _Pragma("GCC diagnostic pop")

/**@}*/

#endif // WARNINGS_HPP_202410092046
#endif // COMPILER_GCC_ARM
