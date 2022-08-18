/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/string/levenshtein_distance.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace string {

using namespace testing;
using namespace gpcc::string;

TEST(gpcc_string_levenshtein_distance_Tests, C_Strings_SpecialCases)
{
  ASSERT_EQ(3U, LevenshteinDistance("ABC", "", true));
  ASSERT_EQ(3U, LevenshteinDistance("", "ABC", true));
  ASSERT_EQ(0U, LevenshteinDistance("", "", true));
  ASSERT_EQ(0U, LevenshteinDistance("ABC", "ABC", true));

  ASSERT_EQ(3U, LevenshteinDistance("ABC", "", false));
  ASSERT_EQ(3U, LevenshteinDistance("", "ABC", false));
  ASSERT_EQ(0U, LevenshteinDistance("", "", false));
  ASSERT_EQ(0U, LevenshteinDistance("ABC", "ABC", false));
  ASSERT_EQ(0U, LevenshteinDistance("ABC", "AbC", false));
}
TEST(gpcc_string_levenshtein_distance_Tests, C_Strings_CaseSensitive)
{
  ASSERT_EQ(1U, LevenshteinDistance("Test", "Text", true));
  ASSERT_EQ(2U, LevenshteinDistance("Test", "text", true));
  ASSERT_EQ(7U, LevenshteinDistance("Test", "Textwriter", true));
  ASSERT_EQ(3U, LevenshteinDistance("Tester", "Text", true));
}
TEST(gpcc_string_levenshtein_distance_Tests, C_Strings_CaseInSensitive)
{
  ASSERT_EQ(1U, LevenshteinDistance("Test", "Text", false));
  ASSERT_EQ(1U, LevenshteinDistance("Test", "text", false));
  ASSERT_EQ(7U, LevenshteinDistance("Test", "Textwriter", false));
  ASSERT_EQ(3U, LevenshteinDistance("Tester", "TexT", false));
}

TEST(gpcc_string_levenshtein_distance_Tests, stdString_C_Strings_SpecialCases)
{
  std::string str_ABC("ABC");
  std::string str_empty;

  ASSERT_EQ(3U, LevenshteinDistance(str_ABC, "", true));
  ASSERT_EQ(3U, LevenshteinDistance(str_empty, "ABC", true));
  ASSERT_EQ(0U, LevenshteinDistance(str_empty, "", true));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, "ABC", true));

  ASSERT_EQ(3U, LevenshteinDistance(str_ABC, "", false));
  ASSERT_EQ(3U, LevenshteinDistance(str_empty, "ABC", false));
  ASSERT_EQ(0U, LevenshteinDistance(str_empty, "", false));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, "ABC", false));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, "AbC", false));
}
TEST(gpcc_string_levenshtein_distance_Tests, stdString_C_Strings_CaseSensitive)
{
  std::string str_Test("Test");
  std::string str_Tester("Tester");
  ASSERT_EQ(1U, LevenshteinDistance(str_Test, "Text", true));
  ASSERT_EQ(2U, LevenshteinDistance(str_Test, "text", true));
  ASSERT_EQ(7U, LevenshteinDistance(str_Test, "Textwriter", true));
  ASSERT_EQ(3U, LevenshteinDistance(str_Tester, "Text", true));
}
TEST(gpcc_string_levenshtein_distance_Tests, stdString_C_Strings_CaseInSensitive)
{
  std::string str_Test("Test");
  std::string str_Tester("Tester");
  ASSERT_EQ(1U, LevenshteinDistance(str_Test, "Text", false));
  ASSERT_EQ(1U, LevenshteinDistance(str_Test, "text", false));
  ASSERT_EQ(7U, LevenshteinDistance(str_Test, "Textwriter", false));
  ASSERT_EQ(3U, LevenshteinDistance(str_Tester, "TexT", false));
}

TEST(gpcc_string_levenshtein_distance_Tests, stdString_stdString_SpecialCases)
{
  std::string str_ABC("ABC");
  std::string str_ABC2("ABC");
  std::string str_AbC("AbC");
  std::string str_empty;
  std::string str_empty2;

  ASSERT_EQ(3U, LevenshteinDistance(str_ABC, str_empty, true));
  ASSERT_EQ(3U, LevenshteinDistance(str_empty, str_ABC, true));
  ASSERT_EQ(0U, LevenshteinDistance(str_empty, str_empty, true));
  ASSERT_EQ(0U, LevenshteinDistance(str_empty, str_empty2, true));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, str_ABC, true));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, str_ABC2, true));

  ASSERT_EQ(3U, LevenshteinDistance(str_ABC, str_empty, false));
  ASSERT_EQ(3U, LevenshteinDistance(str_empty, str_ABC, false));
  ASSERT_EQ(0U, LevenshteinDistance(str_empty, str_empty, false));
  ASSERT_EQ(0U, LevenshteinDistance(str_empty, str_empty2, false));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, str_ABC, false));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, str_ABC2, false));
  ASSERT_EQ(0U, LevenshteinDistance(str_ABC, str_AbC, false));
}
TEST(gpcc_string_levenshtein_distance_Tests, stdString_stdString_CaseSensitive)
{
  std::string str_Test("Test");
  std::string str_Tester("Tester");
  std::string str_Text("Text");
  std::string str_text("text");
  std::string str_Textwriter("Textwriter");
  ASSERT_EQ(1U, LevenshteinDistance(str_Test, str_Text, true));
  ASSERT_EQ(2U, LevenshteinDistance(str_Test, str_text, true));
  ASSERT_EQ(7U, LevenshteinDistance(str_Test, str_Textwriter, true));
  ASSERT_EQ(3U, LevenshteinDistance(str_Tester, str_Text, true));
}
TEST(gpcc_string_levenshtein_distance_Tests, stdString_stdString_CaseInSensitive)
{
  std::string str_Test("Test");
  std::string str_Tester("Tester");
  std::string str_Text("Text");
  std::string str_text("text");
  std::string str_Textwriter("Textwriter");
  std::string str_TexT("TexT");

  ASSERT_EQ(1U, LevenshteinDistance(str_Test, str_Text, false));
  ASSERT_EQ(1U, LevenshteinDistance(str_Test, str_text, false));
  ASSERT_EQ(7U, LevenshteinDistance(str_Test, str_Textwriter, false));
  ASSERT_EQ(3U, LevenshteinDistance(str_Tester, str_TexT, false));
}

} // namespace string
} // namespace gpcc_tests
