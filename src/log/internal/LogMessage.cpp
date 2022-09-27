/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "LogMessage.hpp"

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
 */
LogMessage::LogMessage(string::SharedString const & _srcName, LogType const _type)
: srcName(_srcName)
, type(static_cast<uint8_t>(_type))
, pNext(nullptr)
{
};

} // namespace internal
} // namespace log
} // namespace gpcc
