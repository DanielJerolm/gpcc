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

#include "CStringLogMessageTS.hpp"
#include <cstdlib>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _srcName
 * Name of the source of the log message.
 *
 * \param _type
 * Type of log message.
 *
 * \param _pMsg
 * Pointer to a null-terminated c-string which contains the log message text.\n
 * Ownership will move to the new @ref CStringLogMessageTS instance.\n
 * nullptr is not allowed.
 */
CStringLogMessageTS::CStringLogMessageTS(string::SharedString const & _srcName,
                                         LogType const _type,
                                         std::unique_ptr<char[]> && _pMsg)
: RomConstLogMessageTS(_srcName, _type, _pMsg.get())
{
  _pMsg.release();
}

/**
 * \brief Destructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
CStringLogMessageTS::~CStringLogMessageTS(void)
{
  delete [] const_cast<char*>(pMsg);
}

} // namespace internal
} // namespace log
} // namespace gpcc
