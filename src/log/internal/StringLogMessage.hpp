/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef STRINGLOGMESSAGE_HPP_201701061517
#define STRINGLOGMESSAGE_HPP_201701061517

#include "LogMessage.hpp"

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \class StringLogMessage StringLogMessage.hpp "src/log/internal/StringLogMessage.hpp"
 * \brief Container for the ingredients of a log message comprised of an std::string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringLogMessage final : public LogMessage
{
  public:
    StringLogMessage(void) = delete;
    StringLogMessage(string::SharedString const & _srcName,
                     LogType const _type,
                     std::string const & _msg);
    StringLogMessage(string::SharedString const & _srcName,
                     LogType const _type,
                     std::string && _msg);
    StringLogMessage(StringLogMessage const &) = delete;
    StringLogMessage(StringLogMessage &&) = delete;
    ~StringLogMessage(void) override = default;

    StringLogMessage& operator=(StringLogMessage const &) = delete;
    StringLogMessage& operator=(StringLogMessage &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Log message text.
    std::string const msg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // STRINGLOGMESSAGE_HPP_201701061517
