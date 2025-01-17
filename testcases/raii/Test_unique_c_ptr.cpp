/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include <gpcc/raii/unique_c_ptr.hpp>
#include <gtest/gtest.h>
#include <new>
#include <cstdlib>
#include <cstring>

namespace
{
  char* CreateHelloString(void)
  {
    char* const p = reinterpret_cast<char*>(malloc(7U));
    if (p == nullptr)
      throw std::bad_alloc();

    std::strcpy(p, "Hello!");

    return p;
  }

  char* CreateByeString(void)
  {
    char* const p = reinterpret_cast<char*>(malloc(5U));
    if (p == nullptr)
      throw std::bad_alloc();

    std::strcpy(p, "Bye!");

    return p;
  }

  uint8_t* Create1to4(void)
  {
    uint8_t* const p = reinterpret_cast<uint8_t*>(malloc(4U));
    if (p == nullptr)
      throw std::bad_alloc();

    p[0] = 1U;
    p[1] = 2U;
    p[2] = 3U;
    p[3] = 4U;

    return p;
  }
}

namespace gpcc_tests {
namespace raii       {

TEST(gpcc_raii_unique_c_ptr_Tests, Init_without_data)
{
  gpcc::raii::unique_c_ptr<char> spUUT;
  EXPECT_TRUE(spUUT.get() == nullptr);
}

TEST(gpcc_raii_unique_c_ptr_Tests, Init_nullptr)
{
  gpcc::raii::unique_c_ptr<char> spUUT(nullptr);
  EXPECT_TRUE(spUUT.get() == nullptr);
}

TEST(gpcc_raii_unique_c_ptr_Tests, Init_withNonConstData)
{
  gpcc::raii::unique_c_ptr<char> spUUT(CreateHelloString());

  EXPECT_STREQ(spUUT.get(), "Hello!");
}

TEST(gpcc_raii_unique_c_ptr_Tests, Init_withConstData)
{
  gpcc::raii::unique_c_ptr<char const> spUUT(CreateHelloString());

  EXPECT_STREQ(spUUT.get(), "Hello!");
}

TEST(gpcc_raii_unique_c_ptr_Tests, Reset)
{
  gpcc::raii::unique_c_ptr<char> spUUT(CreateHelloString());

  // memcheck should not find a leak
  spUUT.reset();
}

TEST(gpcc_raii_unique_c_ptr_Tests, Reset_withNewData)
{
  gpcc::raii::unique_c_ptr<char> spUUT(CreateHelloString());

  // memcheck should not find a leak
  spUUT.reset(CreateByeString());

  EXPECT_STREQ(spUUT.get(), "Bye!");
}

TEST(gpcc_raii_unique_c_ptr_Tests, Release)
{
  gpcc::raii::unique_c_ptr<char> spUUT(CreateHelloString());

  char* const p = spUUT.release();
  ASSERT_EQ(spUUT.get(), nullptr);

  // memcheck should not find use-after-free
  EXPECT_STREQ(p, "Hello!");

  std::free(p);
}

TEST(gpcc_raii_unique_c_ptr_Tests, Dereference)
{
  gpcc::raii::unique_c_ptr<uint8_t> spUUT(Create1to4());

  EXPECT_EQ(*spUUT, 1U);
  EXPECT_EQ(spUUT.get()[2], 3U);
}

} // namespace raii
} // namespace gpcc_tests
