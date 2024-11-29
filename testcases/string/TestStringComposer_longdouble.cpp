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

TEST(gpcc_string_StringComposer, Append_longdouble_defaults)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_defaultAlignmentRight)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_alignLeft_widthIsNotSticky)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_alignLeftIsSticky)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_alignRight_widthIsNotSticky)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_alignRightIsSticky)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_alignRightPadZeroIsSticky)
{
  long double v1 = 37.5;
  long double v2 = 133.23;
  long double v3 = -133.23;

  char const * const pExpectedStr = "000037.5;+0133.23;-0133.23;";

  // The reference (std::ostringstream) is not tested. It pads in front of the sign.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::Width(8) << v1 << ';'
      << StringComposer::ShowPos           << StringComposer::Width(8) << v2 << ';'
                                           << StringComposer::Width(8) << v3 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_exceedFieldWidth)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_FormatSticky)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "0x9.6p+2;-0x8.53ae147ae1478p+4;";

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

TEST(gpcc_string_StringComposer, Append_longdouble_AlignRightPadWhitespaces)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "    0x9.6p+2;-0x8.53ae147ae1478p+4;";

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

TEST(gpcc_string_StringComposer, Append_longdouble_HexFloat_AlignRightPadWhitespaces_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "      0x0p+0;0x0p+0;";

  // test reference
  std::ostringstream reference;
  reference << std::hexfloat << std::setw(12) << z << ';' << z << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::HexFloat << StringComposer::Width(12) << z << ';' << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_HexFloat_AlignLeftPadWhitespaces_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "0x0p+0      ;0x0p+0;";

  // test reference
  std::ostringstream reference;
  reference << std::hexfloat << std::left << std::setw(12) << z << ';' << z << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::HexFloat << StringComposer::AlignLeft << StringComposer::Width(12) << z << ';' << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_HexFloat_AlignRightPadZero)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "0x00000000000000009.6p+2;-0x0008.53ae147ae1478p+4;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::HexFloat
      << StringComposer::Width(24) << v1 << ';' << StringComposer::Width(24) << v2 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_HexFloat_AlignRightPadZero_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "0x00000000000p+0;0x0p+0;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::HexFloat
      << StringComposer::Width(16) << z << ';'  << z << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_HexFloat_UppercaseIsSticky)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "0X9.6P+2;-0X8.53AE147AE1478P+4;";

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

TEST(gpcc_string_StringComposer, Append_longdouble_Scientific_UppercaseLowercase)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

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

TEST(gpcc_string_StringComposer, Append_longdouble_Scientific_AlignRightPadWhitespaces_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "    0.000000e+00;0.000000e+00;";

  // test reference
  std::ostringstream reference;
  reference << std::scientific << std::setw(16) << z << ';' << z << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::ScientificFloat << StringComposer::Width(16) << z << ';' << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Scientific_AlignLeftPadWhitespaces_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "0.000000e+00    ;0.000000e+00;";

  // test reference
  std::ostringstream reference;
  reference << std::scientific << std::left << std::setw(16) << z << ';' << z << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::ScientificFloat << StringComposer::AlignLeft << StringComposer::Width(16) << z << ';' << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Scientific_AlignRightPadZero)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "00003.750000e+01;-0001.332300e+02;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::ScientificFloat
      << StringComposer::Width(16) << v1 << ';' << StringComposer::Width(16) << v2 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Scientific_AlignRightPadZero_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "00000.000000e+00;0.000000e+00;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::ScientificFloat
      << StringComposer::Width(16) << z << ';'  << z << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_DefaultPrec)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "37.500000;-133.230000;";

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

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_DefaultPrec_ShowPos)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "+37.500000;-133.230000;";

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

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_PrecisionIsSticky)
{
  long double v1 = 37.558;
  long double v2 = -133.2;
  long double v3 = 5;

  char const * const pExpectedStr = "37.56;-133.20;5.00;";

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

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_AlignRightPadWhitespaces_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "    0.000000;0.000000;";

  // test reference
  std::ostringstream reference;
  reference << std::fixed << std::setw(12) << z << ';' << z << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::FixedFloat << StringComposer::Width(12) << z << ';' << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_AlignLeftPadWhitespaces_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "0.000000    ;0.000000;";

  // test reference
  std::ostringstream reference;
  reference << std::fixed << std::left << std::setw(12) << z << ';' << z << ';';
  std::string s(reference.str());
  EXPECT_STREQ(s.c_str(), pExpectedStr);

  // test UUT
  StringComposer uut;
  uut << StringComposer::FixedFloat << StringComposer::AlignLeft << StringComposer::Width(12) << z << ';' << z << ';';

  s = uut.Get();
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_AlignRightPadZero)
{
  long double v1 = 37.5;
  long double v2 = -133.23;

  char const * const pExpectedStr = "00037.500000;-0133.230000;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::FixedFloat
      << StringComposer::Width(12) << v1 << ';' << StringComposer::Width(12) << v2 << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_Fixed_AlignRightPadZero_Zero)
{
  long double z = 0;

  char const * const pExpectedStr = "00000.000000;0.000000;";

  // The reference (std::ostringstream) is not tested. It pads in front of the base.

  StringComposer uut;
  uut << StringComposer::AlignRightPadZero << StringComposer::FixedFloat
      << StringComposer::Width(12) << z << ';' << z << ';';

  std::string s(uut.Get());
  EXPECT_STREQ(s.c_str(), pExpectedStr);
}

TEST(gpcc_string_StringComposer, Append_longdouble_AutoFloat_NoShowpoint)
{
  long double v1 = 37;
  long double v2 = -133;

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

TEST(gpcc_string_StringComposer, Append_longdouble_AutoFloat_ShowpointIsSticky)
{
  long double v1 = 37;
  long double v2 = -133;

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

TEST(gpcc_string_StringComposer, Append_longdouble_AutoFloat_ShowPoint_ShowPosIsSticky)
{
  long double v1 = 37;
  long double v2 = 133;

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
