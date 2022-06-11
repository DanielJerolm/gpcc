/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019 Daniel Jerolm

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

#ifndef ILOGFACILITYCTRL_HPP_201701061537
#define ILOGFACILITYCTRL_HPP_201701061537

#include "gpcc/src/log/log_levels.hpp"
#include <string>
#include <tuple>
#include <vector>

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG_LOGFACILITIES
 * \brief Common interface for controlling log facilities.
 *
 * This interface allows to...
 * - retrieve a list with the name of each log source (@ref Logger instance) registered at the log facility
 *   plus the log level currently configured at the log source.
 * - set the log level of a specific log source.
 * - ensure a minimum log level for a specific log source.
 * - ensure a maximum log level for a specific log source.
 * - setup default settings for new @ref Logger instances registered at the log facility.
 * - remove previously setup default settings for new @ref Logger instances.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ILogFacilityCtrl
{
  public:
    /// Type definition for an std::pair containing a log source name and the associated log level.
    typedef std::pair<std::string, LogLevel> tLogSrcConfig;


    virtual std::vector<tLogSrcConfig> EnumerateLogSources(void) const = 0;
    virtual LogLevel GetLogLevel(std::string const & srcName) const = 0;
    virtual bool SetLogLevel(std::string const & srcName, LogLevel const level) = 0;
    virtual bool LowerLogLevel(std::string const & srcName, LogLevel const level) = 0;
    virtual bool RaiseLogLevel(std::string const & srcName, LogLevel const level) = 0;

    virtual void SetDefaultSettings(std::vector<tLogSrcConfig> _defaultSettings) = 0;
    virtual std::vector<tLogSrcConfig> RemoveDefaultSettings(void) = 0;

  protected:
    virtual ~ILogFacilityCtrl(void) = default;
};

/**
 * \fn ILogFacilityCtrl::EnumerateLogSources
 * \brief Retrieves a list with the names and log levels of all log sources currently registered at the log facility.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Vector containing the names and log levels of all log sources ([Logger](@ref gpcc::log::Logger) instances)
 * currently registered at the log facility.
 */

/**
 * \fn ILogFacilityCtrl::GetLogLevel
 * \brief Queries the log level of a log source.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   Log source with given name not found.
 *
 * \throws std::bad_alloc          Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param srcName
 * Name of the log source whose current log level shall be queried.
 *
 * \return
 * Current log level of the log source referenced by `srcName`.
 */

/**
 * \fn ILogFacilityCtrl::SetLogLevel
 * \brief Sets the log level of a specific log source.
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
 * \param srcName
 * Name of the log source whose log level shall be set.
 *
 * \param level
 * New log level.
 *
 * \return
 * Result of the operation:\n
 * true  = log source `srcName` found\n
 * false = log source `srcName` not found
 */

/**
 * \fn ILogFacilityCtrl::LowerLogLevel
 * \brief Ensures that the log level of a specific log source is at or below a given level.
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
 * \param srcName
 * Name of the log source whose log level shall be reduced.
 *
 * \param level
 * New log level.\n
 * If the current log level is already lower or equal, then the log level is not altered.
 *
 * \return
 * Result of the operation:\n
 * true  = log source `srcName` found\n
 * false = log source `srcName` not found
 */

/**
 * \fn ILogFacilityCtrl::RaiseLogLevel
 * \brief Ensures that the log level of a specific log source is at or above a given level.
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
 * \param srcName
 * Name of the log source whose log level shall be raised.
 *
 * \param level
 * New log level.\n
 * If the current log level is already higher or equal, then the log level is not altered.
 *
 * \return
 * Result of the operation:\n
 * true  = log source `srcName` found\n
 * false = log source `srcName` not found
 */

/**
 * \fn ILogFacilityCtrl::SetDefaultSettings
 * \brief Provides a list of [tLogSrcConfig](@ref gpcc::log::ILogFacilityCtrl::tLogSrcConfig) entries to the
 *        log facility. The list is used to setup the initial log levels of subsequent registered
 *        [Logger](@ref gpcc::log::Logger) instances.
 *
 * If a [Logger](@ref gpcc::log::Logger) instance is registered at the log facility after calling this, then the log
 * facility will scan the list of default log levels for an entry which matches the log source name of the
 * [Logger](@ref gpcc::log::Logger) instance.
 *
 * In case of a _match_, the [Logger](@ref gpcc::log::Logger) instance's log level will be set to the log level
 * specified by the matching entry. The matching entry will be removed from the list.
 *
 * In case of _no match_, the new [Logger](@ref gpcc::log::Logger) instance's log level will not be modified by the
 * log facility. A warning log message will be generated, that indicates that there was no default setting for the
 * [Logger](@ref gpcc::log::Logger) instance deposited. The warning log messages will be logged using the new
 * registered [Logger](@ref gpcc::log::Logger) instance.
 *
 * If there is already a list of default log levels setup at the log facility, then the existing list will be
 * dropped and replaced by the new list setup via this method.
 *
 * After all [Logger](@ref gpcc::log::Logger) instances have been registered (or at any other point in time),
 * [RemoveDefaultSettings()](@ref gpcc::log::ILogFacilityCtrl::RemoveDefaultSettings) can be invoked to remove the
 * list from the log facility. The removed list is returned by
 * [RemoveDefaultSettings()](@ref gpcc::log::ILogFacilityCtrl::RemoveDefaultSettings) and could be checked for
 * entries that have not been consumed (remember that an entry is consumed in case of a match).
 *
 * The list of default log levels setup via this method will remain at the log facility until it is either replaced by
 * another list setup via another call to this method, or until the list is removed via
 * [RemoveDefaultSettings()](@ref gpcc::log::ILogFacilityCtrl::RemoveDefaultSettings).
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
 * \param _defaultSettings
 * List containing default log levels for subsequent registered [Logger](@ref gpcc::log::Logger) instances.\n
 * To avoid a copy of the list, consider using `std::move()` when passing the list as parameter.
 */

/**
 * \fn ILogFacilityCtrl::RemoveDefaultSettings
 * \brief Removes the list of default log levels for new registered [Logger](@ref gpcc::log::Logger) instances
 *        from the log facility.
 *
 * This method is the counterpart to [SetDefaultSettings()](@ref gpcc::log::ILogFacilityCtrl::SetDefaultSettings).
 *
 * Any [Logger](@ref gpcc::log::Logger) instance which is registered at the log facility after the list with the
 * default settings has been removed from the log facility will keep its original log level when it is registered
 * at the log facility. No warning message indicating a missing default log level will be created when a new logger
 * is registered after removal of the list of default settings.
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
 * \return
 * List of default log levels.\n
 * The list will be empty, if:
 * - ...all default settings have been consumed,
 * - ...or if no list has been setup via [SetDefaultSettings()](@ref gpcc::log::ILogFacilityCtrl::SetDefaultSettings) before,
 * - ...or if the list has already been removed via a previous call to this method.
 */

} // namespace log
} // namespace gpcc

#endif // ILOGFACILITYCTRL_HPP_201701061537
