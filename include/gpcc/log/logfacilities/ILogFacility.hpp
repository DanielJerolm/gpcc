/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ILOGFACILITY_HPP_201701061534
#define ILOGFACILITY_HPP_201701061534

#include <memory>

namespace gpcc {
namespace log  {

class Backend;
class Logger;

namespace internal
{
  class LogMessage;
}

/**
 * \ingroup GPCC_LOG_LOGFACILITIES
 * \brief Common interface for log facilities.
 *
 * This interface allows to...
 * - register and unregister [Logger](@ref gpcc::log::Logger) instances
 * - register and unregister [Backend](@ref gpcc::log::Backend) instances
 * - pass [LogMessage](@ref gpcc::log::internal::LogMessage) objects (from an [Logger](@ref gpcc::log::Logger) instance)
 *   to the log facility for logging
 * - report errors that ocurred during log message creation (e.g. std::bad_alloc) to the log facility for logging
 *
 * Note that one and the same [Logger](@ref gpcc::log::Logger) instance can only be registered at one log facility.\n
 * The other way round, multiple different [Logger](@ref gpcc::log::Logger) instances can be registered at one and
 * the same log facility.
 *
 * The same applies to back-ends. One and the same [Backend](@ref gpcc::log::Backend) instance can only be registered
 * at one log facility.\n
 * The other way round, multiple different [Backend](@ref gpcc::log::Backend) instances can be registered at one and
 * the same log facility.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ILogFacility
{
  public:
    virtual void Register(Logger& logger) = 0;
    virtual void Unregister(Logger& logger) = 0;

    virtual void Register(Backend& backend) = 0;
    virtual void Unregister(Backend& backend) = 0;

    virtual void Log(std::unique_ptr<internal::LogMessage> spMsg) = 0;
    virtual void ReportLogMessageCreationFailed(void) noexcept = 0;

  protected:
    virtual ~ILogFacility(void) = default;
};

/**
 * \fn void ILogFacility::Register(gpcc::log::Logger& logger)
 * \brief Registers a [Logger](@ref gpcc::log::Logger) instance at the log facility.
 *
 * If default settings are deposited at the log facility, then the logger's log level will be setup according to
 * these default settings, if an entry matching the logger's name is found in the default settings.\n
 * At the same time, the deposited default settings for the particular logger will be consumed. Other default settings
 * for other loggers deposited at the log facility will be ignored and will not be affected.\n
 * See [ILogFacilityCtrl::SetDefaultSettings()](@ref gpcc::log::ILogFacilityCtrl::SetDefaultSettings) for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param logger
 * Reference to the [Logger](@ref gpcc::log::Logger) instance that shall be registered at the log facility.\n
 * One and the same [Logger](@ref gpcc::log::Logger) instance can be registered at one log facility only.\n
 * Multiple different [Logger](@ref gpcc::log::Logger) instances can be registered at one and the same log facility.\n
 * Registration will fail, if there is already a [Logger](@ref gpcc::log::Logger) instance with the same log source
 * name registered at the log facility.
 */

/**
 * \fn void ILogFacility::Unregister(gpcc::log::Logger& logger)
 * \brief Unregisters a [Logger](@ref gpcc::log::Logger) instance from the log facility.
 *
 * \pre   The [Logger](@ref gpcc::log::Logger) instance is registered at the log facility.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param logger
 * Reference to the [Logger](@ref gpcc::log::Logger) instance that shall be unregistered from the log facility.
 */

/**
 * \fn void ILogFacility::Register(gpcc::log::Backend& backend)
 * \brief Registers a back-end at the log facility.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param backend
 * Reference to the back-end (instance of a sub-class of class [Backend](@ref gpcc::log::Backend)), that shall be
 * registered at the log facility.\n
 * One and the same [Backend](@ref gpcc::log::Backend) instance can be registered at one log facility only.\n
 * Multiple different [Backend](@ref gpcc::log::Backend) instances can be registered at one and the same log facility.
 */

/**
 * \fn void ILogFacility::Unregister(gpcc::log::Backend& backend)
 * \brief Unregisters a back-end from the log facility.
 *
 * \pre   The [Backend](@ref gpcc::log::Backend) instance is registered at the log facility.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param backend
 * Reference to the back-end (instance of a sub-class of class [Backend](@ref gpcc::log::Backend)), that shall be
 * unregistered from the log facility.
 */

/**
 * \fn void ILogFacility::Log(std::unique_ptr<gpcc::log::internal::LogMessage> spMsg)
 * \brief Passes a log message to the log facility for logging.
 *
 * This is intended to be invoked by [Logger](@ref gpcc::log::Logger) instances only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * This will not throw `std::bad_alloc`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param spMsg
 * Pointer to the log message (instance of a sub-class of class [LogMessage](@ref gpcc::log::internal::LogMessage)),
 * that shall be passed to the log facility for logging.\n
 * Note that ownership moves to the log facility. The log message will be released even in case of an exception.\n
 * nullptr is not allowed.
 */

/**
 * \fn void ILogFacility::ReportLogMessageCreationFailed(void)
 * \brief Reports that a [Logger](@ref gpcc::log::Logger) instance or the user of a [Logger](@ref gpcc::log::Logger)
 *        instance tried to log a message, but failed due to an error (e.g. out-of-memory condition).
 *
 * This is intended to be invoked by [Logger](@ref gpcc::log::Logger) instances only.
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

} // namespace log
} // namespace gpcc

#endif // ILOGFACILITY_HPP_201701061534
