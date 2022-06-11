/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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
