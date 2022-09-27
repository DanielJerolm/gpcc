/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2022 Daniel Jerolm
*/

#include "CStringLogMessage.hpp"
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
 * Ownership will move to the new @ref CStringLogMessage instance.\n
 * nullptr is not allowed.
 */
CStringLogMessage::CStringLogMessage(string::SharedString const & _srcName,
                                     LogType const _type,
                                     std::unique_ptr<char[]> && _pMsg)
: RomConstLogMessage(_srcName, _type, _pMsg.get())
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
CStringLogMessage::~CStringLogMessage(void)
{
  delete [] const_cast<char*>(pMsg);
}

} // namespace internal
} // namespace log
} // namespace gpcc
