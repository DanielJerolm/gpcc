/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019 Daniel Jerolm

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

#include "gtest/gtest.h"
#include "gpcc/src/Stream/IStreamWriter.hpp"

namespace gpcc_tests {
namespace Stream     {

using namespace gpcc::Stream;
using namespace testing;

TEST(gpcc_Stream_IStreamWriter, NativeEndian)
{
  uint16_t const v = 0x1234U;
  uint8_t const * const p = reinterpret_cast<uint8_t const *>(&v);

  if (*p == 0x34U)
  {
    // little endian
    ASSERT_EQ(IStreamWriter::nativeEndian, IStreamWriter::Endian::Little);
  }
  else if (*p == 0x12U)
  {
    // big endian
    ASSERT_EQ(IStreamWriter::nativeEndian, IStreamWriter::Endian::Big);
  }
  else
  {
    FAIL() << "Unknown endian";
  }
}

} // namespace Stream
} // namespace gpcc_tests
