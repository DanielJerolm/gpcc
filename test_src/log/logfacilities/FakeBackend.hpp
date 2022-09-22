/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef FAKEBACKEND_HPP_201701061552
#define FAKEBACKEND_HPP_201701061552

#include <gpcc/log/backends/Backend.hpp>
#include <gpcc/log/log_levels.hpp>
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
