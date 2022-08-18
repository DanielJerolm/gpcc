/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef ROMCONSTEXCEPTIONLOGMESSAGETS_HPP_202111211045
#define ROMCONSTEXCEPTIONLOGMESSAGETS_HPP_202111211045

#include "LogMessage.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include <exception>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located in ROM/code memory and an
 *        exception conserved in an std::exception_ptr plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class RomConstExceptionLogMessageTS final : public LogMessage
{
  public:
    RomConstExceptionLogMessageTS(void) = delete;
    RomConstExceptionLogMessageTS(string::SharedString const & _srcName,
                                  LogType const _type,
                                  char const * const _pMsg,
                                  std::exception_ptr const & _ePtr);
    RomConstExceptionLogMessageTS(RomConstExceptionLogMessageTS const &) = delete;
    RomConstExceptionLogMessageTS(RomConstExceptionLogMessageTS &&) = delete;
    ~RomConstExceptionLogMessageTS(void) override = default;

    RomConstExceptionLogMessageTS& operator=(RomConstExceptionLogMessageTS const &) = delete;
    RomConstExceptionLogMessageTS& operator=(RomConstExceptionLogMessageTS &&) = delete;


    std::string BuildText(void) const override;

  private:
    /// Log message text.
    /** This points to a null-terminated c-string, which is located in ROM/code memory. */
    char const * const pMsg;

    /// Exception to be build into the log message.
    std::exception_ptr const ePtr;

    /// Timestamp.
    gpcc::time::TimePoint const timestamp;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // ROMCONSTEXCEPTIONLOGMESSAGETS_HPP_202111211045
