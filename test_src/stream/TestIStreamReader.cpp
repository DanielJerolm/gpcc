/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#include "gtest/gtest.h"
#include <gpcc/stream/IStreamReader.hpp>

namespace gpcc_tests {
namespace stream     {

using namespace gpcc::stream;
using namespace testing;

TEST(gpcc_Stream_IStreamReader, NativeEndian)
{
  uint16_t const v = 0x1234U;
  uint8_t const * const p = reinterpret_cast<uint8_t const *>(&v);

  if (*p == 0x34U)
  {
    // little endian
    ASSERT_EQ(IStreamReader::nativeEndian, IStreamReader::Endian::Little);
  }
  else if (*p == 0x12U)
  {
    // big endian
    ASSERT_EQ(IStreamReader::nativeEndian, IStreamReader::Endian::Big);
  }
  else
  {
    FAIL() << "Unknown endian";
  }
}

} // namespace stream
} // namespace gpcc_tests
