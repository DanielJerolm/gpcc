/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include <gpcc/cood/sdo_abort_codes.hpp>
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
