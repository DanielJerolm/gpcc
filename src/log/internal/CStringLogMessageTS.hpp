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

#ifndef CSTRINGLOGMESSAGETS_HPP_202201122144
#define CSTRINGLOGMESSAGETS_HPP_202201122144

#include "RomConstLogMessageTS.hpp"
#include <memory>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located on the heap plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class CStringLogMessageTS final : public RomConstLogMessageTS
{
  public:
    CStringLogMessageTS(void) = delete;
    CStringLogMessageTS(string::SharedString const & _srcName,
                        LogType const _type,
                        std::unique_ptr<char[]> && _pMsg);
    CStringLogMessageTS(CStringLogMessageTS const &) = delete;
    CStringLogMessageTS(CStringLogMessageTS &&) = delete;
    ~CStringLogMessageTS(void) override;

    CStringLogMessageTS& operator=(CStringLogMessageTS const &) = delete;
    CStringLogMessageTS& operator=(CStringLogMessageTS &&) = delete;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // CSTRINGLOGMESSAGETS_HPP_202201122144