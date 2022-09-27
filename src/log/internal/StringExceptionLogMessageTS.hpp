/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef STRINGEXCEPTIONLOGMESSAGETS_HPP_202111211426
#define STRINGEXCEPTIONLOGMESSAGETS_HPP_202111211426

#include "LogMessage.hpp"
#include <gpcc/time/TimePoint.hpp>
#include <exception>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \class StringExceptionLogMessageTS StringExceptionLogMessageTS.hpp "src/log/internal/StringExceptionLogMessageTS.hpp"
 * \brief Container for the ingredients of a log message composed of an std::string and an exception conserved in a std::exception_ptr
 *        plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringExceptionLogMessageTS final : public LogMessage
{
  public:
    StringExceptionLogMessageTS(void) = delete;
    StringExceptionLogMessageTS(string::SharedString const & _srcName,
                                LogType const _type,
                                std::string const & _msg,
                                std::exception_ptr const & _ePtr);
    StringExceptionLogMessageTS(string::SharedString const & _srcName,
                                LogType const _type,
                                std::string && _msg,
                                std::exception_ptr const & _ePtr);
    StringExceptionLogMessageTS(StringExceptionLogMessageTS const &) = delete;
    StringExceptionLogMessageTS(StringExceptionLogMessageTS &&) = delete;
    ~StringExceptionLogMessageTS(void) override = default;

    StringExceptionLogMessageTS& operator=(StringExceptionLogMessageTS const &) = delete;
    StringExceptionLogMessageTS& operator=(StringExceptionLogMessageTS &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Exception to be build into the log message.
    std::exception_ptr const ePtr;

    /// Timestamp
    gpcc::time::TimePoint const timestamp;

    /// Log message text.
    std::string const msg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // STRINGEXCEPTIONLOGMESSAGETS_HPP_202111211426
