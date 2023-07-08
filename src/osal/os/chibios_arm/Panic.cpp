/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_CHIBIOS_ARM

#include <gpcc/osal/Panic.hpp>
#include <ch.h>
#include <atomic>
#include <string>
#include <cstring>

namespace gpcc {
namespace osal {

static NORETURN1 void DefaultPanicHandler(char const * const pMessage) noexcept NORETURN2;

/*
 * \ingroup GPCC_OSAL_PANIC
 * \brief Default panic handler.
 *
 * This prints the panic message given by `pMessage` to `chSysHalt()`.
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
  if (pMessage != nullptr)
    chSysHalt(pMessage);
  else
    chSysHalt("No panic message provided");

  while (true) {};
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
  Panic(nullptr);
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
 * This is intended to query and store the current panic handler before setting up a custom one via
 * @ref SetPanicHandler(). At a later point in time, the previous panic handler can be recovered.
 *
 * A typical scenario that requires recovery of the original panic handler is the temporary installation of a custom
 * panic handler during a unittest case.
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
 * This can be used to setup a custom panic handler if the default one does not meet the requirements.
 *
 * If necessary, @ref GetPanicHandler() can be used to retrieve the currently configured panic handler function for
 * later recovery. A typical scenario that requires recovery of the original panic handler is the temporary installation
 * of a custom panic handler during a unittest case.
 *
 * \note  [TFC](@ref GPCC_TIME_FLOW_CONTROL) specific behaviour:\n
 *        If the panic originates from the TFC core (e.g. "Dead-Lock detected. All threads permanently blocked."), then
 *        the default panic handler will be used, even if a custom panic handler has been setup via this function.
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

#endif // #ifdef OS_CHIBIOS_ARM
