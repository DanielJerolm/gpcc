/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc_test/cli/FakeTerminal.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include <gtest/gtest.h>
#include <cstring>

namespace gpcc_tests {
namespace cli        {

using namespace testing;

// Test fixture for FakeTerminal unit tests
class gpcc_cli_FakeTerminal_TestsF: public Test
{
  public:
    gpcc_cli_FakeTerminal_TestsF(void);

  protected:
    enum class EraseMode
    {
      BeginToCursor,
      CursorToEnd,
      WholeLine
    };

    FakeTerminal uut;
    gpcc::cli::ITerminal & uut_ITerminal;

    void PrintText(char const * s);
    void MoveCursorVertical(int16_t delta);
    void MoveCursorHorizontal(int16_t delta);
    void DeleteCharacters(uint8_t n);
    void EraseCharacters(EraseMode const mode);

    void SetUp(void) override;
    void TearDown(void) override;
};

gpcc_cli_FakeTerminal_TestsF::gpcc_cli_FakeTerminal_TestsF(void)
: Test()
, uut(80, 8)
, uut_ITerminal(uut)
{
}

void gpcc_cli_FakeTerminal_TestsF::SetUp(void)
{
}

void gpcc_cli_FakeTerminal_TestsF::TearDown(void)
{
}

// alias for death tests
using gpcc_cli_FakeTerminal_DeathTestsF = gpcc_cli_FakeTerminal_TestsF;

void gpcc_cli_FakeTerminal_TestsF::PrintText(char const * s)
{
  uut_ITerminal.Write(s, strlen(s));
}

void gpcc_cli_FakeTerminal_TestsF::MoveCursorVertical(int16_t delta)
{
  if (delta == 0)
    return;

  std::string cmd;
  cmd = "\x1B[";

  if (delta < 0)
  {
    if (delta < -99)
      throw std::invalid_argument("gpcc_cli_FakeTerminal_TestsF::MoveCursorHorizontal: delta");
    delta = -delta;

    if (delta >= 10)
      cmd += '0' + (delta / 10);
    cmd += '0' + (delta % 10);
    cmd += 'A';
  }
  else
  {
    throw std::invalid_argument("gpcc_cli_FakeTerminal_TestsF::MoveCursorVertical: UUT only supports moving cursor up");
  }

  uut_ITerminal.Write(cmd.c_str(), cmd.length());
}

void gpcc_cli_FakeTerminal_TestsF::MoveCursorHorizontal(int16_t delta)
{
  // Moves the cursor delta characters to the right. Negative values move to the left.

  if (delta == 0)
    return;

  std::string cmd;
  cmd = "\x1B[";

  if (delta < 0)
  {
    if (delta < -99)
      throw std::invalid_argument("gpcc_cli_FakeTerminal_TestsF::MoveCursorHorizontal: delta");
    delta = -delta;

    if (delta >= 10)
      cmd += '0' + (delta / 10);
    cmd += '0' + (delta % 10);
    cmd += 'D';
  }
  else
  {
    if (delta > 99)
      throw std::invalid_argument("gpcc_cli_FakeTerminal_TestsF::MoveCursorHorizontal: delta");

    if (delta >= 10)
      cmd += '0' + (delta / 10);
    cmd += '0' + (delta % 10);
    cmd += 'C';
  }

  uut_ITerminal.Write(cmd.c_str(), cmd.length());
}

void gpcc_cli_FakeTerminal_TestsF::DeleteCharacters(uint8_t n)
{
  // Deletes n characters starting at current cursor position

  if (n == 0)
    return;

  if (n > 99)
    throw std::invalid_argument("gpcc_cli_FakeTerminal_TestsF::DeleteCharacters: n");

  std::string cmd;
  cmd = "\x1B[";
  if (n >= 10)
    cmd += '0' + (n / 10U);
  cmd += '0' + (n % 10U);
  cmd += 'P';

  uut_ITerminal.Write(cmd.c_str(), cmd.length());
}

void gpcc_cli_FakeTerminal_TestsF::EraseCharacters(EraseMode const mode)
{
  // Erase part of the current line or the whole line according to "mode".

  std::string cmd;
  cmd = "\x1B[";

  switch (mode)
  {
    case EraseMode::BeginToCursor:
      cmd += '1';
      break;

    case EraseMode::CursorToEnd:
      cmd += '0';
      break;

    case EraseMode::WholeLine:
      cmd += '2';
      break;
  }
  cmd += 'K';

  uut_ITerminal.Write(cmd.c_str(), cmd.length());
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Create)
{
  char const * ref[8] =
  {
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print1)
{
  char const * ref[8] =
  {
   "Hello World!",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("Hello World!");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(12,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print2)
{
  char const * ref[8] =
  {
   "Hello World!",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("Hello World!\n");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print3)
{
  char const * ref[8] =
  {
   "Hello World!",
   "Second Line",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("Hello World!\nSecond Line");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(11,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print4)
{
  char const * ref[8] =
  {
   "Hello World!",
   "Second Line",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("Hello World!\nSecond Line\n");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,2));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print5)
{
  char const * ref[8] =
  {
   "Hello World!",
   "",
   "Third Line",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("Hello World!\n\nThird Line");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(10,2));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print6)
{
  char const * ref[8] =
  {
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print7)
{
  char const * ref[8] =
  {
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("\n");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print8)
{
  char const * ref[8] =
  {
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("\n\n");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,2));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print9)
{
  char const * ref[8] =
  {
   "Hello World!",
   "Line 2",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("Hello World!");
  MoveCursorHorizontal(-7);
  PrintText("\nLine 2");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(6,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_78chars)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "012345678901234567890123456789012345678901234567890123456789012345678901234567",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("012345678901234567890123456789012345678901234567890123456789012345678901234567");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(78,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_79chars)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "0123456789012345678901234567890123456789012345678901234567890123456789012345678",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(79,0));
}

TEST_F(gpcc_cli_FakeTerminal_DeathTestsF, Print_80chars_RejectExpected)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  char const * ref[8] =
   {
 //  0         1         2         3         4         5         6         7         8
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
   };

   PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678");
   EXPECT_DEATH(PrintText("9"), ".*UUT attempted write to last character of line.*");
   ASSERT_TRUE(uut.Compare(ref));
   ASSERT_TRUE(uut.Compare(79,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_78chars_plus2ndLine)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "012345678901234567890123456789012345678901234567890123456789012345678901234567",
   "Line2",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("012345678901234567890123456789012345678901234567890123456789012345678901234567\nLine2");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(5,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_79chars_plus2ndLine)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "0123456789012345678901234567890123456789012345678901234567890123456789012345678",
   "Line2",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678\nLine2");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(5,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_79chars_plus3newLines)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "0123456789012345678901234567890123456789012345678901234567890123456789012345678",
   "",
   "ABC",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678\n\nABC\n");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,3));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_VerticalScroll1)
{
  char const * ref[8] =
  {
   "Line 1",
   "Line 2",
   "Line 3",
   "Line 4",
   "Line 5",
   "Line 6",
   "Line 7",
   "Line 8"
  };

  PrintText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(6,7));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_VerticalScroll2)
{
  char const * ref[8] =
  {
   "Line 2",
   "Line 3",
   "Line 4",
   "Line 5",
   "Line 6",
   "Line 7",
   "Line 8",
   ""
  };

  PrintText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\n");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,7));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_VerticalScroll3)
{
  char const * ref[8] =
  {
   "Line 2",
   "Line 3",
   "Line 4",
   "Line 5",
   "Line 6",
   "Line 7",
   "Line 8",
   "Line 9"
  };

  PrintText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(6,7));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorVerticalMove1)
{
  char const * ref[8] =
  {
 // 0123456789012
   "ABC1",
   "ABC2DEF",
   "ABC3",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABC1\nABC2\nABC3");
  MoveCursorVertical(-1);
  PrintText("DEF");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(7,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorVerticalMove2)
{
  char const * ref[8] =
  {
 // 0123456789012
   "ABC1DEF",
   "ABC2",
   "ABC3",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABC1\nABC2\nABC3");
  MoveCursorVertical(-2);
  PrintText("DEF");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(7,0));
}

TEST_F(gpcc_cli_FakeTerminal_DeathTestsF, CursorVerticalMove_BeyondTop)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  char const * ref[8] =
  {
 // 0123456789012
   "ABC1",
   "ABC2",
   "ABC3",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABC1\nABC2\nABC3");
  EXPECT_DEATH(MoveCursorVertical(-3), ".*UUT attempt to move cursor up beyond line 0.*");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(4,2));
}

TEST_F(gpcc_cli_FakeTerminal_DeathTestsF, CursorVerticalMove_BeyondTopFromFirstLine)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  char const * ref[8] =
  {
 // 0123456789012
   "ABC1",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABC1");
  EXPECT_DEATH(MoveCursorVertical(-1), ".*UUT attempt to move cursor up, but y is already zero.*");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(4,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove1)
{
  char const * ref[8] =
  {
 // 0123456789012
   "AB55EFGHXXKLM",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEFGHIJKLM");
  MoveCursorHorizontal(-11);
  PrintText("55");
  MoveCursorHorizontal(4);
  PrintText("XX");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(10,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove2)
{
  char const * ref[8] =
  {
 // 012345678901234567890123456789
   "AB55EFGHIJKLMNOPXXSTUVWXYZ",
   "Line2",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  MoveCursorHorizontal(-20);
  MoveCursorHorizontal(-4);
  PrintText("55");
  MoveCursorHorizontal(12);
  PrintText("XX\nLine2");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(5,1));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove_BeyondLeftEnd)
{
  char const * ref[8] =
  {
 // 0123456789
   "XXCDEF",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEF");
  MoveCursorHorizontal(-20);
  PrintText("XX");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(2,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove_BehindLastChar)
{
  char const * ref[8] =
  {
 // 0123456789
   "XXCDEF",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEF");
  MoveCursorHorizontal(-20);
  PrintText("XX");
  MoveCursorHorizontal(4);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(6,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove_BeyondLastCharAndPrint)
{
  char const * ref[8] =
  {
 // 01234567890123456789
   "XXCDEF    TEST",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEF");
  MoveCursorHorizontal(-20);
  PrintText("XX");
  MoveCursorHorizontal(8);
  PrintText("TEST");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(14,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove_MaxLineLengthToLastCharAndPrint)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "0123456789012345678901234567890123456789012345678901234567890123456789012XX567A",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678");
  MoveCursorHorizontal(-6);
  PrintText("XX");
  MoveCursorHorizontal(3);
  PrintText("A");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(79,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, CursorHorizontalMove_MaxLineLengthToBehindLastChar)
{
  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "0123456789012345678901234567890123456789012345678901234567890123456789012XX5678",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678");
  MoveCursorHorizontal(-6);
  PrintText("XX");
  MoveCursorHorizontal(4);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(79,0));
}

TEST_F(gpcc_cli_FakeTerminal_DeathTestsF, CursorHorizontalMove_MaxLineLengthToBeyondLastChar_RejectExpected)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  char const * ref[8] =
  {
//  0         1         2         3         4         5         6         7         8
   "0123456789012345678901234567890123456789012345678901234567890123456789012XX5678",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789012345678901234567890123456789012345678901234567890123456789012345678");
  MoveCursorHorizontal(-6);
  PrintText("XX");
  EXPECT_DEATH(MoveCursorHorizontal(5), ".*UUT attempted to move cursor beyond width of terminal.*");
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(75,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, DeleteChars)
{
  char const * ref[8] =
  {
 // 0123456789
   "ABEFG",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEFG");
  MoveCursorHorizontal(-5);
  DeleteCharacters(2);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(2,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, DeleteChars_RestOfLine)
{
  char const * ref[8] =
  {
 // 0123456789
   "AB",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEFG");
  MoveCursorHorizontal(-5);
  DeleteCharacters(5);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(2,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, DeleteChars_MoreThanRestOfLine)
{
  char const * ref[8] =
  {
 // 0123456789
   "AB",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("ABCDEFG");
  MoveCursorHorizontal(-5);
  DeleteCharacters(6);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(2,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_BeginToCursor_X0)
{
  char const * ref[8] =
  {
 // 0123456789
   " 123456789",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-10);
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_BeginToCursor_X1)
{
  char const * ref[8] =
  {
 // 0123456789
   "  23456789",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-9);
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(1,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_BeginToCursor_EOLminus2)
{
  char const * ref[8] =
  {
 // 0123456789
   "         9",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-2);
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(8,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_BeginToCursor_EOLminus1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-1);
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(9,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_BeginToCursor_EOL)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(10,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_BeginToCursor_EOLplus1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(1);
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(11,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_CursorToEnd_X0)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-10);
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_CursorToEnd_X1)
{
  char const * ref[8] =
  {
 // 0123456789
   "0",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-9);
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(1,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_CursorToEnd_EOLminus2)
{
  char const * ref[8] =
  {
 // 0123456789
   "01234567",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-2);
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(8,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_CursorToEnd_EOLminus1)
{
  char const * ref[8] =
  {
 // 0123456789
   "012345678",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-1);
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(9,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_CursorToEnd_EOL)
{
  char const * ref[8] =
  {
 // 0123456789
   "0123456789",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(10,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_CursorToEnd_EOLplus1)
{
  char const * ref[8] =
  {
 // 0123456789
   "0123456789",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(1);
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(11,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_WholeLine_X0)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-10);
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_WholeLine_X1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-9);
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(1,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_WholeLine_EOLminus2)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-2);
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(8,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_WholeLine_EOLminus1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(-1);
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(9,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_WholeLine_EOL)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(10,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_WholeLine_EOLplus1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  PrintText("0123456789");
  MoveCursorHorizontal(1);
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(11,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_EmptyLine_BeginToCursor_X0)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_EmptyLine_BeginToCursor_X1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  MoveCursorHorizontal(1);
  EraseCharacters(EraseMode::BeginToCursor);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(1,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_EmptyLine_CursorToEnd_X0)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_EmptyLine_CursorToEnd_X1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  MoveCursorHorizontal(1);
  EraseCharacters(EraseMode::CursorToEnd);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(1,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_EmptyLine_WholeLine_X0)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(0,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, EraseChars_EmptyLine_WholeLine_X1)
{
  char const * ref[8] =
  {
 // 0123456789
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  MoveCursorHorizontal(1);
  EraseCharacters(EraseMode::WholeLine);
  ASSERT_TRUE(uut.Compare(ref));
  ASSERT_TRUE(uut.Compare(1,0));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_TimeoutNoData)
{
  char buffer[16];

  gpcc::time::TimePoint start = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 500);
  gpcc::time::TimePoint end = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);

  ASSERT_EQ(0U, retVal);
  ASSERT_TRUE((end - start).ms() >= 500);

#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE((end - start).ms() < 600);
#endif
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_TimeoutWithData)
{
  char buffer[16];

  uut.Input("A");

  gpcc::time::TimePoint start = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 500);
  gpcc::time::TimePoint end = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);

  ASSERT_EQ(1U, retVal);
  ASSERT_EQ('A', buffer[0]);

#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE((end - start).ms() < 100);
#endif
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_ZeroTimeoutNoData)
{
  char buffer[16];

  gpcc::time::TimePoint start = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 0);
  gpcc::time::TimePoint end = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);

  ASSERT_EQ(0U, retVal);
#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE((end - start).ms() < 100);
#endif
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_ZeroTimeoutWithData)
{
  char buffer[16];

  uut.Input("A");

  gpcc::time::TimePoint start = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 0);
  gpcc::time::TimePoint end = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonicPrecise);

  ASSERT_EQ(1U, retVal);
  ASSERT_EQ('A', buffer[0]);

#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE((end - start).ms() < 100);
#endif
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_AllInputMethods)
{
  uut.Input("Test");
  uut.Input_POS1();
  uut.Input_END();
  uut.Input_ENTER();
  uut.Input_DEL(1);
  uut.Input_Backspace(1);
  uut.Input_TAB(1);
  uut.Input_ArrowLeft(1);
  uut.Input_ArrowRight(1);
  uut.Input_ArrowUp(1);
  uut.Input_ArrowDown(1);
  uut.Input_CtrlC();

  char buffer[64];
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 1000);

  char const expected[] =
  {
    'T', 'e', 's', 't',
    0x1B, '[', '1', '~',
    0x1B, '[', '4', '~',
    0x0D,
    0x1B, '[', '3', '~',
    0x7F,
    0x09,
    0x1B, '[', 'D',
    0x1B, '[', 'C',
    0x1B, '[', 'A',
    0x1B, '[', 'B',
    0x03
  };

  ASSERT_EQ(sizeof(expected), retVal);
  ASSERT_TRUE(memcmp(buffer, expected, retVal) == 0);

  retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 1000);
  ASSERT_EQ(0U, retVal);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_InputBufferEmptyAfterRead)
{
  uut.Input("Test");

  char buffer[64];
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);

  char const expected[] =
  {
    'T', 'e', 's', 't'
  };

  ASSERT_EQ(sizeof(expected), retVal);
  ASSERT_TRUE(memcmp(buffer, expected, retVal) == 0);

  // Perform a second read. No data must be read
  retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);
  ASSERT_EQ(0U, retVal);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Flush_InputBufferEmpty)
{
  static_cast<gpcc::cli::ITerminal&>(uut).Flush();

  char buffer[16];
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);
  ASSERT_EQ(0U, retVal);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Flush_InputBufferNotEmpty)
{
  uut.Input("Test");
  static_cast<gpcc::cli::ITerminal&>(uut).Flush();

  char buffer[16];
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);
  ASSERT_EQ(0U, retVal);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Flush_InputAfterFlush)
{
  uut.Input("Test");
  static_cast<gpcc::cli::ITerminal&>(uut).Flush();
  uut.Input("A");

  char buffer[16];
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);
  ASSERT_EQ(1U, retVal);
  ASSERT_EQ('A', buffer[0]);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Read_RequestThrow)
{
  uut.RequestThrowUponRead();

  char buffer[16];
  size_t retVal;
  ASSERT_THROW(retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100), std::runtime_error);

  retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);
  ASSERT_EQ(0U, retVal);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Print_RequestThrow)
{
  uut.RequestThrowUponWrite();

  ASSERT_THROW(PrintText("Hello World!\n"), std::runtime_error);

  PrintText("Second attempt...\n");

  char const * ref[8] =
  {
   "Second attempt...",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
  };

  ASSERT_TRUE(uut.Compare(ref));
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, Flush_RequestThrow)
{
  uut.RequestThrowUponFlush();

  uut.Input("Test");
  ASSERT_THROW(static_cast<gpcc::cli::ITerminal&>(uut).Flush(), std::exception);

  char buffer[16];
  size_t retVal = static_cast<gpcc::cli::ITerminal&>(uut).Read(buffer, sizeof(buffer), 100);
  ASSERT_EQ(4U, retVal);
  ASSERT_EQ('T', buffer[0]);
  ASSERT_EQ('e', buffer[1]);
  ASSERT_EQ('s', buffer[2]);
  ASSERT_EQ('t', buffer[3]);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, GetScreenContent_NoPrintEver)
{
  ASSERT_TRUE(uut.GetScreenContent() == "\n\n\n\n\n\n\n\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, GetScreenContent_Print1)
{
  PrintText("Hello World!");

  ASSERT_TRUE(uut.GetScreenContent() == "Hello World!\n\n\n\n\n\n\n\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, GetScreenContent_Print2)
{
  PrintText("Hello World!\n");
  PrintText("Line 2");

  ASSERT_TRUE(uut.GetScreenContent() == "Hello World!\nLine 2\n\n\n\n\n\n\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, GetScreenContent_PrintBlanks)
{
  PrintText("   ");

  ASSERT_TRUE(uut.GetScreenContent() == "   \n\n\n\n\n\n\n\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, RecordDroppedOutLines_NotEnabled)
{
  ASSERT_THROW(uut.GetDroppedOutLinesPlusCurrentScreenContent(), std::logic_error);
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, RecordDroppedOutLines_NoPrintEver)
{
  uut.EnableRecordingOfDroppedOutLines();

  EXPECT_TRUE(uut.GetScreenContent() == "\n\n\n\n\n\n\n\n");
  EXPECT_TRUE(uut.GetDroppedOutLinesPlusCurrentScreenContent() == "\n\n\n\n\n\n\n\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, RecordDroppedOutLines_Print1)
{
  uut.EnableRecordingOfDroppedOutLines();

  PrintText("Hello World!");

  EXPECT_TRUE(uut.GetScreenContent() == "Hello World!\n\n\n\n\n\n\n\n");
  EXPECT_TRUE(uut.GetDroppedOutLinesPlusCurrentScreenContent() == "Hello World!\n\n\n\n\n\n\n\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, RecordDroppedOutLines_ScreenFull)
{
  uut.EnableRecordingOfDroppedOutLines();

  PrintText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8");

  EXPECT_TRUE(uut.GetScreenContent() == "Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\n");
  EXPECT_TRUE(uut.GetDroppedOutLinesPlusCurrentScreenContent() == "Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\n");
}

TEST_F(gpcc_cli_FakeTerminal_TestsF, RecordDroppedOutLines_OneLineDroppedOut)
{
  uut.EnableRecordingOfDroppedOutLines();

  PrintText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9");

  EXPECT_TRUE(uut.GetScreenContent() == "Line 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\n");
  EXPECT_TRUE(uut.GetDroppedOutLinesPlusCurrentScreenContent() == "Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\n");
}

} // namespace cli
} // namespace gpcc_tests
