/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#ifndef PANIC_HPP_201904071051
#define PANIC_HPP_201904071051

#include <gpcc/compiler/definitions.hpp>
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

#endif // #ifndef PANIC_HPP_201904071051
#endif // #ifdef OS_LINUX_ARM_TFC
