/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2022 Daniel Jerolm

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

#ifdef OS_LINUX_X64_TFC

#ifndef PANIC_HPP_201702252115
#define PANIC_HPP_201702252115

#include "gpcc/src/Compiler/definitions.hpp"
#include <stdexcept>

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Panic handler function pointer type definition.
 *
 * The referenced function shall never return.
 *
 * - - -
 *
 * Parameters:\n
 * 1st:\n
 * Pointer to a null-terminated c-string containing a panic message provided by the caller.\n
 * This may be `nullptr`, if no panic message is provided by the caller. In this case the handler function shall use
 * an appropriate default message.
 *
 * - - -
 *
 * __Thread-safety requirements/hints:__\n
 * The referenced function/method shall be thread-safe.
 *
 * __Exception-safety requirements/hints:__\n
 * The referenced function/method shall provide the no-throw guarantee.
 *
 * __Thread-cancellation-safety requirements/hints:__\n
 * The referenced function/method shall not contain any cancellation point.
 */
typedef void(*tPanicHandler)(char const * const pMessage);

NORETURN1 void Panic(void) noexcept NORETURN2;
NORETURN1 void Panic(char const * const pMessage) noexcept NORETURN2;
NORETURN1 void Panic(char const * const pMessage, std::exception const & e) noexcept NORETURN2;
NORETURN1 void Panic(char const * const pFileName, int const line) noexcept NORETURN2;
NORETURN1 void Panic(char const * const pFileName, int const line, std::exception const & e) noexcept NORETURN2;

tPanicHandler GetPanicHandler(void) noexcept;
void SetPanicHandler(tPanicHandler const newPanicHandler) noexcept;

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Macro for invocation of [Panic(char const * const, int)](@ref gpcc::osal::Panic(char const * const, int)).
 *
 * Intended use:
 * ~~~{.cpp}
 * if ((brokenInvariant) || (unrecoverableError))
 *   PANIC();
 *
 * try
 * {
 *   ...
 * }
 * catch (...)
 * {
 *   PANIC();
 * }
 * ~~~
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
#define PANIC() \
do { \
  gpcc::osal::Panic(__FILE__, __LINE__); \
} while(false)

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Macro for invocation of
 *        [Panic(char const * const, int, std::exception const &)](@ref gpcc::osal::Panic(char const * const, int, std::exception const &)).
 *
 * Intended use:
 * ~~~{.cpp}
 * try
 * {
 *   ...
 * }
 * catch (std::exception const & e)
 * {
 *   PANIC_E(e);
 * }
 * catch (...)
 * {
 *   PANIC();
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param ex
 * Reference to an exception, whose message text (retrieved from `e.what()`) shall be appended to the filename and
 * line number.
 */
#define PANIC_E(ex) \
do { \
  gpcc::osal::Panic(__FILE__, __LINE__, (ex)); \
} while(false)

} // namespace osal
} // namespace gpcc

#endif // #ifndef PANIC_HPP_201702252115
#endif // #ifdef OS_LINUX_X64_TFC
