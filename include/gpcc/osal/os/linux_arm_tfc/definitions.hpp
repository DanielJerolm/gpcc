/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#ifndef DEFINITIONS_HPP_202408132019
#define DEFINITIONS_HPP_202408132019

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_DEFS
 * \brief Preprocessor definition of a literal string containing the character sequence used to indicate a line ending
 *        on the underlying platform.
 *
 * Usage:
 * ~~~{.cpp}
 * std::string multilineText("Line1" GPCC_ENDL \\
 *                           "Line2" GPCC_ENDL \\
 *                           "Line3" GPCC_ENDL);
 * ~~~
 *
 * \see gpcc::osal::endl
 */
#define GPCC_ENDL   "\n"

/**
 * \ingroup GPCC_OSAL_DEFS
 * \brief Null-terminated string containing the character or character sequence used to indicate a line ending on the
 *        underlying platform.
 */
constexpr char const endl[] = GPCC_ENDL;

} // namespace osal
} // namespace gpcc

#endif // DEFINITIONS_HPP_202408132019
#endif // OS_LINUX_ARM_TFC
