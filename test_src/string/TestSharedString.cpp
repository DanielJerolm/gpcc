/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "gpcc/src/string/SharedString.hpp"
#include "gtest/gtest.h"

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
