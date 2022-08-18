/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2022 Daniel Jerolm
*/

#ifndef CSTRINGLOGMESSAGETS_HPP_202201122144
#define CSTRINGLOGMESSAGETS_HPP_202201122144

#include "RomConstLogMessageTS.hpp"
#include <memory>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Container for the ingredients of a log message composed of a c-string located on the heap plus a timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class CStringLogMessageTS final : public RomConstLogMessageTS
{
  public:
    CStringLogMessageTS(void) = delete;
    CStringLogMessageTS(string::SharedString const & _srcName,
                        LogType const _type,
                        std::unique_ptr<char[]> && _pMsg);
    CStringLogMessageTS(CStringLogMessageTS const &) = delete;
    CStringLogMessageTS(CStringLogMessageTS &&) = delete;
    ~CStringLogMessageTS(void) override;

    CStringLogMessageTS& operator=(CStringLogMessageTS const &) = delete;
    CStringLogMessageTS& operator=(CStringLogMessageTS &&) = delete;
};

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // CSTRINGLOGMESSAGETS_HPP_202201122144