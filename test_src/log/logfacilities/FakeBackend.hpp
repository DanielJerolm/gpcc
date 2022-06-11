/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#ifndef FAKEBACKEND_HPP_201701061552
#define FAKEBACKEND_HPP_201701061552

#include "gpcc/src/log/backends/Backend.hpp"
#include "gpcc/src/log/log_levels.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace log {

// Fake Backend for log-facilities.
// This backend records all log messages in an public std::vector.
// It is possible to intentionally throw an exception in Process() by setting logsTillThrow
// to an value larger than zero.
class FakeBackend final: public gpcc::log::Backend
{
  public:
    std::vector<std::string> records;
    uint8_t logsTillThrow = 0;

    FakeBackend(void) = default;
    virtual ~FakeBackend(void) = default;

    void Process(std::string const & msg, gpcc::log::LogType const type);
};

} // namespace log
} // namespace gpcc_tests

#endif // FAKEBACKEND_HPP_201701061552
