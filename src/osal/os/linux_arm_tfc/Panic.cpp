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

#ifdef OS_LINUX_ARM_TFC

#include "Panic.hpp"
#include <atomic>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

namespace gpcc {
namespace osal {

static NORETURN1 void DefaultPanicHandler(char const * const pMessage) noexcept NORETURN2;

/*
 * \ingroup GPCC_OSAL_PANIC
 * \brief Default panic handler.
 *
 * This prints the panic message given by `pMessage` to stderr and aborts the current process.
 *
 * _This function never returns._
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
 * \param pMessage
 * Pointer to a null-terminated c-string containing a panic message.\n
 * nullptr is allowed.
 */
static void DefaultPanicHandler(char const * const pMessage) noexcept
{
  try
  {
    if (pMessage != nullptr)
      std::cerr << "PANIC: " << pMessage;
    else
      std::cerr << "PANIC: No message";
  }
  catch (...)
  {
    // ignored by intention, abort() will be invoked anyway
  }

  abort();
}

// Pointer to the currently configured panic handler function.
static std::atomic<tPanicHandler> panicHandler(&DefaultPanicHandler);

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Panic function. Aborts program execution.
 *
 * _This never returns._
 *
 * Note:\n
 * There are multiple overloads of this function available.\n
 * This one has minimal requirements and will likely work properly even if the program is seriously broken.
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
 */
void Panic(void) noexcept
{
  panicHandler(nullptr);
  abort();
}

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Panic function. Aborts program execution and takes a message as argument.
 *
 * _This never returns._
 *
 * Note:\n
 * There are multiple overloads of this function available.\n
 * This one has minimal requirements and will likely work properly even if the program is seriously broken.
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
 * \param pMessage
 * Pointer to a null-terminated c-string containing a panic message.\n
 * If this is nullptr, then the behavior is the same as if [Panic()](@ref Panic(void)) had been called.
 */
void Panic(char const * const pMessage) noexcept
{
  panicHandler(pMessage);
  abort();
}

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Panic function. Aborts program execution and takes a message and an exception as arguments.
 *
 * _This never returns._
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   ...
 * }
 * catch (std::exception const & e)
 * {
 *   Panic("MyClass::MyFunc: Caught unexpected exception: ", e);
 * }
 * ~~~
 *
 * Note:\n
 * There are multiple overloads of this function available.\n
 * This overload of `Panic()` uses dynamic memory to build the panic message.\n
 * Therefore it may not work properly, if the program is seriously broken.\n
 * In such cases it is recommended to use a different overload of this function, which has less strict requirements:
 * - @ref Panic(void)
 * - @ref Panic(char const * const)
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
 * \param pMessage
 * Pointer to a null-terminated c-string containing a panic message. It will be prepended to the description of the
 * exception referenced by `e`. Therefore it should end with a colon and a space character.\n
 * If this is nullptr, then the behavior is the same as if [Panic(e.what())](@ref Panic(char const * const)) had been
 * called.
 *
 * \param e
 * Reference to an exception, whose message text (retrieved from `e.what()`) shall be appended to `pMessage`.
 */
void Panic(char const * const pMessage, std::exception const & e) noexcept
{
  try
  {
    char const * const pExDescr = e.what();

    if (pMessage == nullptr)
      Panic(pExDescr);

    std::string s;
    s.reserve(strlen(pMessage) + strlen(pExDescr));

    s = pMessage;
    s += pExDescr;

    Panic(s.c_str());
  }
  catch (...)
  {
    Panic("Panic() failed");
  }
}

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Panic function. Aborts program execution and takes the filename and line number as arguments.
 *
 * _This never returns._
 *
 * For ease of use one may use the @ref PANIC() macro to invoke this.
 *
 * Note:\n
 * There are multiple overloads of this function available.\n
 * This overload of `Panic()` uses dynamic memory to build the panic message.\n
 * Therefore it may not work properly, if the program is seriously broken.\n
 * In such cases it is recommended to use a different overload of this function, which has less strict requirements:
 * - @ref Panic(void)
 * - @ref Panic(char const * const)
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
 * \param pFileName
 * Pointer to a null-terminated c-string containing the filename.\n
 * The caller shall use the preprocessor macro \_\_FILE\_\_ to gather this parameter.\n
 * If this is nullptr, then the behavior is the same as if [Panic()](@ref Panic(void)) had been called.
 *
 * \param line
 * Number of line. The caller shall use the preprocessor macro \_\_LINE\_\_ to gather this parameter.
 */
void Panic(char const * const pFileName, int const line) noexcept
{
  try
  {
    if (pFileName == nullptr)
      Panic();

    std::string s;
    s.reserve(strlen(pFileName) + 3U + 10U);

    s = pFileName;
    s += " (";
    s += std::to_string(line);
    s += ')';

    Panic(s.c_str());
  }
  catch (...)
  {
    Panic("Panic() failed");
  }
}

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Panic function. Aborts program execution and takes the filename, the line number, and an exception as
 *        arguments.
 *
 * _This never returns._
 *
 * For ease of use one may use the @ref PANIC_E() macro to invoke this.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   ...
 * }
 * catch (std::exception const & e)
 * {
 *   Panic(__FILE__, __LINE__, e);
 * }
 * ~~~
 *
 * Note:\n
 * There are multiple overloads of this function available.\n
 * This overload of `Panic()` uses dynamic memory to build the panic message.\n
 * Therefore it may not work properly, if the program is seriously broken.\n
 * In such cases it is recommended to use a different overload of this function, which has less strict requirements:
 * - @ref Panic(void)
 * - @ref Panic(char const * const)
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
 * \param pFileName
 * Pointer to a null-terminated c-string containing the filename.\n
 * The caller shall use the preprocessor macro \_\_FILE\_\_ to gather this parameter.\n
 * If this is nullptr, then the behavior is the same as if [Panic()](@ref Panic(void)) had been called.
 *
 * \param line
 * Number of line. The caller shall use the preprocessor macro \_\_LINE\_\_ to gather this parameter.
 *
 * \param e
 * Reference to an exception, whose message text (retrieved from `e.what()`) shall be appended to the filename and
 * line number.
 */
void Panic(char const * const pFileName, int const line, std::exception const & e) noexcept
{
  try
  {
    if (pFileName == nullptr)
      Panic();

    char const * const pExDescr = e.what();

    std::string s;
    s.reserve(strlen(pFileName) + 5U + 10U + strlen(pExDescr));

    s = pFileName;
    s += " (";
    s += std::to_string(line);
    s += "): ";
    s += pExDescr;

    Panic(s.c_str());
  }
  catch (...)
  {
    Panic("Panic() failed");
  }
}

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Retrieves the currently configured panic handler function.
 *
 * This can be used to retrieve the current panic handler before changing it via @ref SetPanicHandler(). \n
 * If the current panic handler is stored before changing it, then it can be recovered later.\n
 * Typical scenarios that require recovery of the original panic handler are e.g. GPCC's own unit tests.
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
 * \return
 * Currently configured panic hander function. See @ref tPanicHandler for details.
 */
tPanicHandler GetPanicHandler(void) noexcept
{
  return panicHandler;
}

/**
 * \ingroup GPCC_OSAL_PANIC
 * \brief Sets the panic handler function.
 *
 * This can be used to setup your own panic handler if the default one does not meet your requirements.
 *
 * If necessary, use @ref GetPanicHandler() to retrieve the currently configured panic handler
 * function for later recovery. Typical scenarios that require recovery of the original panic handler
 * are e.g. GPCC's own unit tests.
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
 * \param newPanicHandler
 * New panic handler function. See @ref tPanicHandler for details.\n
 * nullptr is not allowed.
 */
void SetPanicHandler(tPanicHandler const newPanicHandler) noexcept
{
  if (newPanicHandler == nullptr)
    PANIC();

  panicHandler = newPanicHandler;
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
