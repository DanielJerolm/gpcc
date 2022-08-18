/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "Backend.hpp"

namespace gpcc {
namespace log  {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 */
Backend::Backend(void) noexcept
: registered(false)
, pNext(nullptr)
{
}

} // namespace log
} // namespace gpcc
