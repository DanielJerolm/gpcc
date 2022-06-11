/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2021, 2022 Daniel Jerolm

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
#ifndef ROMCONSTLOGMESSAGE_HPP_201701061513
#define ROMCONSTLOGMESSAGE_HPP_201701061513

#include "LogMessage.hpp"

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located in ROM/code memory.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class RomConstLogMessage : public LogMessage
{
  public:
    RomConstLogMessage(void) = delete;
    RomConstLogMessage(string::SharedString const & _srcName,
                       LogType const _type,
                       char const * const _pMsg);
    RomConstLogMessage(RomConstLogMessage const &) = delete;
    RomConstLogMessage(RomConstLogMessage &&) = delete;
    virtual ~RomConstLogMessage(void) override = default;

    RomConstLogMessage& operator=(RomConstLogMessage const &) = delete;
    RomConstLogMessage& operator=(RomConstLogMessage &&) = delete;


    std::string BuildText(void) const override;

  protected:
    /// Log message text.
    /** This points to a null-terminated c-string.\n
        For objects of type @ref RomConstLogMessage, this points into ROM/code memory.\n
        For objects of type @ref CStringLogMessage, this points into the heap and the referenced memory will be released
        via `free()`. */
    char const * const pMsg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // ROMCONSTLOGMESSAGE_HPP_201701061513
