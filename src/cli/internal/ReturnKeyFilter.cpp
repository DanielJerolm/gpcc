/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "ReturnKeyFilter.hpp"

namespace gpcc     {
namespace cli      {
namespace internal {

/**
 * \brief Samples the filter.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param input
 * Input for the filter.
 * \return
 * true  = RETURN key has been pressed\n
 * false = RETURN key has not been pressed
 */
bool ReturnKeyFilter::Filter(TerminalRxParser::Result const input) noexcept
{
  if (input == TerminalRxParser::Result::NeedMoreData)
    return false;

  switch (state)
  {
    case State::normal:
    {
      if (input == TerminalRxParser::Result::CR)
      {
        state = State::ignoreNextLF;
        return true;
      }
      else if (input == TerminalRxParser::Result::LF)
      {
        state = State::ignoreNextCR;
        return true;
      }
      else
      {
        return false;
      }

      break;
    }

    case State::ignoreNextCR:
    {
      if (input == TerminalRxParser::Result::LF)
      {
        return true;
      }
      else
      {
        state = State::normal;
        return false;
      }

      break;
    }

    case State::ignoreNextLF:
    {
      if (input == TerminalRxParser::Result::CR)
      {
        return true;
      }
      else
      {
        state = State::normal;
        return false;
      }

      break;
    }
  }

  // never reached
  return false;
}

} // namespace internal
} // namespace cli
} // namespace gpcc
