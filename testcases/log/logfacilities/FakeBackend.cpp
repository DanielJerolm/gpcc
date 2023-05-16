/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "FakeBackend.hpp"
#include <stdexcept>

namespace gpcc_tests {
namespace log {

void FakeBackend::Process(std::string const & msg, gpcc::log::LogType const type)
{
  (void)type;

  if (logsTillThrow != 0)
  {
    logsTillThrow--;
    if (logsTillThrow == 0)
      throw std::runtime_error("Intentionally thrown exception from FakeBackend");
  }

  records.push_back(msg);
};

} // namespace log
} // namespace gpcc_tests
