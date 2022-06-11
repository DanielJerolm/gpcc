/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2022 Daniel Jerolm

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

#ifndef CSTRINGLOGMESSAGE_HPP_202201112136
#define CSTRINGLOGMESSAGE_HPP_202201112136

#include "RomConstLogMessage.hpp"
#include <memory>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located on the heap.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class CStringLogMessage final : public RomConstLogMessage
{
  public:
    CStringLogMessage(void) = delete;
    CStringLogMessage(string::SharedString const & _srcName,
                      LogType const _type,
                      std::unique_ptr<char[]> && _pMsg);
    CStringLogMessage(CStringLogMessage const &) = delete;
    CStringLogMessage(CStringLogMessage &&) = delete;
    ~CStringLogMessage(void) override;

    CStringLogMessage& operator=(CStringLogMessage const &) = delete;
    CStringLogMessage& operator=(CStringLogMessage &&) = delete;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // CSTRINGLOGMESSAGE_HPP_202201112136