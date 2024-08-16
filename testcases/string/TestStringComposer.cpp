/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#include <gpcc/string/StringComposer.hpp>
#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>

namespace gpcc_tests {
namespace string {

using gpcc::string::StringComposer;

TEST(gpcc_string_StringComposer, CTOR_Std)
{
  gpcc::string::StringComposer uut;

  std::string s = uut.Get();
  EXPECT_TRUE(s.empty());
}

TEST(gpcc_string_StringComposer, CTOR_cstring)
{
  gpcc::string::StringComposer uut("Test");

  std::string s = uut.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, CTOR_Str)
{
  std::string const original("Test");
  gpcc::string::StringComposer uut(original);

  std::string s = uut.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, CTOR_StrMove)
{
  std::string original("Test");
  gpcc::string::StringComposer uut(std::move(original));

  std::string s = uut.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, CopyCTOR)
{
  gpcc::string::StringComposer uut1("Test");
  gpcc::string::StringComposer uut2(uut1);

  std::string s = uut1.Get();
  EXPECT_STREQ(s.c_str(), "Test");

  uut2 << '2';
  s = uut2.Get();
  EXPECT_STREQ(s.c_str(), "Test2");
}

TEST(gpcc_string_StringComposer, MoveCTOR)
{
  gpcc::string::StringComposer uut1("Test");
  gpcc::string::StringComposer uut2(std::move(uut1));

  uut2 << '2';
  std::string s = uut2.Get();
  EXPECT_STREQ(s.c_str(), "Test2");
}

TEST(gpcc_string_StringComposer, CopyAssignment)
{
  gpcc::string::StringComposer uut1("Test");
  gpcc::string::StringComposer uut2("Blah");

  uut2 = uut1;

  std::string s = uut1.Get();
  EXPECT_STREQ(s.c_str(), "Test");
  s = uut2.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, MoveAssignment)
{
  gpcc::string::StringComposer uut1("Test");
  gpcc::string::StringComposer uut2("Blah");

  uut2 = std::move(uut1);

  std::string s = uut2.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, Clear)
{
  gpcc::string::StringComposer uut;

  uut.Set("Test");
  uut.Clear();

  std::string const s = uut.Get();
  EXPECT_STREQ(s.c_str(), "");
}

TEST(gpcc_string_StringComposer, SetCString)
{
  gpcc::string::StringComposer uut;

  uut.Set("Test");

  std::string const s = uut.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, SetStr)
{
  gpcc::string::StringComposer uut;

  std::string const original = "Test";
  uut.Set(original);

  std::string const s = uut.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

TEST(gpcc_string_StringComposer, SetMoveStr)
{
  gpcc::string::StringComposer uut;

  std::string original = "Test";
  uut.Set(std::move(original));

  std::string const s = uut.Get();
  EXPECT_STREQ(s.c_str(), "Test");
}

} // namespace string
} // namespace gpcc_tests
