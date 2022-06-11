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
#ifndef STRINGLOGMESSAGE_HPP_201701061517
#define STRINGLOGMESSAGE_HPP_201701061517

#include "LogMessage.hpp"

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message comprised of an std::string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringLogMessage final : public LogMessage
{
  public:
    StringLogMessage(void) = delete;
    StringLogMessage(string::SharedString const & _srcName,
                     LogType const _type,
                     std::string const & _msg);
    StringLogMessage(string::SharedString const & _srcName,
                     LogType const _type,
                     std::string && _msg);
    StringLogMessage(StringLogMessage const &) = delete;
    StringLogMessage(StringLogMessage &&) = delete;
    ~StringLogMessage(void) override = default;

    StringLogMessage& operator=(StringLogMessage const &) = delete;
    StringLogMessage& operator=(StringLogMessage &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Log message text.
    std::string const msg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // STRINGLOGMESSAGE_HPP_201701061517
