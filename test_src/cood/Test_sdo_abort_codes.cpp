/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018 Daniel Jerolm

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

#include "gpcc/src/cood/sdo_abort_codes.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

TEST(gpcc_cood_sdo_abort_codes_Tests, SDOAbortCodeToDescrString)
{
  using namespace gpcc::cood;
  char const * p = SDOAbortCodeToDescrString(SDOAbortCode::OK);

  ASSERT_TRUE(p != nullptr);
  ASSERT_STREQ(p, "0x00000000 (OK)");
}

TEST(gpcc_cood_sdo_abort_codes_Tests, U32ToSDOAbortCode)
{
  using namespace gpcc::cood;

  // do 2 positive tests...
  SDOAbortCode sac = U32ToSDOAbortCode(0x00000000UL);
  EXPECT_TRUE(sac == SDOAbortCode::OK);

  sac = U32ToSDOAbortCode(0x06070010UL);
  EXPECT_TRUE(sac == SDOAbortCode::DataTypeMismatch);

  // ...and 1 negative test
  EXPECT_THROW(sac = U32ToSDOAbortCode(0xDEADBEEFUL), std::invalid_argument);
}

} // namespace cood
} // namespace gpcc_tests
