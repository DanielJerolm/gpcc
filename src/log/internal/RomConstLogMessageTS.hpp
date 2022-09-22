/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ROMCONSTLOGMESSAGETS_HPP_201701061515
#define ROMCONSTLOGMESSAGETS_HPP_201701061515

#include "LogMessage.hpp"
#include <gpcc/time/TimePoint.hpp>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located in ROM/code memory
 *        plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class RomConstLogMessageTS : public LogMessage
{
  public:
    RomConstLogMessageTS(void) = delete;
    RomConstLogMessageTS(string::SharedString const & _srcName,
                         LogType const _type,
                         char const * const _pMsg);
    RomConstLogMessageTS(RomConstLogMessageTS const &) = delete;
    RomConstLogMessageTS(RomConstLogMessageTS &&) = delete;
    virtual ~RomConstLogMessageTS(void) override = default;

     RomConstLogMessageTS& operator=(RomConstLogMessageTS const &) = delete;
     RomConstLogMessageTS& operator=(RomConstLogMessageTS &&) = delete;


    std::string BuildText(void) const override;

  protected:
    /// Log message text.
    /** This points to a null-terminated c-string.\n
        For objects of type @ref RomConstLogMessage, this points into ROM/code memory.\n
        For objects of type @ref CStringLogMessage, this points into the heap and the referenced memory will be released
        via `free()`. */
    char const * const pMsg;

    /// Timestamp.
    gpcc::time::TimePoint const timestamp;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // ROMCONSTLOGMESSAGETS_HPP_201701061515
