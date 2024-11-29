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

TEST(gpcc_string_StringComposer, Append_ushort_defaults)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_defaultAlignmentRight)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_alignLeft_widthIsNotSticky)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_alignLeftIsSticky)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_alignRight_widthIsNotSticky)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_alignRightIsSticky)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_alignRightPadZeroIsSticky)
{
  unsigned short v1 = 37;
  unsigned short v2 = 133;

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

TEST(gpcc_string_StringComposer, Append_ushort_showPosHasNoEffect)
{
  unsigned short v1 = 37;

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

TEST(gpcc_string_StringComposer, Append_ushort_baseHexIsSticky)
{
  unsigned short v1 = 184;
  unsigned short v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_ushort_showBaseIsSticky)
{
  unsigned short v1 = 184;
  unsigned short v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_ushort_baseHex_showBase_Width4_Zero)
{
  unsigned short z = 0;

  char const * const pExpectedStrRef = "0   ;   0;0000;";
  char const * const pExpectedStrUUT = "0   ;   0;0x00;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << std::showbase
            << std::left << std::setw(4) << z << ';'
            << std::right << std::setw(4) << z << ';'
            << std::right << std::setfill('0') << std::setw(4) << z << ';';

  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStrRef);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << StringComposer::ShowBase
      << StringComposer::AlignLeft << StringComposer::Width(4) << z << ';'
      << StringComposer::AlignRight << StringComposer::Width(4) << z << ';'
      << StringComposer::AlignRightPadZero << StringComposer::Width(4) << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStrUUT);
}

TEST(gpcc_string_StringComposer, Append_ushort_baseHex_showBase_Width0_Zero)
{
  unsigned short z = 0;

  char const * const pExpectedStrRef = "0;0;0;";
  char const * const pExpectedStrUUT = "0;0;0x0;";

  // test reference
  std::ostringstream reference;
  reference << std::hex << std::showbase
            << std::left <<  z << ';'
            << std::right << z << ';'
            << std::right << std::setfill('0') << z << ';';

  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStrRef);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseHex << StringComposer::ShowBase
      << StringComposer::AlignLeft << z << ';'
      << StringComposer::AlignRight << z << ';'
      << StringComposer::AlignRightPadZero << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStrUUT);
}


TEST(gpcc_string_StringComposer, Append_ushort_baseHex_alignRightPadZero)
{
  unsigned short v1 = 184;
  unsigned short v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_ushort_baseHex_showBase_alignRightPadZero)
{
  unsigned short v1 = 184;
  unsigned short v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_ushort_uppercaseIsSticky)
{
  unsigned short v1 = 184;
  unsigned short v2 = 44;

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

TEST(gpcc_string_StringComposer, Append_ushort_baseOctal)
{
  unsigned short v = 467;

  char const * const pExpectedStr = "   723;  0723;";

  // test reference
  std::ostringstream reference;
  reference << std::oct << std::setw(6) << v << ';'
            << std::showbase << std::setw(6) << v << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseOct << StringComposer::Width(6) << v << ';'
      << StringComposer::ShowBase << StringComposer::Width(6) << v << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_ushort_baseOctal_showBase_Width4_Zero)
{
  unsigned short z = 0;

  char const * const pExpectedStrRef = "0   ;   0;0000;";
  char const * const pExpectedStrUUT = "0   ;   0;0000;";

  // test reference
  std::ostringstream reference;
  reference << std::oct << std::showbase
            << std::left << std::setw(4) << z << ';'
            << std::right << std::setw(4) << z << ';'
            << std::right << std::setfill('0') << std::setw(4) << z << ';';

  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStrRef);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseOct << StringComposer::ShowBase
      << StringComposer::AlignLeft << StringComposer::Width(4) << z << ';'
      << StringComposer::AlignRight << StringComposer::Width(4) << z << ';'
      << StringComposer::AlignRightPadZero << StringComposer::Width(4) << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStrUUT);
}

TEST(gpcc_string_StringComposer, Append_ushort_baseOctal_showBase_Width0_Zero)
{
  unsigned short z = 0;

  char const * const pExpectedStrRef = "0;0;0;";
  char const * const pExpectedStrUUT = "0;0;00;";

  // test reference
  std::ostringstream reference;
  reference << std::oct << std::showbase
            << std::left <<  z << ';'
            << std::right << z << ';'
            << std::right << std::setfill('0') << z << ';';

  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStrRef);

  // test UUT
  StringComposer uut;
  uut << StringComposer::BaseOct << StringComposer::ShowBase
      << StringComposer::AlignLeft << z << ';'
      << StringComposer::AlignRight << z << ';'
      << StringComposer::AlignRightPadZero << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStrUUT);
}


TEST(gpcc_string_StringComposer, Append_ushort_MinMax)
{
  unsigned short min = std::numeric_limits<unsigned short>::min();
  unsigned short max = std::numeric_limits<unsigned short>::max();

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

TEST(gpcc_string_StringComposer, Append_ushort_ExceedFieldWidth)
{
  unsigned short v = 1000;

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
