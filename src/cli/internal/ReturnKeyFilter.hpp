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

#ifndef RETURNKEYFILTER_HPP_201711021005
#define RETURNKEYFILTER_HPP_201711021005

#include "TerminalRxParser.hpp"

namespace gpcc     {
namespace cli      {
namespace internal {

/**
 * \ingroup GPCC_CLI_INTERNALS
 * \brief Filter for RETURN key codes (CR/LF) used by class @ref CLI.
 *
 * This class is a helper for class @ref CLI.
 *
 * # Rationale
 * Different terminals and terminal emulation software use different key codes for the
 * RETURN/ENTER-key:
 * - CR
 * - LF
 * - CR LF
 * - LF CR
 *
 * This filter is fed with the output of class @ref TerminalRxParser and detects if the
 * RETURN-key has been pressed. The filter is aware of CR-LF and LF-CR sequences an prevents
 * double recognition of the RETURN-key, except the key was _really_ pressed twice or more
 * times.
 *
 * # Usage
 * All return values from the @ref TerminalRxParser shall be passed to @ref Filter(). \n
 * @ref Filter() will return true, if ENTER has been pressed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ReturnKeyFilter final
{
  public:
    ReturnKeyFilter(void) = default;
    ReturnKeyFilter(ReturnKeyFilter const &) = default;
    ReturnKeyFilter(ReturnKeyFilter &&) noexcept = default;
    ~ReturnKeyFilter(void) = default;

    ReturnKeyFilter& operator=(ReturnKeyFilter const &) = default;
    ReturnKeyFilter& operator=(ReturnKeyFilter &&) = default;

    bool Filter(TerminalRxParser::Result const input) noexcept;

  private:
    /// Enumeration with internal states of the filter.
    enum class State
    {
      normal,         ///<There is no CR/LF sequence currently present.
      ignoreNextCR,   ///<If the next key is CR, then it shall be ignored.
      ignoreNextLF    ///<If the next key is LF, then it shall be ignored.
    };

    /// Current state of the filter.
    State state = State::normal;
};

} // namespace internal
} // namespace cli
} // namespace gpcc

#endif // RETURNKEYFILTER_HPP_201711021005
