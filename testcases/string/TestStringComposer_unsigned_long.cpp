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

TEST(gpcc_string_StringComposer, Append_ulong_defaults)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "37;133;";

  // test reference
  std::ostringstream reference;
  reference << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_defaultAlignmentRight)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "    37;   133;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(6) << v1 << ';' << std::setw(6) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(6) << v1 << ';' << StringComposer::Width(6) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_alignLeft_widthIsNotSticky)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "37    ;133;";

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

TEST(gpcc_string_StringComposer, Append_ulong_alignLeftIsSticky)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "37    ;133   ;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(6) << v1 << ';' << std::setw(6) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(6) << v1 << ';' << StringComposer::Width(6) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(),pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_alignRight_widthIsNotSticky)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "    37;133;";

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

TEST(gpcc_string_StringComposer, Append_ulong_alignRightIsSticky)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "    37;   133;";

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

TEST(gpcc_string_StringComposer, Append_ulong_alignRightPadZeroIsSticky)
{
  unsigned long v1 = 37;
  unsigned long v2 = 133;

  char const * const pExpectedStr = "000037;000133;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << std::setfill('0') << v1 << ';'
                          << std::setw(6) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::Width(6) << v1 << ';'
                                           << StringComposer::Width(6) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_showPosHasNoEffect)
{
  unsigned long v1 = 37;

  char const * const pExpectedStr = "    37;";

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

TEST(gpcc_string_StringComposer, Append_ulong_baseHexIsSticky)
{
  unsigned long v1 = 184;
  unsigned long v2 = 44;

  char const * const pExpectedStr = "b8;2c;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_showBaseIsSticky)
{
  unsigned long v1 = 184;
  unsigned long v2 = 44;

  char const * const pExpectedStr = "0xb8;0x2c;2c;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << std::showbase << v1 << ';' << v2 << ';' << std::noshowbase << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex
      << StringComposer::ShowBase << v1 << ';' << v2 << ';'
      << StringComposer::NoShowBase << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_baseHex_alignRightPadZero)
{
  unsigned long v1 = 184;
  unsigned long v2 = 44;

  char const * const pExpectedStr = "0000b8;00002C;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << std::setfill('0')
            << std::setw(6) << v1 << ';' << std::uppercase << std::setw(6) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << StringComposer::AlignRightPadZero
      << StringComposer::Width(6) << v1 << ';' << StringComposer::Uppercase << StringComposer::Width(6) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_baseHex_showBase_alignRightPadZero)
{
  unsigned long v1 = 184;
  unsigned long v2 = 44;

  char const * const pExpectedStr = "0x00b8;0X002C;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << StringComposer::AlignRightPadZero
      << StringComposer::ShowBase
      << StringComposer::Width(6) << v1 << ';' << StringComposer::Uppercase << StringComposer::Width(6) << v2 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_uppercaseIsSticky)
{
  unsigned long v1 = 184;
  unsigned long v2 = 44;

  char const * const pExpectedStr = "0XB8;0X2C;0x2c;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << std::showbase << std::uppercase << v1 << ';' << v2 << ';' << std::nouppercase << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << StringComposer::ShowBase
      << StringComposer::Uppercase << v1 << ';' << v2 << ';'
      << StringComposer::NoUppercase << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_baseOctal)
{
  unsigned long v = 467;

  char const * const pExpectedStr = "   723;";

  // test reference
  std::ostringstream reference;
  reference << std::oct << std::setw(6) << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseOct << StringComposer::Width(6) << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ulong_MinMax)
{
  unsigned long min = std::numeric_limits<unsigned long>::min();
  unsigned long max = std::numeric_limits<unsigned long>::max();

  std::string const expected = std::to_string(min) + ';' + std::to_string(max) + ';';

  // test reference
  std::ostringstream reference;
  reference << min << ';' << max << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), expected.c_str());

  // test UUT
  StringComposer uut;
  uut << min << ';' << max << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), expected.c_str());
}

TEST(gpcc_string_StringComposer, Append_ulong_ExceedFieldWidth)
{
  unsigned long v = 1000;

  char const * const pExpectedStr = "1000;1000;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(2) << v << ';' << std::setw(2) << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(2) << v << ';' << StringComposer::Width(2) << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

} // namespace string
} // namespace gpcc_tests
