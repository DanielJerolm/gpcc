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

TEST(gpcc_string_StringComposer, Append_bool)
{
  // Check for the reference (std::ostringstream) and the UUT:
  // - Default settings
  // - uppercase/nouppercase has no effect

  // test reference
  std::ostringstream reference;
  reference << true << ' ' << false << ' '
            << std::boolalpha << true << ' ' << false << ' '
            << std::uppercase << true << ' ' << false << ' '
            << std::nouppercase << true << ' ' << false << ' '
            << std::noboolalpha << true << ' ' << false;
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), "1 0 true false true false true false 1 0");

  // test UUT
  StringComposer uut;
  uut << true << ' ' << false << ' '
      << StringComposer::BoolAlpha << true << ' ' << false << ' '
      << StringComposer::Uppercase << true << ' ' << false << ' '
      << StringComposer::NoUppercase << true << ' ' << false << ' '
      << StringComposer::NoBoolAlpha << true << ' ' << false;

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), "1 0 true false true false true false 1 0");
}

TEST(gpcc_string_StringComposer, Append_bool_defaultAlignmentRight)
{
  char const * const pExpectedStr = "     1;     0;  true; false;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(6) << true << ';' << std::setw(6) << false << ';'
            << std::boolalpha
            << std::setw(6) << true << ';' << std::setw(6) << false << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(6) << true << ';' << StringComposer::Width(6) << false << ';'
      << StringComposer::BoolAlpha
      << StringComposer::Width(6) << true << ';' << StringComposer::Width(6) << false << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_bool_alignRightPadZeroPadsWithWhiteSpaces)
{
  char const * const pExpectedStr = "     1;     0;  true; false;";

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero
      << StringComposer::Width(6) << true << ';' << StringComposer::Width(6) << false << ';'
      << StringComposer::BoolAlpha
      << StringComposer::Width(6) << true << ';' << StringComposer::Width(6) << false << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

} // namespace string
} // namespace gpcc_tests
