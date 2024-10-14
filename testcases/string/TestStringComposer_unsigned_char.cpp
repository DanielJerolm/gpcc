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
#include <limits>
#include <sstream>
#include <string>

namespace gpcc_tests {
namespace string {

using gpcc::string::StringComposer;

TEST(gpcc_string_StringComposer, Append_uchar_defaults)
{
  unsigned char v = 'A';

  char const * const pExpectedStr = "A;";

  // test reference
  std::ostringstream reference;
  reference << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_defaultAlignmentRight)
{
  unsigned char v = 'A';

  char const * const pExpectedStr = "     A;     A;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(6) << v << ';' << std::setw(6) << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(6) << v << ';' << StringComposer::Width(6) << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_alignLeft_widthIsNotSticky)
{
  unsigned char v1 = 'A';
  unsigned char v2 = 'B';

  char const * const pExpectedStr = "A     ;B;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(6) << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(6) << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_alignLeftIsSticky)
{
  unsigned char v1 = 'A';
  unsigned char v2 = 'B';

  char const * const pExpectedStr = "A     ;B     ;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(6) << v1 << ';' << std::setw(6) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(6) << v1 << ';' << StringComposer::Width(6) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_alignRight_widthIsNotSticky)
{
  unsigned char v1 = 'A';
  unsigned char v2 = 'B';

  char const * const pExpectedStr = "     A;B;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(6) << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_alignRightIsSticky)
{
  unsigned char v1 = 'A';
  unsigned char v2 = 'B';

  char const * const pExpectedStr = "     A;     B;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << v1 << ';' << std::setw(6) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(6) << v1 << ';' << StringComposer::Width(6) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_alignRightPadZeroPadsWithWhiteSpaces)
{
  unsigned char v1 = 'A';
  unsigned char v2 = 'B';

  char const * const pExpectedStr = "     A;     B;";

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::Width(6) << v1 << ';' << StringComposer::Width(6) << v2 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_showPosHasNoEffect)
{
  unsigned char v1 = 'A';

  char const * const pExpectedStr = "     A;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << std::showpos << v1 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(6) << StringComposer::ShowPos << v1 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_baseHexHasNoEffect)
{
  unsigned char v = 'A';

  char const * const pExpectedStr = "A;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_baseOctHasNoEffect)
{
  unsigned char v = 'A';

  char const * const pExpectedStr = "A;";

  // test reference
  std::ostringstream reference;
  reference << std::oct << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseOct << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_showBaseHasNoEffect)
{
  unsigned char v = 'A';

  char const * const pExpectedStr = "A;";

  // test reference
  std::ostringstream reference;
  reference << std::showbase << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::ShowBase << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_uchar_uppercaseHasNoEffect)
{
  unsigned char v = 'a';

  char const * const pExpectedStr = "a;";

  // test reference
  std::ostringstream reference;
  reference << std::uppercase << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Uppercase << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

} // namespace string
} // namespace gpcc_tests
