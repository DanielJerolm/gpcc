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

TEST(gpcc_string_StringComposer, Append_longlong_defaults)
{
  long long vp = 37;
  long long vn = -133;

  char const * const pExpectedStr = "37;-133;";

  // test reference
  std::ostringstream reference;
  reference << vp << ';' << vn << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << vp << ';' << vn << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_defaultAlignmentRight)
{
  long long vp = 37;
  long long vn = -133;

  char const * const pExpectedStr = "    37;  -133;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(6) << vp << ';' << std::setw(6) << vn << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(6) << vp << ';' << StringComposer::Width(6) << vn << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_alignLeft_widthIsNotSticky)
{
  long long vp = 37;
  long long vn = -133;

  char const * const pExpectedStr = "37    ;-133;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(6) << vp << ';' << vn << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(6) << vp << ';' << vn << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_alignLeftIsSticky)
{
  long long vp = 37;
  long long vn = -133;

  char const * const pExpectedStr = "37    ;-133  ;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(6) << vp << ';' << std::setw(6) << vn << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(6) << vp << ';' << StringComposer::Width(6) << vn << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(),pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_alignRight_widthIsNotSticky)
{
  long long vp = 37;
  long long vn = -133;

  char const * const pExpectedStr = "    37;-133;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << vp << ';' << vn << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(6) << vp << ';' << vn << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_alignRightIsSticky)
{
  long long vp = 37;
  long long vn = -133;

  char const * const pExpectedStr = "    37;  -133;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << vp << ';' << std::setw(6) << vn << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(6) << vp << ';' << StringComposer::Width(6) << vn << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_alignRightPadZeroIsSticky)
{
  long long v1 = 37;
  long long v2 = 133;
  long long v3 = -133;

  char const * const pExpectedStr = "000037;+00133;-00133;";

  // The reference (std::ostringstream) is not tested. It pads in front of the sign.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::Width(6) << v1 << ';'
      << StringComposer::ShowPos           << StringComposer::Width(6) << v2 << ';'
                                           << StringComposer::Width(6) << v3 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_showPosIsSticky)
{
  long long vp1 = 37;
  long long vn = -12;
  long long vp2 = 133;

  char const * const pExpectedStr = "   +37;   -12;  +133;   133;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(6) << std::showpos << vp1 << ';'
                          << std::setw(6) << vn << ';'
                          << std::setw(6) << vp2 << ';'
                          << std::noshowpos << std::setw(6) << vp2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(6) << StringComposer::ShowPos << vp1 << ';'
                                      << StringComposer::Width(6) << vn << ';'
                                      << StringComposer::Width(6) << vp2 << ';'
                                      << StringComposer::NoShowPos << StringComposer::Width(6) << vp2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longlong_baseHexIsSticky)
{
  long long v1 = 184;
  long long v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_longlong_baseHexNegativeValue)
{
  long long v = -184;

  // test UUT against reference, since result depends on CPU architecture (32/64 bit)
  std::ostringstream reference;
  reference << std::hex << v << ';';
  std::string s_ref(reference.str());

  StringComposer uut;
  uut << StringComposer::BaseHex << v << ';';

  std::string s_uut = uut.Get();
  EXPECT_STREQ(s_ref.c_str(), s_uut.c_str());
}

TEST(gpcc_string_StringComposer, Append_longlong_showBaseIsSticky)
{
  long long v1 = 184;
  long long v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_longlong_baseHex_alignRightPadZero)
{
  long long v1 = 184;
  long long v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_longlong_baseHex_showBase_alignRightPadZero)
{
  long long v1 = 184;
  long long v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_longlong_uppercaseIsSticky)
{
  long long v1 = 184;
  long long v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_longlong_baseOctal)
{
  long long v = 467;

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

TEST(gpcc_string_StringComposer, Append_longlong_baseOctalNegativeValue)
{
  long long v = -184;

  // test UUT against reference, since result depends on CPU architecture (32/64 bit)
  std::ostringstream reference;
  reference << std::oct << v << ';';
  std::string s_ref(reference.str());

  StringComposer uut;
  uut << StringComposer::BaseOct << v << ';';

  std::string s_uut = uut.Get();
  EXPECT_STREQ(s_ref.c_str(), s_uut.c_str());
}

TEST(gpcc_string_StringComposer, Append_longlong_MinMax)
{
  long long min = std::numeric_limits<long long>::min();
  long long max = std::numeric_limits<long long>::max();

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

TEST(gpcc_string_StringComposer, Append_longlong_ExceedFieldWidth)
{
  long long v = 1000;

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
