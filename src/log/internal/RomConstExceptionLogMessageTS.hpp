/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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
#ifndef ROMCONSTEXCEPTIONLOGMESSAGETS_HPP_202111211045
#define ROMCONSTEXCEPTIONLOGMESSAGETS_HPP_202111211045

#include "LogMessage.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include <exception>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located in ROM/code memory and an
 *        exception conserved in an std::exception_ptr plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class RomConstExceptionLogMessageTS final : public LogMessage
{
  public:
    RomConstExceptionLogMessageTS(void) = delete;
    RomConstExceptionLogMessageTS(string::SharedString const & _srcName,
                                  LogType const _type,
                                  char const * const _pMsg,
                                  std::exception_ptr const & _ePtr);
    RomConstExceptionLogMessageTS(RomConstExceptionLogMessageTS const &) = delete;
    RomConstExceptionLogMessageTS(RomConstExceptionLogMessageTS &&) = delete;
    ~RomConstExceptionLogMessageTS(void) override = default;

    RomConstExceptionLogMessageTS& operator=(RomConstExceptionLogMessageTS const &) = delete;
    RomConstExceptionLogMessageTS& operator=(RomConstExceptionLogMessageTS &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Log message text.
    /** This points to a null-terminated c-string, which is located in ROM/code memory. */
    char const * const pMsg;

    /// Exception to be build into the log message.
    std::exception_ptr const ePtr;

    /// Timestamp.
    gpcc::time::TimePoint const timestamp;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // ROMCONSTEXCEPTIONLOGMESSAGETS_HPP_202111211045
