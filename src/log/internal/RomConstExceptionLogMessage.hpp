/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ROMCONSTEXCEPTIONLOGMESSAGE_HPP_201801312035
#define ROMCONSTEXCEPTIONLOGMESSAGE_HPP_201801312035

#include "LogMessage.hpp"
#include <exception>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located in ROM/code memory and an
 *        exception conserved in an std::exception_ptr.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class RomConstExceptionLogMessage final : public LogMessage
{
  public:
    RomConstExceptionLogMessage(void) = delete;
    RomConstExceptionLogMessage(string::SharedString const & _srcName,
                                LogType const _type,
                                char const * const _pMsg,
                                std::exception_ptr const & _ePtr);
    RomConstExceptionLogMessage(RomConstExceptionLogMessage const &) = delete;
    RomConstExceptionLogMessage(RomConstExceptionLogMessage &&) = delete;
    ~RomConstExceptionLogMessage(void) override = default;

    RomConstExceptionLogMessage& operator=(RomConstExceptionLogMessage const &) = delete;
    RomConstExceptionLogMessage& operator=(RomConstExceptionLogMessage &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Log message text.
    /** This points to a null-terminated c-string, which is located in ROM/code memory. */
    char const * const pMsg;

    /// Exception to be build into the log message.
    std::exception_ptr const ePtr;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // ROMCONSTEXCEPTIONLOGMESSAGE_HPP_201801312035
