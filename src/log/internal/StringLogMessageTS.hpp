/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef STRINGLOGMESSAGETS_HPP_201701061519
#define STRINGLOGMESSAGETS_HPP_201701061519

#include "LogMessage.hpp"
#include <gpcc/time/TimePoint.hpp>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \class StringLogMessageTS StringLogMessageTS.hpp "src/log/internal/StringLogMessageTS.hpp"
 * \brief Container for the ingredients of a log message comprised of an std::string plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringLogMessageTS final : public LogMessage
{
  public:
    StringLogMessageTS(void) = delete;
    StringLogMessageTS(string::SharedString const & _srcName,
                       LogType const _type,
                       std::string const & _msg);
    StringLogMessageTS(string::SharedString const & _srcName,
                       LogType const _type,
                       std::string && _msg);
    StringLogMessageTS(StringLogMessageTS const &) = delete;
    StringLogMessageTS(StringLogMessageTS &&) = delete;
    ~StringLogMessageTS(void) override = default;

    StringLogMessageTS& operator=(StringLogMessageTS const &) = delete;
    StringLogMessageTS& operator=(StringLogMessageTS &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Timestamp
    gpcc::time::TimePoint const timestamp;

    /// Log message text.
    std::string const msg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // STRINGLOGMESSAGETS_HPP_201701061519
