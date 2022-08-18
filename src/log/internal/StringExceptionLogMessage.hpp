/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef STRINGEXCEPTIONLOGMESSAGE_HPP_201801262128
#define STRINGEXCEPTIONLOGMESSAGE_HPP_201801262128

#include "LogMessage.hpp"
#include <exception>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of an std::string and an exception conserved in a std::exception_ptr.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringExceptionLogMessage final : public LogMessage
{
  public:
    StringExceptionLogMessage(void) = delete;
    StringExceptionLogMessage(string::SharedString const & _srcName,
                              LogType const _type,
                              std::string const & _msg,
                              std::exception_ptr const & _ePtr);
    StringExceptionLogMessage(string::SharedString const & _srcName,
                              LogType const _type,
                              std::string && _msg,
                              std::exception_ptr const & _ePtr);
    StringExceptionLogMessage(StringExceptionLogMessage const &) = delete;
    StringExceptionLogMessage(StringExceptionLogMessage &&) = delete;
    ~StringExceptionLogMessage(void) override = default;

    StringExceptionLogMessage& operator=(StringExceptionLogMessage const &) = delete;
    StringExceptionLogMessage& operator=(StringExceptionLogMessage &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Exception to be build into the log message.
    std::exception_ptr const ePtr;

    /// Log message text.
    std::string const msg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // STRINGEXCEPTIONLOGMESSAGE_HPP_201801262128
