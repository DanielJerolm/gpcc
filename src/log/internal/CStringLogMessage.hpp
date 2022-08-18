/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2022 Daniel Jerolm
*/

#ifndef CSTRINGLOGMESSAGE_HPP_202201112136
#define CSTRINGLOGMESSAGE_HPP_202201112136

#include "RomConstLogMessage.hpp"
#include <memory>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located on the heap.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class CStringLogMessage final : public RomConstLogMessage
{
  public:
    CStringLogMessage(void) = delete;
    CStringLogMessage(string::SharedString const & _srcName,
                      LogType const _type,
                      std::unique_ptr<char[]> && _pMsg);
    CStringLogMessage(CStringLogMessage const &) = delete;
    CStringLogMessage(CStringLogMessage &&) = delete;
    ~CStringLogMessage(void) override;

    CStringLogMessage& operator=(CStringLogMessage const &) = delete;
    CStringLogMessage& operator=(CStringLogMessage &&) = delete;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // CSTRINGLOGMESSAGE_HPP_202201112136