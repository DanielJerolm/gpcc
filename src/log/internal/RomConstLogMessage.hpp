/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ROMCONSTLOGMESSAGE_HPP_201701061513
#define ROMCONSTLOGMESSAGE_HPP_201701061513

#include "LogMessage.hpp"

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \class RomConstLogMessage RomConstLogMessage.hpp "src/log/internal/RomConstLogMessage.hpp"
 * \brief Container for the ingredients of a log message composed of a c-string located in ROM/code memory.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class RomConstLogMessage : public LogMessage
{
  public:
    RomConstLogMessage(void) = delete;
    RomConstLogMessage(string::SharedString const & _srcName,
                       LogType const _type,
                       char const * const _pMsg);
    RomConstLogMessage(RomConstLogMessage const &) = delete;
    RomConstLogMessage(RomConstLogMessage &&) = delete;
    virtual ~RomConstLogMessage(void) override = default;

    RomConstLogMessage& operator=(RomConstLogMessage const &) = delete;
    RomConstLogMessage& operator=(RomConstLogMessage &&) = delete;


    std::string BuildText(void) const override;

  protected:
    /// Log message text.
    /** This points to a null-terminated c-string.\n
        For objects of type @ref RomConstLogMessage, this points into ROM/code memory.\n
        For objects of type @ref CStringLogMessage, this points into the heap and the referenced memory will be released
        via `free()`. */
    char const * const pMsg;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // ROMCONSTLOGMESSAGE_HPP_201701061513
