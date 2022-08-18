/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
