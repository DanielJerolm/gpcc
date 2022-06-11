/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2021 Daniel Jerolm

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
#ifndef STRINGEXCEPTIONLOGMESSAGE_HPP_201801262128
#define STRINGEXCEPTIONLOGMESSAGE_HPP_201801262128

#include "LogMessage.hpp"
#include <exception>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of an std::string and an exception conserved in a std::exception_ptr.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringExceptionLogMessage final : public LogMessage
{
  public:
    StringExceptionLogMessage(void) = delete;
    StringExceptionLogMessage(string::SharedString const & _srcName,
                              LogType const _type,
                              std::string const & _msg,
                              std::exception_ptr const & _ePtr);
    StringExceptionLogMessage(string::SharedString const & _srcName,
                              LogType const _type,
                              std::string && _msg,
                              std::exception_ptr const & _ePtr);
    StringExceptionLogMessage(StringExceptionLogMessage const &) = delete;
    StringExceptionLogMessage(StringExceptionLogMessage &&) = delete;
    ~StringExceptionLogMessage(void) override = default;

    StringExceptionLogMessage& operator=(StringExceptionLogMessage const &) = delete;
    StringExceptionLogMessage& operator=(StringExceptionLogMessage &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Exception to be build into the log message.
    std::exception_ptr const ePtr;

    /// Log message text.
    std::string const msg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // STRINGEXCEPTIONLOGMESSAGE_HPP_201801262128
