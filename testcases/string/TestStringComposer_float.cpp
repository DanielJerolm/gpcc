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
#include <string>

namespace gpcc_tests {
namespace string {

using gpcc::string::StringComposer;

TEST(gpcc_string_StringComposer, Append_float_defaults)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "37.5;-133.23;";

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

TEST(gpcc_string_StringComposer, Append_float_defaultAlignmentRight)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "    37.5; -133.23;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(8) << v1 << ';' << std::setw(8) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(8) << v1 << ';' << StringComposer::Width(8) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_alignLeft_widthIsNotSticky)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "37.5    ;-133.23;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(8) << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(8) << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_alignLeftIsSticky)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "37.5    ;-133.23 ;";

  // test reference
  std::ostringstream reference;
  reference << std::left << std::setw(8) << v1 << ';' << std::setw(8) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignLeft << StringComposer::Width(8) << v1 << ';' << StringComposer::Width(8) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_alignRight_widthIsNotSticky)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "    37.5;-133.23;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(8) << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(8) << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_alignRightIsSticky)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "    37.5; -133.23;";

  // test reference
  std::ostringstream reference;
  reference << std::right << std::setw(8) << v1 << ';' << std::setw(8) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AlignRight << StringComposer::Width(8) << v1 << ';' << StringComposer::Width(8) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_alignRightPadZeroIsSticky)
{
  float v1 = 37.5;
  float v2 = 133.23;
  float v3 = -133.23;

  char const * const pExpectedStr = "000037.5;+0133.23;-0133.23;";

  // The reference (std::ostringstream) is not tested. It pads in front of the sign.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::Width(8) << v1 << ';'
      << StringComposer::ShowPos           << StringComposer::Width(8) << v2 << ';'
                                           << StringComposer::Width(8) << v3 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_exceedFieldWidth)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "37.5;-133.23;";

  // test reference
  std::ostringstream reference;
  reference << std::setw(2) << v1 << ';' << std::setw(2) << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::Width(2) << v1 << ';' << StringComposer::Width(2) << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_FormatSticky)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "0x1.2cp+5;-0x1.0a75c2p+7;";

  // test reference
  std::ostringstream reference;
  reference << std::hexfloat << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::HexFloat << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_HexFloat_AlignRightPadWhitespaces)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "   0x1.2cp+5;-0x1.0a75c2p+7;";

  // test reference
  std::ostringstream reference;
  reference << std::hexfloat << std::setw(12) << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::HexFloat << StringComposer::Width(12) << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_HexFloat_AlignRightPadZero)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "0x00000001.2cp+5;-0x001.0a75c2p+7;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::HexFloat
      << StringComposer::Width(16) << v1 << ';' << StringComposer::Width(16) << v2 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_HexFloat_UppercaseIsSticky)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "0X1.2CP+5;-0X1.0A75C2P+7;";

  // test reference
  std::ostringstream reference;
  reference << std::hexfloat << std::uppercase << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::HexFloat << StringComposer::Uppercase << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_Scientific_UppercaseLowercase)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "3.750000E+01;-1.332300e+02;";

  // test reference
  std::ostringstream reference;
  reference << std::scientific << std::uppercase << v1 << ';' << std::nouppercase << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::ScientificFloat << StringComposer::Uppercase << v1 << ';' << StringComposer::NoUppercase << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_Fixed_DefaultPrec)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "37.500000;-133.229996;";

  // test reference
  std::ostringstream reference;
  reference << std::fixed << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::FixedFloat << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_Fixed_DefaultPrec_ShowPos)
{
  float v1 = 37.5;
  float v2 = -133.23;

  char const * const pExpectedStr = "+37.500000;-133.229996;";

  // test reference
  std::ostringstream reference;
  reference << std::fixed << std::showpos << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::FixedFloat << StringComposer::ShowPos << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_Fixed_Prec2Sticky)
{
  float v1 = 537.558;
  float v2 = -133.2;
  float v3 = 5;

  char const * const pExpectedStr = "537.56;-133.20;5.00;";

  // test reference
  std::ostringstream reference;
  reference << std::fixed << std::setprecision(2) << v1 << ';' << v2 << ';' << v3 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::FixedFloat << StringComposer::Precision(2) << v1 << ';' << v2 << ';' << v3 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_AutoFloat_NoShowpoint)
{
  float v1 = 37;
  float v2 = -133;

  char const * const pExpectedStr = "37;-133;";

  // test reference
  std::ostringstream reference;
  reference << std::defaultfloat << std::setprecision(6) << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AutoFloat << StringComposer::Precision(6) << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_AutoFloat_ShowpointIsSticky)
{
  float v1 = 37;
  float v2 = -133;

  char const * const pExpectedStr = "37.0000;-133.000;";

  // test reference
  std::ostringstream reference;
  reference << std::defaultfloat << std::setprecision(6) << std::showpoint << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AutoFloat << StringComposer::Precision(6) << StringComposer::ShowPoint << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_float_AutoFloat_ShowPoint_ShowPosIsSticky)
{
  float v1 = 37;
  float v2 = 133;

  char const * const pExpectedStr = "+37.0000;+133.000;";

  // test reference
  std::ostringstream reference;
  reference << std::defaultfloat << std::setprecision(6) << std::showpoint << std::showpos << v1 << ';' << v2 << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::AutoFloat << StringComposer::Precision(6) << StringComposer::ShowPoint << StringComposer::ShowPos << v1 << ';' << v2 << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

} // namespace string
} // namespace gpcc_tests
