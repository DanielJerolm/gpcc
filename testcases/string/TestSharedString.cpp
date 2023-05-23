/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/string/SharedString.hpp>
#include <gtest/gtest.h>

namespace gpcc_tests {
namespace string     {

using gpcc::string::SharedString;

TEST(gpcc_string_SharedString_Tests, CTOR_From_c_string)
{
  SharedString uut("Test");
  EXPECT_STREQ(uut.GetStr().c_str(), "Test");
}

TEST(gpcc_string_SharedString_Tests, CTOR_From_c_string_nullptr)
{
  EXPECT_THROW(SharedString uut(nullptr), std::invalid_argument);
}

TEST(gpcc_string_SharedString_Tests, CTOR_From_String)
{
  std::string const s("Test");

  SharedString uut(s);
  EXPECT_STREQ(uut.GetStr().c_str(), "Test");
}

TEST(gpcc_string_SharedString_Tests, CTOR_From_MovedString)
{
  std::string s("Test");

  SharedString uut(std::move(s));
  EXPECT_STREQ(uut.GetStr().c_str(), "Test");

}

TEST(gpcc_string_SharedString_Tests, CopyCTOR_From_SharedString)
{
  SharedString uut("Test");
  SharedString uut2(uut);
  EXPECT_STREQ(uut.GetStr().c_str(), "Test");
  EXPECT_STREQ(uut2.GetStr().c_str(), "Test");
  EXPECT_TRUE(&(uut.GetStr()) == &(uut2.GetStr())) << "Container objects should be the same, but they are not.";
}

TEST(gpcc_string_SharedString_Tests, CopyAssign_SharedString_A)
{
  SharedString uut("Test");
  SharedString uut2("ABC");

  uut = uut2;
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
  EXPECT_STREQ(uut2.GetStr().c_str(), "ABC");
  EXPECT_TRUE(&(uut.GetStr()) == &(uut2.GetStr())) << "Container objects should be the same, but they are not.";
}

TEST(gpcc_string_SharedString_Tests, CopyAssign_SharedString_B)
{
  SharedString uut("Test");
  SharedString uut2("ABC");
  SharedString uut3(uut);

  uut = uut2;
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
  EXPECT_STREQ(uut2.GetStr().c_str(), "ABC");
  EXPECT_TRUE(&(uut.GetStr()) == &(uut2.GetStr())) << "Container objects should be the same, but they are not.";
  EXPECT_STREQ(uut3.GetStr().c_str(), "Test") << "Copy of uut was affected!";
}

TEST(gpcc_string_SharedString_Tests, CopyAssign_Self)
{
  SharedString uut("Test");

  uut = uut;
  EXPECT_STREQ(uut.GetStr().c_str(), "Test");
}

TEST(gpcc_string_SharedString_Tests, MoveAssign_SharedString_A)
{
  SharedString uut("Test");
  SharedString uut2("ABC");

  uut = std::move(uut2);
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
}

TEST(gpcc_string_SharedString_Tests, MoveAssign_SharedString_B)
{
  SharedString uut("Test");
  SharedString uut2("ABC");
  SharedString uut3(uut2);
  SharedString uut4(uut);

  uut = std::move(uut2);
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
  EXPECT_STREQ(uut3.GetStr().c_str(), "ABC") << "Copy of uut2 was affected!";
  EXPECT_STREQ(uut4.GetStr().c_str(), "Test") << "Copy of uut was affected!";
}

TEST(gpcc_string_SharedString_Tests, Assign_c_string_A)
{
  SharedString uut("Test");

  uut = "ABC";
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
}

TEST(gpcc_string_SharedString_Tests, Assign_c_string_B)
{
  SharedString uut("Test");
  SharedString uut2(uut);

  uut = "ABC";
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
  EXPECT_STREQ(uut2.GetStr().c_str(), "Test") << "Copy of uut was affected!";
}

TEST(gpcc_string_SharedString_Tests, Assign_c_string_nullptr)
{
  SharedString uut("Test");

  ASSERT_THROW(uut = nullptr, std::invalid_argument);
  EXPECT_STREQ(uut.GetStr().c_str(), "Test") << "Strong guarantee not met.";
}

TEST(gpcc_string_SharedString_Tests, CopyAssign_stdstring_A)
{
  std::string const s("ABC");
  SharedString uut("Test");

  uut = s;
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
}

TEST(gpcc_string_SharedString_Tests, CopyAssign_stdstring_B)
{
  std::string const s("ABC");
  SharedString uut("Test");
  SharedString uut2(uut);

  uut = s;
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
  EXPECT_STREQ(uut2.GetStr().c_str(), "Test") << "Copy of uut was affected!";
}

TEST(gpcc_string_SharedString_Tests, MoveAssign_stdstring_A)
{
  std::string s("ABC");
  SharedString uut("Test");

  uut = std::move(s);
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
}

TEST(gpcc_string_SharedString_Tests, MoveAssign_stdstring_B)
{
  std::string s("ABC");
  SharedString uut("Test");
  SharedString uut2(uut);

  uut = std::move(s);
  EXPECT_STREQ(uut.GetStr().c_str(), "ABC");
  EXPECT_STREQ(uut2.GetStr().c_str(), "Test") << "Copy of uut was affected!";
}

} // namespace string
} // namespace gpcc_tests
