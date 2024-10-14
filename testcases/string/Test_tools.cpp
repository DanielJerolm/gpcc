/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#include <gpcc/string/tools.hpp>
#include <gtest/gtest.h>

namespace {

void ThrowFunc1(void)
{
  throw std::runtime_error("ThrowFunc1");
}

void ThrowFunc2(void)
{
  try
  {
    ThrowFunc1();
  }
  catch (std::exception const & e)
  {
    std::throw_with_nested(std::runtime_error("ThrowFunc2"));
  }
}

void ThrowFunc3(void)
{
  throw 5;
}

void ThrowFunc4(void)
{
  try
  {
    ThrowFunc3();
  }
  catch (...)
  {
    std::throw_with_nested(std::runtime_error("ThrowFunc4"));
  }
}

} // anonymous namespace

namespace gpcc_tests {
namespace string {

using namespace testing;
using namespace gpcc::string;

// String manipulation --------------------------------------------------------
TEST(gpcc_string_tools_Tests, Trim)
{
  std::string result;

  result = Trim("");
  ASSERT_EQ(0U, result.length());

  result = Trim(" ");
  ASSERT_EQ(0U, result.length());

  result = Trim("  ");
  ASSERT_EQ(0U, result.length());

  result = Trim("Text");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim(" Text");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim("  Text");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim("Text ");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim("Text  ");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim(" Text ");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim("  Text  ");
  ASSERT_EQ(4U, result.length());
  ASSERT_TRUE(result == "Text");

  result = Trim("Te xt");
  ASSERT_EQ(5U, result.length());
  ASSERT_TRUE(result == "Te xt");

  result = Trim("  Te xt  ");
  ASSERT_EQ(5U, result.length());
  ASSERT_TRUE(result == "Te xt");

  result = Trim("A");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim(" A");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim("  A");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim("A ");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim("A  ");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim(" A ");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim("  A  ");
  ASSERT_EQ(1U, result.length());
  ASSERT_TRUE(result == "A");

  result = Trim("AB");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");

  result = Trim(" AB");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");

  result = Trim("  AB");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");

  result = Trim("AB ");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");

  result = Trim("AB  ");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");

  result = Trim(" AB ");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");

  result = Trim("  AB  ");
  ASSERT_EQ(2U, result.length());
  ASSERT_TRUE(result == "AB");
}

TEST(gpcc_string_tools_Tests, Trim_char)
{
  std::string result;

  result = Trim("", '!');
  ASSERT_EQ(result, "");

  result = Trim("!", '!');
  ASSERT_EQ(result, "");

  result = Trim("!!", '!');
  ASSERT_EQ(result, "");

  result = Trim("!!!", '!');
  ASSERT_EQ(result, "");

  result = Trim("!Test!", '!');
  ASSERT_EQ(result, "Test");

  result = Trim("!Test! Test!", '!');
  ASSERT_EQ(result, "Test! Test");
}

TEST(gpcc_string_tools_Tests, Split1)
{
  std::vector<std::string> v;

  // 0 empty parts
  v = Split("This is a test", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test", ' ', false);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  // 1 empty part in middle
  v = Split("This is  a test", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is  a test", ' ', false);
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // 2 empty parts in middle
  v = Split("This is   a test", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is   a test", ' ', false);
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "");
  ASSERT_TRUE(v[3] == "");
  ASSERT_TRUE(v[4] == "a");
  ASSERT_TRUE(v[5] == "test");

  // 1 empty part at head
  v = Split(" This is a test", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split(" This is a test", ' ', false);
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "This");
  ASSERT_TRUE(v[2] == "is");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // 2 empty parts at head
  v = Split("  This is a test", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("  This is a test", ' ', false);
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");
  ASSERT_TRUE(v[2] == "This");
  ASSERT_TRUE(v[3] == "is");
  ASSERT_TRUE(v[4] == "a");
  ASSERT_TRUE(v[5] == "test");

  // 3 empty parts at head
  v = Split("   This is a test", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("   This is a test", ' ', false);
  ASSERT_EQ(7U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");
  ASSERT_TRUE(v[2] == "");
  ASSERT_TRUE(v[3] == "This");
  ASSERT_TRUE(v[4] == "is");
  ASSERT_TRUE(v[5] == "a");
  ASSERT_TRUE(v[6] == "test");

  // 1 empty part at tail
  v = Split("This is a test ", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test ", ' ', false);
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "");

  // 2 empty parts at tail
  v = Split("This is a test  ", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test  ", ' ', false);
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "");
  ASSERT_TRUE(v[5] == "");

  // 3 empty parts at tail
  v = Split("This is a test   ", ' ', true);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test   ", ' ', false);
  ASSERT_EQ(7U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "");
  ASSERT_TRUE(v[5] == "");
  ASSERT_TRUE(v[6] == "");

  // empty string
  v = Split("", ' ', true);
  ASSERT_EQ(0U, v.size());

  v = Split("", ' ', false);
  ASSERT_EQ(0U, v.size());

  // only separators (1)
  v = Split(" ", ' ', true);
  ASSERT_EQ(0U, v.size());

  v = Split(" ", ' ', false);
  ASSERT_EQ(2U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");

  // only separators (2)
  v = Split("  ", ' ', true);
  ASSERT_EQ(0U, v.size());

  v = Split("  ", ' ', false);
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");
  ASSERT_TRUE(v[2] == "");

  // no separator
  v = Split("Test", ' ', true);
  ASSERT_EQ(1U, v.size());
  ASSERT_TRUE(v[0] == "Test");

  v = Split("Test", ' ', false);
  ASSERT_EQ(1U, v.size());
  ASSERT_TRUE(v[0] == "Test");
}
TEST(gpcc_string_tools_Tests, Split2a)
{
  // ---------------------------------------------
  // Same test patterns as in test case "Split1".
  // No quotation mark characters appear in input.
  // ---------------------------------------------

  std::vector<std::string> v;

  char const qm = '"';

  // 0 empty parts
  v = Split("This is a test", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test", ' ', false, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  // 1 empty part in middle
  v = Split("This is  a test", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is  a test", ' ', false, qm);
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // 2 empty parts in middle
  v = Split("This is   a test", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is   a test", ' ', false, qm);
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "");
  ASSERT_TRUE(v[3] == "");
  ASSERT_TRUE(v[4] == "a");
  ASSERT_TRUE(v[5] == "test");

  // 1 empty part at head
  v = Split(" This is a test", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split(" This is a test", ' ', false, qm);
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "This");
  ASSERT_TRUE(v[2] == "is");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // 2 empty parts at head
  v = Split("  This is a test", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("  This is a test", ' ', false, qm);
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");
  ASSERT_TRUE(v[2] == "This");
  ASSERT_TRUE(v[3] == "is");
  ASSERT_TRUE(v[4] == "a");
  ASSERT_TRUE(v[5] == "test");

  // 3 empty parts at head
  v = Split("   This is a test", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("   This is a test", ' ', false, qm);
  ASSERT_EQ(7U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");
  ASSERT_TRUE(v[2] == "");
  ASSERT_TRUE(v[3] == "This");
  ASSERT_TRUE(v[4] == "is");
  ASSERT_TRUE(v[5] == "a");
  ASSERT_TRUE(v[6] == "test");

  // 1 empty part at tail
  v = Split("This is a test ", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test ", ' ', false, qm);
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "");

  // 2 empty parts at tail
  v = Split("This is a test  ", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test  ", ' ', false, qm);
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "");
  ASSERT_TRUE(v[5] == "");

  // 3 empty parts at tail
  v = Split("This is a test   ", ' ', true, qm);
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  v = Split("This is a test   ", ' ', false, qm);
  ASSERT_EQ(7U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "");
  ASSERT_TRUE(v[5] == "");
  ASSERT_TRUE(v[6] == "");

  // empty string
  v = Split("", ' ', true, qm);
  ASSERT_EQ(0U, v.size());

  v = Split("", ' ', false, qm);
  ASSERT_EQ(0U, v.size());

  // only separators (1)
  v = Split(" ", ' ', true, qm);
  ASSERT_EQ(0U, v.size());

  v = Split(" ", ' ', false, qm);
  ASSERT_EQ(2U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");

  // only separators (2)
  v = Split("  ", ' ', true, qm);
  ASSERT_EQ(0U, v.size());

  v = Split("  ", ' ', false, qm);
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "");
  ASSERT_TRUE(v[1] == "");
  ASSERT_TRUE(v[2] == "");

  // no separator
  v = Split("Test", ' ', true, qm);
  ASSERT_EQ(1U, v.size());
  ASSERT_TRUE(v[0] == "Test");

  v = Split("Test", ' ', false, qm);
  ASSERT_EQ(1U, v.size());
  ASSERT_TRUE(v[0] == "Test");
}

TEST(gpcc_string_tools_Tests, Split2b)
{
  std::vector<std::string> v;

  char const qm = '\'';

  // basic test (qm has neighbouring whitespaces outside surrounded range)
  ASSERT_NO_THROW(v = Split("This 'is a' test", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "'is a'");
  ASSERT_TRUE(v[2] == "test");

  // white spaces on both sides of each qm
  ASSERT_NO_THROW(v = Split("This ' is a ' test", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "' is a '");
  ASSERT_TRUE(v[2] == "test");

  // qm not neighbouring any white spaces
  ASSERT_NO_THROW(v = Split("This >'is a'< test", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == ">'is a'<");
  ASSERT_TRUE(v[2] == "test");

  // each qm has a neighbouring white space inside surrounded range
  ASSERT_NO_THROW(v = Split("This >' is a '< test", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == ">' is a '<");
  ASSERT_TRUE(v[2] == "test");

  // range at the beginning
  ASSERT_NO_THROW(v = Split("'This is' a test", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "'This is'");
  ASSERT_TRUE(v[1] == "a");
  ASSERT_TRUE(v[2] == "test");

  // range at the end
  ASSERT_NO_THROW(v = Split("This is 'a test'", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "'a test'");

  // range surrounded by qm empty (at the beginning, no whitspace)
  ASSERT_NO_THROW(v = Split("''This is a test", ' ', true, qm));
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "''This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");

  // range surrounded by qm empty (at the beginning, with whitspace)
  ASSERT_NO_THROW(v = Split("'' This is a test", ' ', true, qm));
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "''");
  ASSERT_TRUE(v[1] == "This");
  ASSERT_TRUE(v[2] == "is");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // range surrounded by qm empty (in the middle, no whitespace)
  ASSERT_NO_THROW(v = Split("This''is a test", ' ', true, qm));
  ASSERT_EQ(3U, v.size());
  ASSERT_TRUE(v[0] == "This''is");
  ASSERT_TRUE(v[1] == "a");
  ASSERT_TRUE(v[2] == "test");

  // range surrounded by qm empty (in the middle, with whitespace)
  ASSERT_NO_THROW(v = Split("This '' is a test", ' ', true, qm));
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "''");
  ASSERT_TRUE(v[2] == "is");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // range surrounded by qm empty (at the end, no whitespace)
  ASSERT_NO_THROW(v = Split("This is a test''", ' ', true, qm));
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test''");

  // range surrounded by qm empty (at the end, with whitespace)
  ASSERT_NO_THROW(v = Split("This is a test ''", ' ', true, qm));
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "is");
  ASSERT_TRUE(v[2] == "a");
  ASSERT_TRUE(v[3] == "test");
  ASSERT_TRUE(v[4] == "''");

  // two ranges surrounded by qm empty, ranges separated by whitspace
  ASSERT_NO_THROW(v = Split("This '' '' is a test", ' ', true, qm));
  ASSERT_EQ(6U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "''");
  ASSERT_TRUE(v[2] == "''");
  ASSERT_TRUE(v[3] == "is");
  ASSERT_TRUE(v[4] == "a");
  ASSERT_TRUE(v[5] == "test");

  // two ranges surrounded by qm empty, ranges not separated by whitespace
  ASSERT_NO_THROW(v = Split("This '''' is a test", ' ', true, qm));
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "''''");
  ASSERT_TRUE(v[2] == "is");
  ASSERT_TRUE(v[3] == "a");
  ASSERT_TRUE(v[4] == "test");

  // two neighbouring not empty ranges
  ASSERT_NO_THROW(v = Split("This 'is a'' much more' sophisticated test", ' ', true, qm));
  ASSERT_EQ(4U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "'is a'' much more'");
  ASSERT_TRUE(v[2] == "sophisticated");
  ASSERT_TRUE(v[3] == "test");

  // two ranges surrounded by qm
  ASSERT_NO_THROW(v = Split("This 'is a' much 'more sophisticated' test", ' ', true, qm));
  ASSERT_EQ(5U, v.size());
  ASSERT_TRUE(v[0] == "This");
  ASSERT_TRUE(v[1] == "'is a'");
  ASSERT_TRUE(v[2] == "much");
  ASSERT_TRUE(v[3] == "'more sophisticated'");
  ASSERT_TRUE(v[4] == "test");

  // ranges surround the whole string
  ASSERT_NO_THROW(v = Split("'This is a test'", ' ', true, qm));
  ASSERT_EQ(1U, v.size());
  ASSERT_TRUE(v[0] == "'This is a test'");

  // two qm characters only
  ASSERT_NO_THROW(v = Split("''", ' ', true, qm));
  ASSERT_EQ(1U, v.size());
  ASSERT_TRUE(v[0] == "''");

  // qm missing
  ASSERT_THROW(v = Split("This 'is a much more sophisticated test", ' ', true, qm), std::invalid_argument);
  ASSERT_THROW(v = Split("This 'is a much 'more sophisticated' test", ' ', true, qm), std::invalid_argument);
  ASSERT_THROW(v = Split("'", ' ', true, qm), std::invalid_argument);
  ASSERT_THROW(v = Split("'''", ' ', true, qm), std::invalid_argument);

  // qm same as separator
  ASSERT_THROW(v = Split("This is a test", ' ', true, ' '), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, ConditionalConcat_ExamplesFromDox)
{
  std::vector<std::string> v;
  std::vector<std::string> expect;

  // Examples for common input:
  v = {"Name:Willy"};
  expect = v;
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name:Willy\" -> \"Name:Willy\" failed";

  v = {"Name:Willy", "Age:5"};
  expect = v;
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name:Willy\", \"Age:5\" -> \"Name:Willy\", \"Age:5\" failed";

  v = {"Name:", "Willy"};
  expect = {"Name:Willy"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name:\", \"Willy\" -> \"Name:Willy\" failed";

  v = {"Name", ":Willy"};
  expect = {"Name:Willy"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name\", \":Willy\" -> \"Name:Willy\" failed";

  v = {"Name", ":", "Willy"};
  expect = {"Name:Willy"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name\", \":\", \"Willy\" -> \"Name:Willy\" failed";

  v = {"Name", ":", "Willy", "Age", ":", "50"};
  expect = {"Name:Willy", "Age:50"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name\", \":\", \"Willy\", \"Age\", \":\", \"50\" -> \"Name:Willy\", \"Age:50\" failed";

  // Examples containing empty strings
  v = {"Name:", "", "Willy", ""};
  expect = {"Name:Willy", ""};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name:\", \"\", \"Willy\", \"\" -> \"Name:Willy\", \"\" failed";

  v = {"Name", "", ":", "", "Willy", ""};
  expect = {"Name:Willy", ""};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name\", \"\", \":\", \"\", \"Willy\", \"\" -> \"Name:Willy\", \"\" failed";

  v = {"Name:", "", "", "Willy", ""};
  expect = {"Name:Willy", ""};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name:\", \"\", \"\", \"Willy\", \"\" -> \"Name:Willy\", \"\" failed";

  // Examples for not-so-common input
  v = {"Name", "::", "Willy"};
  expect = {"Name::Willy"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name\", \"::\", \"Willy\" -> \"Name::Willy\" failed";

  v = {"Name:", ":Willy"};
  expect = {"Name::Willy"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name:\", \":Willy\" -> \"Name::Willy\" failed";

  v = {"Name", ":", "Willy:", "Age:", "50"};
  expect = {"Name:Willy:Age:50"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Name\", \":\", \"Willy:\", \"Age:\", \"50\" -> \"Name:Willy:Age:50\" failed";
}

TEST(gpcc_string_tools_Tests, ConditionalConcat_Other)
{
  std::vector<std::string> v;
  std::vector<std::string> expect;

  // empty vector
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << " empty vector -> empty vector failed";

  // one empty string
  v = {""};
  expect = {""};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"\" -> \"\" failed";

  // single string with ':'
  v = {":"};
  expect = {":"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\":\" -> \":\" failed";

  // two empty string
  v = {"", ""};
  expect = {"", ""};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"\", \"\" -> \"\", \"\" failed";

  // two strings with ':'
  v = {":", ":"};
  expect = {"::"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\":\", \":\" -> \"::\" failed";

  // two empty string with ':' in the middle
  v = {"", ":", ""};
  expect = {":"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"\", \":\", \"\" -> \":\" failed";

  // ':' at the beginning
  v = {":", "", ""};
  expect = {":"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\":\", \"\", \"\" -> \":\" failed";

  // ':' plus text at the beginning
  v = {":Test", "", ""};
  expect = {":Test", "", ""};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\":Test\", \"\", \"\" -> \":Test\", \"\", \"\" failed";

  // ':' at the beginning and text in a subsequent string
  v = {":", "", "Test"};
  expect = {":Test"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\":\", \"\", \"Test\" -> \":Test\" failed";

  // ':' at the end
  v = {"", "", ":"};
  expect = {":"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"\", \"\", \":\" -> \":\" failed";

  // text plus ':' at the end
  v = {"", "", "Test:"};
  expect = {"", "", "Test:"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"\", \"\", \"Test:\" -> \"Test:\" failed";

  // text at front and ':' at the end
  v = {"Test", "", ":"};
  expect = {"Test:"};
  ConditionalConcat(v, ':');
  EXPECT_EQ(v, expect) << "\"Test\", \"\", \":\" -> \"Test:\" failed";
}
#if 0
TEST(gpcc_string_tools_Tests, RealLifeExampleSplitAndConditionalConcat)
{
  std::string input = "Name=\"Willy Pearson\"   Profession= Carpenter Age = 50";

  auto fieldsWithValue = Split(input, ' ', true, '"');
  ConditionalConcat(fieldsWithValue, '=');

  std::vector<std::string> expect = {"Name=\"Willy Pearson\"", "Profession=Carpenter", "Age=50"};
  ASSERT_EQ(fieldsWithValue, expect);

  std::vector<std::pair<std::string,std::string>> pairsOfFieldAndValue;
  for (auto const & fieldWithValue: fieldsWithValue)
  {
    auto fieldAndValue = Split(fieldWithValue, '=', false);
    ASSERT_EQ(fieldWithValue.size(), 2U);


  }
}
#endif
TEST(gpcc_string_tools_Tests, InsertIndention)
{
  std::string s = "Test\nLine1\nLine2";
  InsertIndention(s, 2);
  ASSERT_TRUE(s == "Test\n  Line1\n  Line2");
}
TEST(gpcc_string_tools_Tests, InsertIndention_Zero)
{
  std::string s = "Test\nLine1\nLine2";
  InsertIndention(s, 0);
  ASSERT_TRUE(s == "Test\nLine1\nLine2");
}
TEST(gpcc_string_tools_Tests, InsertIndention_NoNewline)
{
  std::string s = "TestLine1Line2";
  InsertIndention(s, 2);
  ASSERT_TRUE(s == "TestLine1Line2");
}
TEST(gpcc_string_tools_Tests, InsertIndention_TrailingNewLine)
{
  std::string s = "TestLine1Line2\n";
  InsertIndention(s, 2);
  ASSERT_TRUE(s == "TestLine1Line2\n  ");
}

// Tests ----------------------------------------------------------------------
TEST(gpcc_string_tools_Tests, StartsWith)
{
  std::string testStr("Abcdef");

  ASSERT_TRUE(StartsWith(testStr, ""));
  ASSERT_TRUE(StartsWith(testStr, "A"));
  ASSERT_TRUE(StartsWith(testStr, "Abc"));
  ASSERT_TRUE(StartsWith(testStr, "Abcdef"));

  ASSERT_FALSE(StartsWith(testStr, "Abcdefg"));
  ASSERT_FALSE(StartsWith(testStr, "a"));
  ASSERT_FALSE(StartsWith(testStr, "abc"));
  ASSERT_FALSE(StartsWith(testStr, "bc"));
  ASSERT_FALSE(StartsWith(testStr, " "));
  ASSERT_FALSE(StartsWith(testStr, " A"));

  testStr.clear();
  ASSERT_TRUE(StartsWith(testStr, ""));
  ASSERT_FALSE(StartsWith(testStr, "Abc"));
  ASSERT_FALSE(StartsWith(testStr, "abc"));
  ASSERT_FALSE(StartsWith(testStr, " "));
  ASSERT_FALSE(StartsWith(testStr, " Abc"));
}
TEST(gpcc_string_tools_Tests, EndsWith)
{
  std::string testStr("Abcdef");

  ASSERT_TRUE(EndsWith(testStr, ""));
  ASSERT_TRUE(EndsWith(testStr, "f"));
  ASSERT_TRUE(EndsWith(testStr, "ef"));
  ASSERT_TRUE(EndsWith(testStr, "def"));
  ASSERT_TRUE(EndsWith(testStr, "Abcdef"));

  ASSERT_FALSE(EndsWith(testStr, "F"));
  ASSERT_FALSE(EndsWith(testStr, "dEf"));
  ASSERT_FALSE(EndsWith(testStr, "Def"));
  ASSERT_FALSE(EndsWith(testStr, "Abcd"));

  ASSERT_FALSE(EndsWith(testStr, "Abcdefg"));

  testStr.clear();
  ASSERT_TRUE(EndsWith(testStr, ""));
  ASSERT_FALSE(EndsWith(testStr, "A"));
  ASSERT_FALSE(EndsWith(testStr, "dEf"));
  ASSERT_FALSE(EndsWith(testStr, "Def"));
  ASSERT_FALSE(EndsWith(testStr, "Abcd"));
}
TEST(gpcc_string_tools_Tests, CountChar)
{
  size_t n;

  // zero hits
  n = CountChar("zero", 'x');
  EXPECT_EQ(0U, n);

  // case sensitivity
  n = CountChar("zero", 'E');
  EXPECT_EQ(0U, n);

  // one hit
  n = CountChar("One", 'O');
  EXPECT_EQ(1U, n);

  n = CountChar("One", 'n');
  EXPECT_EQ(1U, n);

  n = CountChar("One", 'e');
  EXPECT_EQ(1U, n);

  // zero length string
  n = CountChar("", 'x');
  EXPECT_EQ(0U, n);

  // two hits
  n = CountChar("abbba", 'a');
  EXPECT_EQ(2U, n);

  n = CountChar("babbbab", 'a');
  EXPECT_EQ(2U, n);

  // all hits
  n = CountChar("aaaaa", 'a');
  EXPECT_EQ(5U, n);
}
TEST(gpcc_string_tools_Tests, TestSimplePatternMatch_std_string)
{
  // Note:
  // The std::string-based version used the null-terminated-string overload,
  // so we do a very raw check here only.
  EXPECT_TRUE(TestSimplePatternMatch(std::string("Abc def"), "Abc def", true));
  EXPECT_FALSE(TestSimplePatternMatch(std::string("Abc def"), "Abc de", true));
}
TEST(gpcc_string_tools_Tests, TestSimplePatternMatch_NTS)
{
  // special cases
  EXPECT_TRUE(TestSimplePatternMatch("", "", true));
  EXPECT_TRUE(TestSimplePatternMatch("", "*", true));
  EXPECT_TRUE(TestSimplePatternMatch("A", "*", true));
  EXPECT_TRUE(TestSimplePatternMatch("A", "?", true));

  EXPECT_FALSE(TestSimplePatternMatch("A", "", true));
  EXPECT_FALSE(TestSimplePatternMatch("", "A", true));
  EXPECT_FALSE(TestSimplePatternMatch("", "?", true));
  EXPECT_FALSE(TestSimplePatternMatch("x", "??", true));

  // "non complicated cases"
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc de", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc deF", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc defg", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "abc def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "xAbc def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "AbC def", true));

  // leading *
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "*def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*Def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*dEf", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*deF", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*De", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*defg", true));

  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "*Abc def", true));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "*bc def", true));

  // trailing *
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "abc*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "ABc*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "AbC*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abcd*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "xAbc*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abcx*", true));

  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc def*", true));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc de*", true));

  // mid *
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Ab*ef", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "ab*ef", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "AB*ef", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*Ef", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*eF", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*efg", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "xAb*ef", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abx*ef", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*xef", true));

  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc*def", true));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc *def", true));

  // leading and mid *
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "*c *f", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*C *f", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*c *F", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*c *fg", true));

  // mid and trailing *
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Ab* d*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "ab* d*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "AB* d*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*xd*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab* D*", true));

  // leading and trailing *
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "*c d*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*C d*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*c D*", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "*cxd*", true));

  // single character wildcards (?)
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc?def", true));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "?bc def", true));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Abc de?", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "abc?def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "AbC?def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc?Def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc?deF", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc?defg", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc?de", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "?Abc def", true));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abc def?", true));

  // all intermixed
  EXPECT_TRUE(TestSimplePatternMatch("The brown dog ran across the hill.", "*dog*ran* t??*.", true));
  EXPECT_TRUE(TestSimplePatternMatch("The brown dog ran across the hill.", "*dog*ran* t?? *.", true));
  EXPECT_TRUE(TestSimplePatternMatch("The brown dog ran across the hill.", "*dog *ran* t??*.", true));
  EXPECT_TRUE(TestSimplePatternMatch("The brown dog ran across the hill.", "*dog?*ran* t??*.", true));
  EXPECT_FALSE(TestSimplePatternMatch("The brown dog ran across the hill.", "*dog*ran* t??*!", true));

  // escapes
  EXPECT_TRUE(TestSimplePatternMatch("The * character", "The \\* character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The * character", "The \\*acter", true));

  EXPECT_TRUE(TestSimplePatternMatch("The ? character", "The \\? character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The x character", "The \\? character", true));

  EXPECT_TRUE(TestSimplePatternMatch("The \\ character", "The \\\\ character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The x character", "The \\\\ character", true));

  // escapes after * - wildcard
  EXPECT_TRUE(TestSimplePatternMatch("The * character", "T*\\* character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The *X character", "T*\\* character", true));
  EXPECT_TRUE(TestSimplePatternMatch("The * character", "The *\\* character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The *X character", "The *\\* character", true));

  EXPECT_TRUE(TestSimplePatternMatch("The ? character", "T*\\? character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The ?X character", "T*\\? character", true));

  EXPECT_TRUE(TestSimplePatternMatch("The \\ character", "T*\\\\ character", true));
  EXPECT_FALSE(TestSimplePatternMatch("The \\X character", "T*\\\\ character", true));

  // case insensitivity
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Ab*ef",  false));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "ab*ef",  false));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "AB*ef",  false));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Ab*Ef",  false));
  EXPECT_TRUE(TestSimplePatternMatch("Abc def", "Ab*eF",  false));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*efg", false));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "xAb*ef", false));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Abx*ef", false));
  EXPECT_FALSE(TestSimplePatternMatch("Abc def", "Ab*xef", false));

  // bad escapes
  EXPECT_THROW(TestSimplePatternMatch("Abcdef", "Ab\\", true), std::invalid_argument);
  EXPECT_THROW(TestSimplePatternMatch("Abcdef", "Ab\\xy", true), std::invalid_argument);

  // bad wildcards
  EXPECT_THROW(TestSimplePatternMatch("Abcdef", "Ab**cdef", true), std::invalid_argument);
}
TEST(gpcc_string_tools_Tests, IsPrintableASCII)
{
  for (uint_fast8_t i = 0; i < 0x20; i++)
  {
    ASSERT_FALSE(IsPrintableASCII(static_cast<char>(i)));
  }

  for (uint_fast8_t i = 0x20; i < 0x7F; i++)
  {
    ASSERT_TRUE(IsPrintableASCII(static_cast<char>(i)));
  }

  for (uint_fast16_t i = 0x7F; i < 0x100; i++)
  {
    ASSERT_FALSE(IsPrintableASCII(static_cast<char>(i)));
  }
}
TEST(gpcc_string_tools_Tests, IsPrintableASCIIOnly)
{
  EXPECT_TRUE(IsPrintableASCIIOnly("0"));
  EXPECT_TRUE(IsPrintableASCIIOnly("abc"));
  EXPECT_TRUE(IsPrintableASCIIOnly(""));
  EXPECT_TRUE(IsPrintableASCIIOnly("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
  EXPECT_TRUE(IsPrintableASCIIOnly("0123456789 ^!\"$%&/()=?{[]}+-*/,.;:-_#~<>|@'"));
  EXPECT_FALSE(IsPrintableASCIIOnly("Test\x80"));
}
TEST(gpcc_string_tools_Tests, IsDecimalDigitsOnly)
{
  EXPECT_TRUE(IsDecimalDigitsOnly("0"));
  EXPECT_TRUE(IsDecimalDigitsOnly("1"));
  EXPECT_TRUE(IsDecimalDigitsOnly("23456789"));
  EXPECT_TRUE(IsDecimalDigitsOnly("-5"));
  EXPECT_TRUE(IsDecimalDigitsOnly("-10"));
  EXPECT_TRUE(IsDecimalDigitsOnly("-0"));

  EXPECT_FALSE(IsDecimalDigitsOnly(""));
  EXPECT_FALSE(IsDecimalDigitsOnly(" "));
  EXPECT_FALSE(IsDecimalDigitsOnly(" 3"));
  EXPECT_FALSE(IsDecimalDigitsOnly("3 "));
  EXPECT_FALSE(IsDecimalDigitsOnly("a"));
  EXPECT_FALSE(IsDecimalDigitsOnly("+5"));
}

// Conversion X to string -----------------------------------------------------
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringA_1)
{
  try
  {
    ThrowFunc1();
    ASSERT_TRUE(false);
  }
  catch (std::exception const & e)
  {
    auto text = ExceptionDescriptionToString(e);
    std::string const expected = "1: ThrowFunc1";
    ASSERT_TRUE(text == expected);
  }
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringA_2)
{
  try
  {
    ThrowFunc2();
    ASSERT_TRUE(false);
  }
  catch (std::exception const & e)
  {
    auto text = ExceptionDescriptionToString(e);
    std::string const expected = "1: ThrowFunc2\n"\
                                 "2: ThrowFunc1";
    ASSERT_TRUE(text == expected);
  }
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringA_3_unknown)
{
  try
  {
    ThrowFunc4();
    ASSERT_TRUE(false);
  }
  catch (std::exception const & e)
  {
    auto text = ExceptionDescriptionToString(e);
    std::string const expected = "1: ThrowFunc4\n"\
                                 "2: Unknown exception";
    ASSERT_TRUE(text == expected);
  }
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringB_1)
{
  try
  {
    ThrowFunc1();
    ASSERT_TRUE(false);
  }
  catch (std::exception const & e)
  {
    auto text = ExceptionDescriptionToString(std::current_exception());
    std::string const expected = "1: ThrowFunc1";
    ASSERT_TRUE(text == expected);
  }
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringB_2)
{
  try
  {
    ThrowFunc2();
    ASSERT_TRUE(false);
  }
  catch (std::exception const & e)
  {
    auto text = ExceptionDescriptionToString(std::current_exception());
    std::string const expected = "1: ThrowFunc2\n"\
                                 "2: ThrowFunc1";
    ASSERT_TRUE(text == expected);
  }
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringB_3_unknown)
{
  try
  {
    ThrowFunc4();
    ASSERT_TRUE(false);
  }
  catch (std::exception const & e)
  {
    auto text = ExceptionDescriptionToString(std::current_exception());
    std::string const expected = "1: ThrowFunc4\n"\
                                 "2: Unknown exception";
    ASSERT_TRUE(text == expected);
  }
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringB_NoException)
{
  std::string s;
  ASSERT_THROW(s = ExceptionDescriptionToString(std::exception_ptr()), std::invalid_argument);
  (void)s;
}
TEST(gpcc_string_tools_Tests, ExceptionDescriptionToStringB_UnknownException)
{
  try
  {
    uint32_t i = 5;
    throw i;
    ASSERT_TRUE(false);
  }
  catch (...)
  {
    auto text = ExceptionDescriptionToString(std::current_exception());
    ASSERT_STREQ(text.c_str(), "1: Unknown exception");
    return;
  }

  FAIL();
}

TEST(gpcc_string_tools_Tests, HexDump_8bit)
{
  uint8_t const data[8] =
  {
    0x41,
    0x42,
    0x61,
    0xFF,
    0xAB,
    0x21,
    0x7E,
    0x12
  };

  std::string result = HexDump(0x1234ABCDU, data, 8, 1, 8);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD: 41 42 61 FF AB 21 7E 12 ABa..!~.");

  result = HexDump(0x1234ABCDU, data, 4, 1, 8);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD: 41 42 61 FF             ABa.");

  result = HexDump(0x1234ABCDU, data, 0, 1, 8);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD:                         ");
}
TEST(gpcc_string_tools_Tests, HexDump_16bit)
{
  uint16_t const data[4] =
  {
    0x0102,
    0x0304,
    0x0506,
    0x0708
  };

  std::string result = HexDump(0x1234ABCDU, data, 8, 2, 4);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD: 0102 0304 0506 0708 ........");

  result = HexDump(0x1234ABCDU, data, 4, 2, 4);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD: 0102 0304           ....");

  result = HexDump(0x1234ABCDU, data, 0, 2, 4);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD:                     ");
}
TEST(gpcc_string_tools_Tests, HexDump_32bit)
{
  uint32_t const data[2] =
  {
    0x01020304,
    0x05060708
  };

  std::string result = HexDump(0x1234ABCDU, data, 8, 4, 2);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD: 01020304 05060708 ........");

  result = HexDump(0x1234ABCDU, data, 4, 4, 2);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD: 01020304          ....");

  result = HexDump(0x1234ABCDU, data, 0, 4, 2);
  ASSERT_STREQ(result.c_str(), "0x1234ABCD:                   ");
}
TEST(gpcc_string_tools_Tests, HexDump_Errors)
{
  uint8_t const data[8] =
  {
    0x41,
    0x42,
    0x61,
    0xFF,
    0xAB,
    0x21,
    0x7E,
    0x12
  };

  std::string s;

  // pData nullptr
  ASSERT_THROW(s = HexDump(0x12345678U, nullptr, 8, 1, 8), std::invalid_argument);

  // wordSize zero
  ASSERT_THROW(s = HexDump(0x12345678U, data, 8, 0, 8), std::invalid_argument);

  // n % wordSize != 0
  ASSERT_THROW(s = HexDump(0x12345678U, data, 7, 2, 4), std::invalid_argument);

  // valuesPerLine too small
  ASSERT_THROW(s = HexDump(0x12345678U, data, 8, 1, 4), std::invalid_argument);

  // invalid word size
  ASSERT_THROW(s = HexDump(0x12345678U, data, 8, 8, 1), std::invalid_argument);

  (void)s;
}
TEST(gpcc_string_tools_Tests, ToHex)
{
  std::string s;

  // minimum with
  s = ToHex(0, 0);
  EXPECT_TRUE(s == "0x0");
  s = ToHex(0, 1);
  EXPECT_TRUE(s == "0x0");
  s = ToHex(0, 2);
  EXPECT_TRUE(s == "0x00");
  s = ToHex(0, 3);
  EXPECT_TRUE(s == "0x000");
  s = ToHex(0, 4);
  EXPECT_TRUE(s == "0x0000");
  s = ToHex(0, 5);
  EXPECT_TRUE(s == "0x00000");
  s = ToHex(0, 6);
  EXPECT_TRUE(s == "0x000000");
  s = ToHex(0, 7);
  EXPECT_TRUE(s == "0x0000000");
  s = ToHex(0, 8);
  EXPECT_TRUE(s == "0x00000000");

  // number larger than minimum width
  s = ToHex(1024, 2);
  EXPECT_TRUE(s == "0x400");

  // upper case charaters
  s = ToHex(10, 2);
  EXPECT_TRUE(s == "0x0A");

  // bad width
  ASSERT_THROW(s = ToHex(0, 9), std::invalid_argument);
}
TEST(gpcc_string_tools_Tests, ToBin)
{
  std::string s;

  // minimum width
  s = ToBin(0, 0);
  EXPECT_STREQ(s.c_str(), "0b0");
  s = ToBin(0, 1);
  EXPECT_STREQ(s.c_str(), "0b0");
  s = ToBin(0, 2);
  EXPECT_STREQ(s.c_str(), "0b00");

  s = ToBin(0, 32);
  EXPECT_STREQ(s.c_str(), "0b00000000000000000000000000000000");

  // some numbers
  s = ToBin(1U, 8);
  EXPECT_STREQ(s.c_str(), "0b00000001");

  s = ToBin(17U, 8);
  EXPECT_STREQ(s.c_str(), "0b00010001");

  s = ToBin(254U, 8);
  EXPECT_STREQ(s.c_str(), "0b11111110");

  // number larger than minimum width
  s = ToBin(17U, 2);
  EXPECT_STREQ(s.c_str(), "0b10001");

  // bad width
  ASSERT_THROW(s = ToBin(0, 33), std::invalid_argument);
}
TEST(gpcc_string_tools_Tests, ToHexNoPrefix)
{
  std::string s;

  // minimum with
  s = ToHexNoPrefix(0, 0);
  EXPECT_TRUE(s == "0");
  s = ToHexNoPrefix(0, 1);
  EXPECT_TRUE(s == "0");
  s = ToHexNoPrefix(0, 2);
  EXPECT_TRUE(s == "00");
  s = ToHexNoPrefix(0, 3);
  EXPECT_TRUE(s == "000");
  s = ToHexNoPrefix(0, 4);
  EXPECT_TRUE(s == "0000");
  s = ToHexNoPrefix(0, 5);
  EXPECT_TRUE(s == "00000");
  s = ToHexNoPrefix(0, 6);
  EXPECT_TRUE(s == "000000");
  s = ToHexNoPrefix(0, 7);
  EXPECT_TRUE(s == "0000000");
  s = ToHexNoPrefix(0, 8);
  EXPECT_TRUE(s == "00000000");

  // number larger than minimum width
  s = ToHexNoPrefix(1024, 2);
  EXPECT_TRUE(s == "400");

  // upper case charaters
  s = ToHexNoPrefix(10, 2);
  EXPECT_TRUE(s == "0A");

  // bad width
  ASSERT_THROW(s = ToHexNoPrefix(0, 9), std::invalid_argument);
}
TEST(gpcc_string_tools_Tests, ToDecAndHex)
{
  std::string s;

  // minimum with
  s = ToDecAndHex(0, 0);
  EXPECT_TRUE(s == "0 (0x0)");
  s = ToDecAndHex(0, 1);
  EXPECT_TRUE(s == "0 (0x0)");
  s = ToDecAndHex(0, 2);
  EXPECT_TRUE(s == "0 (0x00)");
  s = ToDecAndHex(0, 3);
  EXPECT_TRUE(s == "0 (0x000)");
  s = ToDecAndHex(0, 4);
  EXPECT_TRUE(s == "0 (0x0000)");
  s = ToDecAndHex(0, 5);
  EXPECT_TRUE(s == "0 (0x00000)");
  s = ToDecAndHex(0, 6);
  EXPECT_TRUE(s == "0 (0x000000)");
  s = ToDecAndHex(0, 7);
  EXPECT_TRUE(s == "0 (0x0000000)");
  s = ToDecAndHex(0, 8);
  EXPECT_TRUE(s == "0 (0x00000000)");

  // number larger than minimum width
  s = ToDecAndHex(1024, 2);
  EXPECT_TRUE(s == "1024 (0x400)");

  // upper case charaters
  s = ToDecAndHex(10, 2);
  EXPECT_TRUE(s == "10 (0x0A)");

  // bad width
  ASSERT_THROW(s = ToDecAndHex(0, 9), std::invalid_argument);
}

// Conversion string to X -----------------------------------------------------
TEST(gpcc_string_tools_Tests, DecimalToU8)
{
  // valid numbers within range
  EXPECT_EQ(0U,   DecimalToU8("0"));
  EXPECT_EQ(0U,   DecimalToU8("00"));
  EXPECT_EQ(0U,   DecimalToU8("+0"));
  EXPECT_EQ(0U,   DecimalToU8("+00"));
  EXPECT_EQ(0U,   DecimalToU8("-0"));
  EXPECT_EQ(0U,   DecimalToU8("-00"));
  EXPECT_EQ(1U,   DecimalToU8("1"));
  EXPECT_EQ(3U,   DecimalToU8("+3"));
  EXPECT_EQ(12U,  DecimalToU8("012"));
  EXPECT_EQ(12U,  DecimalToU8("+12"));
  EXPECT_EQ(12U,  DecimalToU8("+012"));
  EXPECT_EQ(254U, DecimalToU8("254"));
  EXPECT_EQ(255U, DecimalToU8("255"));
  EXPECT_EQ(255U, DecimalToU8("+255"));

  // valid numbers out of range
  EXPECT_THROW(DecimalToU8("-1"),  std::out_of_range);
  EXPECT_THROW(DecimalToU8("256"), std::out_of_range);

  // invalid values
  EXPECT_THROW(DecimalToU8(""),    std::invalid_argument);
  EXPECT_THROW(DecimalToU8(" "),   std::invalid_argument);
  EXPECT_THROW(DecimalToU8(" 0"),  std::invalid_argument);
  EXPECT_THROW(DecimalToU8("0 "),  std::invalid_argument);
  EXPECT_THROW(DecimalToU8("--0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU8("++0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU8("X7"),  std::invalid_argument);
  EXPECT_THROW(DecimalToU8("7X"),  std::invalid_argument);
  EXPECT_THROW(DecimalToU8("0x0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU8("0b0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU8("0X0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU8("0B0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU8("c"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyNumberToU8)
{
  // valid binary values within range
  EXPECT_EQ(0U,   AnyNumberToU8("0b0"));
  EXPECT_EQ(1U,   AnyNumberToU8("0b1"));
  EXPECT_EQ(2U,   AnyNumberToU8("0b10"));
  EXPECT_EQ(11U,  AnyNumberToU8("0b01011"));
  EXPECT_EQ(11U,  AnyNumberToU8("0b000000001011"));
  EXPECT_EQ(255U, AnyNumberToU8("0b11111111"));
  EXPECT_EQ(255U, AnyNumberToU8("0b011111111"));

  // valid binary values out of range
  EXPECT_THROW(AnyNumberToU8("0b100000000"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU8("0b100000001"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU8("0b111111111"), std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(0U,   AnyNumberToU8("0x0"));
  EXPECT_EQ(16U,  AnyNumberToU8("0x10"));
  EXPECT_EQ(12U,  AnyNumberToU8("0xc"));
  EXPECT_EQ(12U,  AnyNumberToU8("0xC"));
  EXPECT_EQ(254U, AnyNumberToU8("0xFE"));
  EXPECT_EQ(255U, AnyNumberToU8("0xFF"));
  EXPECT_EQ(255U, AnyNumberToU8("0x0FF"));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyNumberToU8("0x100"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU8("0x101"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU8("0xFFF"), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(0U,   AnyNumberToU8("0"));
  EXPECT_EQ(0U,   AnyNumberToU8("+0"));
  EXPECT_EQ(0U,   AnyNumberToU8("-0"));
  EXPECT_EQ(1U,   AnyNumberToU8("1"));
  EXPECT_EQ(3U,   AnyNumberToU8("+3"));
  EXPECT_EQ(12U,  AnyNumberToU8("12"));
  EXPECT_EQ(12U,  AnyNumberToU8("+12"));
  EXPECT_EQ(254U, AnyNumberToU8("254"));
  EXPECT_EQ(255U, AnyNumberToU8("255"));
  EXPECT_EQ(255U, AnyNumberToU8("+255"));

  // valid decimal values out of range
  EXPECT_THROW(AnyNumberToU8("-1"),   std::out_of_range);
  EXPECT_THROW(AnyNumberToU8("256"),  std::out_of_range);
  EXPECT_THROW(AnyNumberToU8("+256"), std::out_of_range);

  // invalid values
  EXPECT_THROW(AnyNumberToU8(""),      std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8(" "),     std::invalid_argument);

  EXPECT_THROW(AnyNumberToU8("0XC"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("00XC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("00xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8(" 0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0xC "),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("-0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("--0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("+0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("++0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0x0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0x0XC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0xG"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU8("0B0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("00B0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("00b0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8(" 0b0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b0 "),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("-0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("--0b1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("+0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("++0b1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b0b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b0B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b1b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b1B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b0x0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b0X0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b1x0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b1X0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b0c1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b00b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b00B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0b2"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU8(" 0"),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("0 "),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("--0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("++0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("12x"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU8("x12"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("X12"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("b11"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU8("B11"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyStringToU8)
{
  // valid binary values within range
  EXPECT_EQ(0U,   AnyStringToU8("0b0"));
  EXPECT_EQ(1U,   AnyStringToU8("0b1"));
  EXPECT_EQ(2U,   AnyStringToU8("0b10"));
  EXPECT_EQ(11U,  AnyStringToU8("0b01011"));
  EXPECT_EQ(11U,  AnyStringToU8("0b000000001011"));
  EXPECT_EQ(255U, AnyStringToU8("0b11111111"));
  EXPECT_EQ(255U, AnyStringToU8("0b011111111"));

  // valid binary values out of range
  EXPECT_THROW(AnyStringToU8("0b100000000"), std::out_of_range);
  EXPECT_THROW(AnyStringToU8("0b100000001"), std::out_of_range);
  EXPECT_THROW(AnyStringToU8("0b111111111"), std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(0U,   AnyStringToU8("0x0"));
  EXPECT_EQ(16U,  AnyStringToU8("0x10"));
  EXPECT_EQ(12U,  AnyStringToU8("0xc"));
  EXPECT_EQ(12U,  AnyStringToU8("0xC"));
  EXPECT_EQ(254U, AnyStringToU8("0xFE"));
  EXPECT_EQ(255U, AnyStringToU8("0xFF"));
  EXPECT_EQ(255U, AnyStringToU8("0x0FF"));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyStringToU8("0x100"), std::out_of_range);
  EXPECT_THROW(AnyStringToU8("0x101"), std::out_of_range);
  EXPECT_THROW(AnyStringToU8("0xFFF"), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(0U,   AnyStringToU8("0"));
  EXPECT_EQ(0U,   AnyStringToU8("+0"));
  EXPECT_EQ(0U,   AnyStringToU8("-0"));
  EXPECT_EQ(1U,   AnyStringToU8("1"));
  EXPECT_EQ(3U,   AnyStringToU8("+3"));
  EXPECT_EQ(12U,  AnyStringToU8("12"));
  EXPECT_EQ(12U,  AnyStringToU8("+12"));
  EXPECT_EQ(254U, AnyStringToU8("254"));
  EXPECT_EQ(255U, AnyStringToU8("255"));
  EXPECT_EQ(255U, AnyStringToU8("+255"));

  // valid decimal values out of range
  EXPECT_THROW(AnyStringToU8("-1"),   std::out_of_range);
  EXPECT_THROW(AnyStringToU8("256"),  std::out_of_range);
  EXPECT_THROW(AnyStringToU8("+256"), std::out_of_range);

  // valid characters
  EXPECT_EQ(0x41U, AnyStringToU8("'A'"));
  EXPECT_EQ(0x42U, AnyStringToU8("'B'"));
  EXPECT_EQ(0x27U, AnyStringToU8("'''"));
  EXPECT_EQ(0x22U, AnyStringToU8("'\"'"));

  // invalid values
  EXPECT_THROW(AnyStringToU8(""),      std::invalid_argument);
  EXPECT_THROW(AnyStringToU8(" "),     std::invalid_argument);

  EXPECT_THROW(AnyStringToU8("0XC"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("00XC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("00xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8(" 0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0xC "),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("-0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("--0xC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("+0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("++0xC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0x0xC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0x0XC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0xG"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToU8("0B0"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("00B0"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("00b0"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8(" 0b0"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b0 "),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("-0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("--0b1"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("+0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("++0b1"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b0b0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b0B0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b1b0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b1B0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b0x0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b0X0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b1x0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b1X0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b0c1"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b00b0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b00B0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0b2"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToU8(" 0"),    std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("0 "),    std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("--0"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("++0"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("12x"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToU8("x12"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("X12"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("b11"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("B11"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToU8("A"),     std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("'A"),    std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("A'"),    std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("'AA"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToU8(" 'A'"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("'A' "),  std::invalid_argument);
  EXPECT_THROW(AnyStringToU8("'AB'"),  std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, TwoDigitHexToU8)
{
  EXPECT_EQ(  0U, TwoDigitHexToU8("00"));
  EXPECT_EQ(  1U, TwoDigitHexToU8("01"));
  EXPECT_EQ( 16U, TwoDigitHexToU8("10"));
  EXPECT_EQ( 10U, TwoDigitHexToU8("0A"));
  EXPECT_EQ( 10U, TwoDigitHexToU8("0a"));
  EXPECT_EQ(240U, TwoDigitHexToU8("F0"));
  EXPECT_EQ(240U, TwoDigitHexToU8("f0"));
  EXPECT_EQ(255U, TwoDigitHexToU8("FF"));
  EXPECT_EQ(255U, TwoDigitHexToU8("ff"));

  EXPECT_THROW(TwoDigitHexToU8(""),    std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8(" "),   std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8(" 0"),  std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("0 "),  std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8(" 00"), std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("00 "), std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("1"),   std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("123"), std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("G0"),  std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("-1"),  std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("+1"),  std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("-10"), std::invalid_argument);
  EXPECT_THROW(TwoDigitHexToU8("+10"), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, FourDigitHexToU16)
{
  EXPECT_EQ(    0U, FourDigitHexToU16("0000"));
  EXPECT_EQ(    1U, FourDigitHexToU16("0001"));
  EXPECT_EQ(   16U, FourDigitHexToU16("0010"));
  EXPECT_EQ(   10U, FourDigitHexToU16("000A"));
  EXPECT_EQ(   10U, FourDigitHexToU16("000a"));
  EXPECT_EQ(61440U, FourDigitHexToU16("F000"));
  EXPECT_EQ(61440U, FourDigitHexToU16("f000"));
  EXPECT_EQ(65535U, FourDigitHexToU16("FFFF"));
  EXPECT_EQ(65535U, FourDigitHexToU16("ffff"));

  EXPECT_THROW(FourDigitHexToU16(""),      std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16(" "),     std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16(" 000"),  std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("000 "),  std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16(" 0000"), std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("0000 "), std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("1"),     std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("12345"), std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("G0"),    std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("-100"),  std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("+100"),  std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("-1000"), std::invalid_argument);
  EXPECT_THROW(FourDigitHexToU16("+1000"), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, DecimalToU32)
{
  // valid numbers within range
  EXPECT_EQ(0U,           DecimalToU32("0"));
  EXPECT_EQ(0U,           DecimalToU32("00"));
  EXPECT_EQ(0U,           DecimalToU32("+0"));
  EXPECT_EQ(0U,           DecimalToU32("+00"));
  EXPECT_EQ(0U,           DecimalToU32("-0"));
  EXPECT_EQ(0U,           DecimalToU32("-00"));
  EXPECT_EQ(1U,           DecimalToU32("1"));
  EXPECT_EQ(3U,           DecimalToU32("+3"));
  EXPECT_EQ(12U,          DecimalToU32("012"));
  EXPECT_EQ(12U,          DecimalToU32("+12"));
  EXPECT_EQ(12U,          DecimalToU32("+012"));
  EXPECT_EQ(4294967294UL, DecimalToU32("4294967294"));
  EXPECT_EQ(4294967295UL, DecimalToU32("4294967295"));
  EXPECT_EQ(4294967295UL, DecimalToU32("+4294967295"));

  // valid numbers out of range
  EXPECT_THROW(DecimalToU32("-1"),         std::out_of_range);
  EXPECT_THROW(DecimalToU32("4294967296"), std::out_of_range);

  // invalid values
  EXPECT_THROW(DecimalToU32(""),    std::invalid_argument);
  EXPECT_THROW(DecimalToU32(" "),   std::invalid_argument);
  EXPECT_THROW(DecimalToU32(" 0"),  std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0 "),  std::invalid_argument);
  EXPECT_THROW(DecimalToU32("--0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("++0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("X7"),  std::invalid_argument);
  EXPECT_THROW(DecimalToU32("7X"),  std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0x0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0b0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0X0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0B0"), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("c"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, DecimalToU32_minmax)
{
  // valid values within range
  EXPECT_EQ(10U, DecimalToU32("10",   10, 20));
  EXPECT_EQ(10U, DecimalToU32("010",  10, 20));
  EXPECT_EQ(10U, DecimalToU32("+10",  10, 20));
  EXPECT_EQ(10U, DecimalToU32("+010", 10, 20));
  EXPECT_EQ(11U, DecimalToU32("11",   10, 20));
  EXPECT_EQ(19U, DecimalToU32("19",   10, 20));
  EXPECT_EQ(19U, DecimalToU32("019",  10, 20));
  EXPECT_EQ(20U, DecimalToU32("20",   10, 20));
  EXPECT_EQ(20U, DecimalToU32("020",  10, 20));
  EXPECT_EQ(20U, DecimalToU32("+20",  10, 20));
  EXPECT_EQ(20U, DecimalToU32("+020", 10, 20));

  // valid values out of range
  EXPECT_THROW(DecimalToU32("-1", 10, 20), std::out_of_range);
  EXPECT_THROW(DecimalToU32("9",  10, 20), std::out_of_range);
  EXPECT_THROW(DecimalToU32("21", 10, 20), std::out_of_range);

  // invalid values
  EXPECT_THROW(DecimalToU32("",    0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32(" ",   0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32(" 0",  0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0 ",  0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("--0", 0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("++0", 0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("X7",  0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("7X",  0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0x0", 0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0b0", 0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0X0", 0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("0B0", 0, 255), std::invalid_argument);
  EXPECT_THROW(DecimalToU32("c",   0, 255), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, HexToU32)
{
  // valid hexadecimal values within range
  EXPECT_EQ(0U,           HexToU32("0x0"));
  EXPECT_EQ(16U,          HexToU32("0x10"));
  EXPECT_EQ(12U,          HexToU32("0xc"));
  EXPECT_EQ(12U,          HexToU32("0xC"));
  EXPECT_EQ(12U,          HexToU32("0x0c"));
  EXPECT_EQ(12U,          HexToU32("0x0C"));
  EXPECT_EQ(4294967294UL, HexToU32("0xFFFFFFFE"));
  EXPECT_EQ(4294967295UL, HexToU32("0xFFFFFFFF"));
  EXPECT_EQ(4294967295UL, HexToU32("0x0FFFFFFFF"));

  // valid hexadecimal values out of range
  EXPECT_THROW(HexToU32("0x100000000"), std::out_of_range);
  EXPECT_THROW(HexToU32("0x100000001"), std::out_of_range);
  EXPECT_THROW(HexToU32("0xFFFFFFFFF"), std::out_of_range);

  // invalid values
  EXPECT_THROW(HexToU32(""),      std::invalid_argument);
  EXPECT_THROW(HexToU32(" "),     std::invalid_argument);

  EXPECT_THROW(HexToU32("0"),     std::invalid_argument);
  EXPECT_THROW(HexToU32("A"),     std::invalid_argument);

  EXPECT_THROW(HexToU32("0XC"),   std::invalid_argument);
  EXPECT_THROW(HexToU32(" 0xC"),  std::invalid_argument);
  EXPECT_THROW(HexToU32("0xC "),  std::invalid_argument);
  EXPECT_THROW(HexToU32("-0xC"),  std::invalid_argument);
  EXPECT_THROW(HexToU32("--0xC"), std::invalid_argument);
  EXPECT_THROW(HexToU32("+0xC"),  std::invalid_argument);
  EXPECT_THROW(HexToU32("++0xC"), std::invalid_argument);
  EXPECT_THROW(HexToU32("0x0xC"), std::invalid_argument);
  EXPECT_THROW(HexToU32("0xG"),   std::invalid_argument);

  EXPECT_THROW(HexToU32("0B0"),   std::invalid_argument);
  EXPECT_THROW(HexToU32(" 0b0"),  std::invalid_argument);
  EXPECT_THROW(HexToU32("0b0 "),  std::invalid_argument);
  EXPECT_THROW(HexToU32("-0b1"),  std::invalid_argument);
  EXPECT_THROW(HexToU32("--0b1"), std::invalid_argument);
  EXPECT_THROW(HexToU32("+0b1"),  std::invalid_argument);
  EXPECT_THROW(HexToU32("++0b1"), std::invalid_argument);
  EXPECT_THROW(HexToU32("0b0b0"), std::invalid_argument);
  EXPECT_THROW(HexToU32("0b2"),   std::invalid_argument);

  EXPECT_THROW(HexToU32(" 0"),    std::invalid_argument);
  EXPECT_THROW(HexToU32("0 "),    std::invalid_argument);
  EXPECT_THROW(HexToU32("--0"),   std::invalid_argument);
  EXPECT_THROW(HexToU32("++0"),   std::invalid_argument);
  EXPECT_THROW(HexToU32("12x"),   std::invalid_argument);

  EXPECT_THROW(HexToU32("x12"),   std::invalid_argument);
  EXPECT_THROW(HexToU32("b11"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, HexToU32_minmax)
{
  // valid hexadecimal values within range
  EXPECT_EQ(10U,  HexToU32("0xA",   10, 20));
  EXPECT_EQ(11U,  HexToU32("0xb",   10, 20));
  EXPECT_EQ(12U,  HexToU32("0xC",   10, 20));
  EXPECT_EQ(20UL, HexToU32("0x14",  10, 20));
  EXPECT_EQ(20UL, HexToU32("0x014", 10, 20));

  // valid hexadecimal values out of range
  EXPECT_THROW(HexToU32("0x9",   10, 20), std::out_of_range);
  EXPECT_THROW(HexToU32("0x15",  10, 20), std::out_of_range);

  // invalid values
  EXPECT_THROW(HexToU32("",      10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32(" ",     10, 20), std::invalid_argument);

  EXPECT_THROW(HexToU32("0",     10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("A",     10, 20), std::invalid_argument);

  EXPECT_THROW(HexToU32("0XC",   10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32(" 0xC",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0xC ",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("-0xC",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("--0xC", 10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("+0xC",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("++0xC", 10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0x0xC", 10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0xG",   10, 20), std::invalid_argument);

  EXPECT_THROW(HexToU32("0B0",   10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32(" 0b0",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0b0 ",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("-0b1",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("--0b1", 10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("+0b1",  10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("++0b1", 10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0b0b0", 10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0b2",   10, 20), std::invalid_argument);

  EXPECT_THROW(HexToU32(" 0",    10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("0 ",    10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("--0",   10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("++0",   10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("12x",   10, 20), std::invalid_argument);

  EXPECT_THROW(HexToU32("x12",   10, 20), std::invalid_argument);
  EXPECT_THROW(HexToU32("b11",   10, 20), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyNumberToU32)
{
  // valid binary values within range
  EXPECT_EQ(0U,           AnyNumberToU32("0b0"));
  EXPECT_EQ(1U,           AnyNumberToU32("0b1"));
  EXPECT_EQ(2U,           AnyNumberToU32("0b10"));
  EXPECT_EQ(11U,          AnyNumberToU32("0b01011"));
  EXPECT_EQ(4294967295UL, AnyNumberToU32("0b11111111111111111111111111111111"));
  EXPECT_EQ(4294967295UL, AnyNumberToU32("0b011111111111111111111111111111111"));

  // valid binary values out of range
  EXPECT_THROW(AnyNumberToU32("0b100000000000000000000000000000000"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("0b100000000000000000000000000000001"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("0b111111111111111111111111111111111"), std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(0U,           AnyNumberToU32("0x0"));
  EXPECT_EQ(16U,          AnyNumberToU32("0x10"));
  EXPECT_EQ(12U,          AnyNumberToU32("0xc"));
  EXPECT_EQ(12U,          AnyNumberToU32("0xC"));
  EXPECT_EQ(4294967294UL, AnyNumberToU32("0xFFFFFFFE"));
  EXPECT_EQ(4294967295UL, AnyNumberToU32("0xFFFFFFFF"));
  EXPECT_EQ(4294967295UL, AnyNumberToU32("0x0FFFFFFFF"));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyNumberToU32("0x100000000"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("0x100000001"), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("0xFFFFFFFFF"), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(0U,           AnyNumberToU32("0"));
  EXPECT_EQ(0U,           AnyNumberToU32("+0"));
  EXPECT_EQ(0U,           AnyNumberToU32("-0"));
  EXPECT_EQ(1U,           AnyNumberToU32("1"));
  EXPECT_EQ(3U,           AnyNumberToU32("+3"));
  EXPECT_EQ(12U,          AnyNumberToU32("12"));
  EXPECT_EQ(12U,          AnyNumberToU32("+12"));
  EXPECT_EQ(4294967295UL, AnyNumberToU32("4294967295"));
  EXPECT_EQ(4294967295UL, AnyNumberToU32("+4294967295"));

  // valid decimal values out of range
  EXPECT_THROW(AnyNumberToU32("-1"),          std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("4294967296"),  std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("+4294967296"), std::out_of_range);

  // invalid values
  EXPECT_THROW(AnyNumberToU32(""),      std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32(" "),     std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32("0XC"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00XC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32(" 0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0xC "),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("-0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("--0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("+0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("++0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0x0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0x0XC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0xG"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32("0B0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00B0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00b0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32(" 0b0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0 "),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("-0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("--0b1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("+0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("++0b1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0x0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0X0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1x0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1X0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0c1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b00b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b00B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b2"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32(" 0"),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0 "),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("--0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("++0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("12x"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32("x12"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("X12"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("b11"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("B11"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyNumberToU32_minmax)
{
  // valid binary values within range
  EXPECT_EQ(10U, AnyNumberToU32("0b1010",   10, 20));
  EXPECT_EQ(11U, AnyNumberToU32("0b1011",   10, 20));
  EXPECT_EQ(12U, AnyNumberToU32("0b1100",   10, 20));
  EXPECT_EQ(12U, AnyNumberToU32("0b01100",  10, 20));
  EXPECT_EQ(20U, AnyNumberToU32("0b10100",  10, 20));
  EXPECT_EQ(20U, AnyNumberToU32("0b010100", 10, 20));

  // valid binary values out of range
  EXPECT_THROW(AnyNumberToU32("0b1001",  10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("0b10101", 10, 20), std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(10U,  AnyNumberToU32("0xA",   10, 20));
  EXPECT_EQ(11U,  AnyNumberToU32("0xb",   10, 20));
  EXPECT_EQ(12U,  AnyNumberToU32("0xC",   10, 20));
  EXPECT_EQ(20UL, AnyNumberToU32("0x14",  10, 20));
  EXPECT_EQ(20UL, AnyNumberToU32("0x014", 10, 20));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyNumberToU32("0x9",  10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("0x15", 10, 20), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(10U, AnyNumberToU32("10",  10, 20));
  EXPECT_EQ(10U, AnyNumberToU32("+10", 10, 20));
  EXPECT_EQ(11U, AnyNumberToU32("11",  10, 20));
  EXPECT_EQ(20U, AnyNumberToU32("20",  10, 20));
  EXPECT_EQ(20U, AnyNumberToU32("+20", 10, 20));

  // valid decimal values out of range
  EXPECT_THROW(AnyNumberToU32("-1",  10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("9",   10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("21",  10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToU32("+21", 10, 20), std::out_of_range);

  // invalid values
  EXPECT_THROW(AnyNumberToU32("", 10, 20),      std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32(" ", 10, 20),     std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32("0XC", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00XC", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00xC", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32(" 0xC", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0xC ", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("-0xC", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("--0xC", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("+0xC", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("++0xC", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0x0xC", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0x0XC", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0xG", 10, 20),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32("0B0", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00B0", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("00b0", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32(" 0b0", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0 ", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("-0b1", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("--0b1", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("+0b1", 10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("++0b1", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0b0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0B0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1b0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1B0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0x0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0X0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1x0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b1X0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b0c1", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b00b0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b00B0", 10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0b2", 10, 20),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32(" 0", 10, 20),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("0 ", 10, 20),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("--0", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("++0", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("12x", 10, 20),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToU32("x12", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("X12", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("b11", 10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToU32("B11", 10, 20),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyStringToChar)
{
  // valid binary values within range
  EXPECT_EQ(0,  AnyStringToChar("0b0"));
  EXPECT_EQ(1,  AnyStringToChar("0b1"));
  EXPECT_EQ(2,  AnyStringToChar("0b10"));
  EXPECT_EQ(11, AnyStringToChar("0b01011"));
  EXPECT_EQ(-1, AnyStringToChar("0b11111111"));
  EXPECT_EQ(-1, AnyStringToChar("0b011111111"));
  EXPECT_EQ(-2, AnyStringToChar("0b11111110"));

  // valid binary values out of range
  EXPECT_THROW(AnyStringToChar("0b100000000"), std::out_of_range);
  EXPECT_THROW(AnyStringToChar("0b100000001"), std::out_of_range);
  EXPECT_THROW(AnyStringToChar("0b111111111"), std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(0,  AnyStringToChar("0x0"));
  EXPECT_EQ(16, AnyStringToChar("0x10"));
  EXPECT_EQ(12, AnyStringToChar("0xc"));
  EXPECT_EQ(12, AnyStringToChar("0xC"));
  EXPECT_EQ(-2, AnyStringToChar("0xFE"));
  EXPECT_EQ(-1, AnyStringToChar("0xFF"));
  EXPECT_EQ(-1, AnyStringToChar("0x0FF"));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyStringToChar("0x100"), std::out_of_range);
  EXPECT_THROW(AnyStringToChar("0x101"), std::out_of_range);
  EXPECT_THROW(AnyStringToChar("0xFFF"), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(0,    AnyStringToChar("0"));
  EXPECT_EQ(0,    AnyStringToChar("+0"));
  EXPECT_EQ(0,    AnyStringToChar("-0"));
  EXPECT_EQ(1,    AnyStringToChar("1"));
  EXPECT_EQ(3,    AnyStringToChar("+3"));
  EXPECT_EQ(12,   AnyStringToChar("12"));
  EXPECT_EQ(12,   AnyStringToChar("+12"));
  EXPECT_EQ(-128, AnyStringToChar("-128"));
  EXPECT_EQ(127,  AnyStringToChar("127"));
  EXPECT_EQ(127,  AnyStringToChar("+127"));

  // valid decimal values out of range
  EXPECT_THROW(AnyStringToChar("-129"), std::out_of_range);
  EXPECT_THROW(AnyStringToChar("128"),  std::out_of_range);
  EXPECT_THROW(AnyStringToChar("+128"), std::out_of_range);

  // valid characters
  EXPECT_EQ('A',  AnyStringToChar("'A'"));
  EXPECT_EQ('B',  AnyStringToChar("'B'"));
  EXPECT_EQ('\'', AnyStringToChar("'''"));
  EXPECT_EQ('\"', AnyStringToChar("'\"'"));

  // invalid values
  EXPECT_THROW(AnyStringToChar(""),      std::invalid_argument);
  EXPECT_THROW(AnyStringToChar(" "),     std::invalid_argument);

  EXPECT_THROW(AnyStringToChar("0XC"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("00XC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("00xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar(" 0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0xC "),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("-0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("--0xC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("+0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("++0xC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0x0xC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0x0XC"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0xG"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToChar("0B0"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("00B0"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("00b0"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar(" 0b0"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b0 "),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("-0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("--0b1"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("+0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("++0b1"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b0b0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b0B0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b1b0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b1B0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b0x0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b0X0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b1x0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b1X0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b0c1"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b00b0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b00B0"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0b2"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToChar(" 0"),    std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("0 "),    std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("--0"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("++0"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("12x"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToChar("x12"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("X12"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("b11"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("B11"),   std::invalid_argument);

  EXPECT_THROW(AnyStringToChar("A"),    std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("'A"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("A'"),   std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("'AA"),  std::invalid_argument);
  EXPECT_THROW(AnyStringToChar(" 'A'"), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("'A' "), std::invalid_argument);
  EXPECT_THROW(AnyStringToChar("'AB'"), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, DecimalToI32)
{
  // valid numbers in range
  EXPECT_EQ(0,            DecimalToI32("0"));
  EXPECT_EQ(0,            DecimalToI32("00"));
  EXPECT_EQ(0,            DecimalToI32("+0"));
  EXPECT_EQ(0,            DecimalToI32("+00"));
  EXPECT_EQ(0,            DecimalToI32("-0"));
  EXPECT_EQ(0,            DecimalToI32("-00"));
  EXPECT_EQ(1,            DecimalToI32("1"));
  EXPECT_EQ(3,            DecimalToI32("+3"));
  EXPECT_EQ(12,           DecimalToI32("012"));
  EXPECT_EQ(12,           DecimalToI32("+12"));
  EXPECT_EQ(12,           DecimalToI32("+012"));
  EXPECT_EQ(-1,           DecimalToI32("-1"));
  EXPECT_EQ(-1,           DecimalToI32("-001"));
  EXPECT_EQ(2147483647L,  DecimalToI32("2147483647"));
  EXPECT_EQ(2147483647L,  DecimalToI32("+2147483647"));
  EXPECT_EQ(-2147483648L, DecimalToI32("-2147483648"));

  // valid numbers out of range
  EXPECT_THROW(DecimalToI32("2147483648"),  std::out_of_range);
  EXPECT_THROW(DecimalToI32("-2147483649"), std::out_of_range);

  // invalid values
  EXPECT_THROW(DecimalToI32(""),    std::invalid_argument);
  EXPECT_THROW(DecimalToI32(" "),   std::invalid_argument);
  EXPECT_THROW(DecimalToI32(" 0"),  std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0 "),  std::invalid_argument);
  EXPECT_THROW(DecimalToI32("++1"), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("--1"), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("X7"),  std::invalid_argument);
  EXPECT_THROW(DecimalToI32("7X"),  std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0x0"), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0b0"), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0X0"), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0B0"), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("c"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, DecimalToI32_minmax)
{
  // valid values within range
  EXPECT_EQ(-10, DecimalToI32("-10", -10, 20));
  EXPECT_EQ(-10, DecimalToI32("-010", -10, 20));
  EXPECT_EQ(10, DecimalToI32("10", -10, 20));
  EXPECT_EQ(10, DecimalToI32("010", -10, 20));
  EXPECT_EQ(10, DecimalToI32("+10", -10, 20));
  EXPECT_EQ(10, DecimalToI32("+010", -10, 20));
  EXPECT_EQ(11, DecimalToI32("11", -10, 20));
  EXPECT_EQ(19, DecimalToI32("19", -10, 20));
  EXPECT_EQ(20, DecimalToI32("20", -10, 20));
  EXPECT_EQ(20, DecimalToI32("+20", -10, 20));
  EXPECT_EQ(20, DecimalToI32("+020", -10, 20));

  // valid values out of range
  EXPECT_THROW(DecimalToI32("-11", -10, 20), std::out_of_range);
  EXPECT_THROW(DecimalToI32("21", -10, 20),  std::out_of_range);

  // invalid values
  EXPECT_THROW(DecimalToI32("",       -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32(" ",      -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32(" 0",    -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0 ",    -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("--0",   -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("++0",   -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("X7",     -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("7X",     -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0x0",    -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0b0",    -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0X0",    -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("0B0",    -10, 20), std::invalid_argument);
  EXPECT_THROW(DecimalToI32("c",      -10, 20), std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyNumberToI32)
{
  // valid binary values within range
  EXPECT_EQ(-2147483648L, AnyNumberToI32("0b10000000000000000000000000000000"));
  EXPECT_EQ(-2147483647L, AnyNumberToI32("0b10000000000000000000000000000001"));
  EXPECT_EQ(-2147483648L, AnyNumberToI32("0b010000000000000000000000000000000"));
  EXPECT_EQ(-2147483647L, AnyNumberToI32("0b010000000000000000000000000000001"));
  EXPECT_EQ(-1,           AnyNumberToI32("0b11111111111111111111111111111111"));
  EXPECT_EQ(-1,           AnyNumberToI32("0b011111111111111111111111111111111"));
  EXPECT_EQ(0,            AnyNumberToI32("0b0"));
  EXPECT_EQ(1,            AnyNumberToI32("0b1"));
  EXPECT_EQ(1,            AnyNumberToI32("0b01"));
  EXPECT_EQ(2,            AnyNumberToI32("0b10"));
  EXPECT_EQ(11,           AnyNumberToI32("0b01011"));
  EXPECT_EQ(2147483647L,  AnyNumberToI32("0b01111111111111111111111111111111"));
  EXPECT_EQ(2147483647L,  AnyNumberToI32("0b001111111111111111111111111111111"));

  // valid binary values out of range
  EXPECT_THROW(AnyNumberToI32("0b100000000000000000000000000000000"), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("0b100000000000000000000000000000001"), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("0b111111111111111111111111111111111"), std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(-2147483648L, AnyNumberToI32("0x80000000"));
  EXPECT_EQ(-2147483647L, AnyNumberToI32("0x80000001"));
  EXPECT_EQ(-2147483648L, AnyNumberToI32("0x080000000"));
  EXPECT_EQ(-2147483647L, AnyNumberToI32("0x080000001"));
  EXPECT_EQ(-1,           AnyNumberToI32("0xFFFFFFFF"));
  EXPECT_EQ(-1,           AnyNumberToI32("0x0FFFFFFFF"));
  EXPECT_EQ(0,            AnyNumberToI32("0x0"));
  EXPECT_EQ(1,            AnyNumberToI32("0x1"));
  EXPECT_EQ(1,            AnyNumberToI32("0x01"));
  EXPECT_EQ(2,            AnyNumberToI32("0x2"));
  EXPECT_EQ(11,           AnyNumberToI32("0xB"));
  EXPECT_EQ(11,           AnyNumberToI32("0xb"));
  EXPECT_EQ(2147483647L,  AnyNumberToI32("0x7FFFFFFF"));
  EXPECT_EQ(2147483647L,  AnyNumberToI32("0x07FFFFFFF"));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyNumberToI32("0x100000000"), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("0x100000001"), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("0xFFFFFFFFF"), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(-2147483648L, AnyNumberToI32("-2147483648"));
  EXPECT_EQ(-2147483647L, AnyNumberToI32("-2147483647"));
  EXPECT_EQ(0U,           AnyNumberToI32("0"));
  EXPECT_EQ(0U,           AnyNumberToI32("+0"));
  EXPECT_EQ(0U,           AnyNumberToI32("-0"));
  EXPECT_EQ(1U,           AnyNumberToI32("1"));
  EXPECT_EQ(12U,          AnyNumberToI32("12"));
  EXPECT_EQ(12U,          AnyNumberToI32("+12"));
  EXPECT_EQ(2147483647L , AnyNumberToI32("2147483647"));
  EXPECT_EQ(2147483647L , AnyNumberToI32("+2147483647"));

  // valid decimal values out of range
  EXPECT_THROW(AnyNumberToI32("-2147483649"), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("2147483648"),  std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("+2147483648"), std::out_of_range);

  // invalid values
  EXPECT_THROW(AnyNumberToI32(""),      std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32(" "),     std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32("0XC"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00XC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32(" 0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0xC "),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("-0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("--0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("+0xC"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("++0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0x0xC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0x0XC"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0xG"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32("0B0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00B0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00b0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32(" 0b0"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0 "),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("-0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("--0b1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("+0b1"),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("++0b1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0x0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0X0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1x0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1X0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0c1"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b00b0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b00B0"), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b2"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32(" 0"),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0 "),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("--0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("++0"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("12x"),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32("x12"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("X12"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("b11"),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("B11"),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, AnyNumberToI32_minmax)
{
  // valid binary values within range
  EXPECT_EQ(-10, AnyNumberToI32("0b11111111111111111111111111110110", -10, 20));
  EXPECT_EQ(-1,  AnyNumberToI32("0b11111111111111111111111111111111", -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("0b00000000000000000000000000000000", -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("0b0",                                -10, 20));
  EXPECT_EQ(1,   AnyNumberToI32("0b1",                                -10, 20));
  EXPECT_EQ(1,   AnyNumberToI32("0b01",                               -10, 20));
  EXPECT_EQ(2,   AnyNumberToI32("0b10",                               -10, 20));
  EXPECT_EQ(19,  AnyNumberToI32("0b10011",                            -10, 20));
  EXPECT_EQ(20,  AnyNumberToI32("0b10100",                            -10, 20));

  // valid binary values out of range
  EXPECT_THROW(AnyNumberToI32("0b11111111111111111111111111110101", -10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("0b10101", -10, 20),                            std::out_of_range);

  // valid hexadecimal values within range
  EXPECT_EQ(-10, AnyNumberToI32("0xFFFFFFF6", -10, 20));
  EXPECT_EQ(-1,  AnyNumberToI32("0xFFFFFFFF", -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("0x00000000", -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("0x0",        -10, 20));
  EXPECT_EQ(19,  AnyNumberToI32("0x13",       -10, 20));
  EXPECT_EQ(20,  AnyNumberToI32("0x14",       -10, 20));

  // valid hexadecimal values out of range
  EXPECT_THROW(AnyNumberToI32("0xFFFFFFF5", -10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("0x15",       -10, 20), std::out_of_range);

  // valid decimal values within range
  EXPECT_EQ(-10, AnyNumberToI32("-10", -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("0",   -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("+0",  -10, 20));
  EXPECT_EQ(0,   AnyNumberToI32("-0",  -10, 20));
  EXPECT_EQ(1,   AnyNumberToI32("1",   -10, 20));
  EXPECT_EQ(12,  AnyNumberToI32("12",  -10, 20));
  EXPECT_EQ(12,  AnyNumberToI32("+12", -10, 20));
  EXPECT_EQ(19,  AnyNumberToI32("19",  -10, 20));
  EXPECT_EQ(20,  AnyNumberToI32("20",  -10, 20));

  // valid decimal values out of range
  EXPECT_THROW(AnyNumberToI32("-11", -10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("21",  -10, 20), std::out_of_range);
  EXPECT_THROW(AnyNumberToI32("+21", -10, 20), std::out_of_range);

  // invalid values
  EXPECT_THROW(AnyNumberToI32("", -10, 20),      std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32(" ", -10, 20),     std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32("0XC", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00XC", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00xC", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32(" 0xC", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0xC ", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("-0xC", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("--0xC", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("+0xC", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("++0xC", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0x0xC", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0x0XC", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0xG", -10, 20),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32("0B0", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00B0", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("00b0", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32(" 0b0", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0 ", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("-0b1", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("--0b1", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("+0b1", -10, 20),  std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("++0b1", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0b0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0B0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1b0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1B0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0x0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0X0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1x0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b1X0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b0c1", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b00b0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b00B0", -10, 20), std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0b2", -10, 20),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32(" 0", -10, 20),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("0 ", -10, 20),    std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("--0", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("++0", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("12x", -10, 20),   std::invalid_argument);

  EXPECT_THROW(AnyNumberToI32("x12", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("X12", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("b11", -10, 20),   std::invalid_argument);
  EXPECT_THROW(AnyNumberToI32("B11", -10, 20),   std::invalid_argument);
}

TEST(gpcc_string_tools_Tests, ToDouble)
{
  EXPECT_DOUBLE_EQ(0.0, ToDouble("0"));
  EXPECT_DOUBLE_EQ(100.0, ToDouble("1E2"));

  EXPECT_NO_THROW((void)ToDouble("0"));
  EXPECT_NO_THROW((void)ToDouble("+0"));
  EXPECT_NO_THROW((void)ToDouble("-0"));
  EXPECT_NO_THROW((void)ToDouble("0.0"));
  EXPECT_NO_THROW((void)ToDouble("+0.0"));
  EXPECT_NO_THROW((void)ToDouble("-0.0"));

  EXPECT_NO_THROW((void)ToDouble("1E1"));
  EXPECT_NO_THROW((void)ToDouble("1E+1"));
  EXPECT_NO_THROW((void)ToDouble("1E-1"));
  EXPECT_NO_THROW((void)ToDouble("1e1"));
  EXPECT_NO_THROW((void)ToDouble("1e+1"));
  EXPECT_NO_THROW((void)ToDouble("1e-1"));

  EXPECT_NO_THROW((void)ToDouble("+INF"));
  EXPECT_NO_THROW((void)ToDouble("-INF"));
  EXPECT_NO_THROW((void)ToDouble("INF"));
  EXPECT_NO_THROW((void)ToDouble("+inf"));
  EXPECT_NO_THROW((void)ToDouble("-inf"));
  EXPECT_NO_THROW((void)ToDouble("inf"));

  EXPECT_NO_THROW((void)ToDouble("NAN"));
  EXPECT_NO_THROW((void)ToDouble("NAN(0815)"));
  EXPECT_NO_THROW((void)ToDouble("nan"));
  EXPECT_NO_THROW((void)ToDouble("nan(0815)"));

  EXPECT_THROW((void)ToDouble(""),           std::invalid_argument);
  EXPECT_THROW((void)ToDouble(" "),          std::invalid_argument);
  EXPECT_THROW((void)ToDouble(" 0"),         std::invalid_argument);
  EXPECT_THROW((void)ToDouble("0 "),         std::invalid_argument);
  EXPECT_THROW((void)ToDouble("e"),          std::invalid_argument);
  EXPECT_THROW((void)ToDouble(" INF"),       std::invalid_argument);
  EXPECT_THROW((void)ToDouble("INF "),       std::invalid_argument);
  EXPECT_THROW((void)ToDouble(" NAN"),       std::invalid_argument);
  EXPECT_THROW((void)ToDouble("NAN "),       std::invalid_argument);
  EXPECT_THROW((void)ToDouble("NAN(0815) "), std::invalid_argument);
}

// Extraction and breakdown ---------------------------------------------------

TEST(gpcc_string_tools_Tests, ExtractFieldAndValue_DoxygenExamples)
{
  // This test checks the examples provided in the doxygen documentation of ExtractFieldAndValue_DoxygenExamples(...)
  std::string input;
  std::vector<std::pair<std::string,std::string>> result;
  std::vector<std::pair<std::string,std::string>> expect;

  input = "Name: \"Willy Black\" Age: 50";
  result = ExtractFieldAndValue(input, ' ', ':', '"');
  expect = {{"Name", "Willy Black"}, {"Age", "50"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Name: \"Willy Black\", Age: 50";
  result = ExtractFieldAndValue(input, ',', ':', '"');
  expect = {{"Name", "Willy Black"}, {"Age", "50"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Type=Potatoe; maxSize=12; maxWeight=3000";
  result = ExtractFieldAndValue(input, ';', '=', '"');
  expect = {{"Type", "Potatoe"}, {"maxSize", "12"}, {"maxWeight", "3000"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;
}

TEST(gpcc_string_tools_Tests, ExtractFieldAndValue)
{
  std::string input;
  std::vector<std::pair<std::string,std::string>> result;
  std::vector<std::pair<std::string,std::string>> expect;

  // ==========================================================================
  // ==========================================================================
  char sc = ' '; // <-- separating character
  char ac = '='; // <-- assignment character
  char qc = '"'; // <-- quotation character
  // ==========================================================================
  // ==========================================================================

  // empty input --------------------------------------------------------------
  input = "";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = " ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, space characters at different positions ------------------------
  input = "Field1=A";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1= A";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 =A";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = A";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1= \"A\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 =\"A\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = \"A\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, space characters inside quotation ------------------------------
  input = "Field1 = \" A \"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", " A "}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = \"A B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, assignment character within quoted section ---------------------
  input = "Field1 = \"A=B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A=B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, value empty ----------------------------------------------------
  input = "Field1=";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1= ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 =";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1=\"\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = \"\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, field empty ----------------------------------------------------
  input = "=Value1";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", "Value1"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = " =Value1";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", "Value1"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = " = Value1";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", "Value1"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "\"\"=Value1";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", "Value1"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "\"\" = Value1";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", "Value1"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, both field and value empty -------------------------------------
  input = "=";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = " = ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "\"\"=\"\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"", ""}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // one pair, malformed -----------------------------------------------------
  input = "Field1=A\"B\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "\"\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "\"Field1\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "Field1";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "Field1 Value1";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  // two pairs, spaces and quotation at different positions -------------------
  input = "Field1 = A Field2 = B";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = A Field2 = \"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = A \"Field 2\" = \"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field 2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1=A Field2=B";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1=A Field2=\"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = " Field1=A  Field2=\"A and B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // two pairs, empty values --------------------------------------------------
  input = "Field1=\"\" Field2=\"A and B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // two pairs, malformed -----------------------------------------------------
  input = "Field1= Field2=\"A\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "Field1=  Field2=\"A\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "Field1==A";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "Field1=\"A\"\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  // ==========================================================================
  // ==========================================================================
  sc = ',';
  ac = '=';
  qc = '"';
  // ==========================================================================
  // ==========================================================================

  // two pairs, spaces and quotation at different positions -------------------
  input = "Field1= A, Field2 = \"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 =A , Field2 =\"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1=\"A \" ,Field2= \"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A "}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1 = A,Field2=\"A and B\"";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", "A"}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // two pairs, empty values --------------------------------------------------
  input = "Field1=\"\", Field2=\"A and B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1= , Field2=\"A and B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1=, Field2=\"A and B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  input = "Field1=,Field2=\"A and B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}, {"Field2", "A and B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // two pairs, assignment character within quoted section --------------------
  input = "Field1=,Field2=\"A,B\" ";
  result = ExtractFieldAndValue(input, sc, ac, qc);
  expect = {{"Field1", ""}, {"Field2", "A,B"}};
  EXPECT_EQ(result, expect) << "Failed. Input was: " << input;

  // two pairs, malformed -----------------------------------------------------
  input = "Field1 = A,, Field2 = \"A and B\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = "Field1 = A, Field2 = \"A and B\",";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;

  input = ",Field1 = A, Field2 = \"A and B\"";
  EXPECT_THROW(result = ExtractFieldAndValue(input, sc, ac, qc), std::invalid_argument) << "Failed. Input was: " << input;
}

// Composition ----------------------------------------------------------------

TEST(gpcc_string_tools_Tests, VASPrintf_ASPrintf)
{
  // White-box: ASPrintf() delegates all work to VASPrintf(...)

  std::unique_ptr<char[]> spStr;

  // Test: One arg
  spStr = ASPrintf("Test %u", static_cast<unsigned int>(5U));
  ASSERT_TRUE(spStr != nullptr);
  EXPECT_STREQ(spStr.get(), "Test 5");

  // Test: Arg not used
  spStr = ASPrintf("Test", static_cast<unsigned int>(5U));
  ASSERT_TRUE(spStr != nullptr);
  EXPECT_STREQ(spStr.get(), "Test");

  // Test: Empty string
  spStr = ASPrintf("", static_cast<unsigned int>(5U));
  ASSERT_TRUE(spStr != nullptr);
  EXPECT_STREQ(spStr.get(), "");

  // Test: Invalid arg
  EXPECT_THROW(spStr = ASPrintf(nullptr, static_cast<unsigned int>(5U)), std::invalid_argument);
}

} // namespace string
} // namespace gpcc_tests
