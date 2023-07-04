/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "src/cli/internal/TerminalRxParser.hpp"
#include <gtest/gtest.h>

namespace gpcc_tests {
namespace cli        {
namespace internal   {

using namespace testing;
using gpcc::cli::internal::TerminalRxParser;

TEST(gpcc_cli_internal_TerminalRxParser_Tests, Instantiation)
{
  TerminalRxParser uut;
  ASSERT_EQ(0U, uut.GetLevel());
}

TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_Backspace)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::Backspace, uut.Input(0x7F));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_Tab)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::Tab, uut.Input(0x09));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_LF)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::LF, uut.Input(0x0A));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_CR)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::CR, uut.Input(0x0D));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_ArrowLeft)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut.Input('D'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_ArrowRight)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::ArrowRight, uut.Input('C'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_ArrowUp)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::ArrowUp, uut.Input('A'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_ArrowDown)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::ArrowDown, uut.Input('B'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_Pos1)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('1'));
  ASSERT_EQ(TerminalRxParser::Result::Pos1, uut.Input('~'));

  uut.Clear();

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::Pos1, uut.Input('H'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_End)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('4'));
  ASSERT_EQ(TerminalRxParser::Result::END, uut.Input('~'));


  uut.Clear();

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::END, uut.Input('F'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_DEL)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('3'));
  ASSERT_EQ(TerminalRxParser::Result::DEL, uut.Input('~'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_PgUp)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('5'));
  ASSERT_EQ(TerminalRxParser::Result::PgUp, uut.Input('~'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_PgDn)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('6'));
  ASSERT_EQ(TerminalRxParser::Result::PgDn, uut.Input('~'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_ETX)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::ETX, uut.Input(0x03));
}

TEST(gpcc_cli_internal_TerminalRxParser_Tests, Clear)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('6'));
  uut.Clear();
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('~'));

  char const * const pBuffer = uut.Output();
  ASSERT_EQ('~',  pBuffer[0]);
  ASSERT_EQ(0x00, pBuffer[1]);
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_NoCommand1)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('X'));

  char const * const pBuffer = uut.Output();
  ASSERT_EQ(0x1B, pBuffer[0]);
  ASSERT_EQ('[',  pBuffer[1]);
  ASSERT_EQ('X',  pBuffer[2]);
  ASSERT_EQ(0x00, pBuffer[3]);
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, Input_NoCommand2)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('A'));

  char const * const pBuffer = uut.Output();
  ASSERT_EQ('A',  pBuffer[0]);
  ASSERT_EQ(0x00, pBuffer[1]);
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, CallToOutputButEmpty)
{
  TerminalRxParser uut;

  char const * const pBuffer = uut.Output();
  ASSERT_EQ(0x00, pBuffer[0]);
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, DoubleCallToOutput)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('A'));

  char const * pBuffer = uut.Output();
  ASSERT_EQ('A',  pBuffer[0]);
  ASSERT_EQ(0x00, pBuffer[1]);

  pBuffer = uut.Output();
  ASSERT_EQ(0x00, pBuffer[0]);
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, GetLevel)
{
  TerminalRxParser uut;

  ASSERT_EQ(0U, uut.GetLevel());
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(1U, uut.GetLevel());
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(2U, uut.GetLevel());
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('6'));
  ASSERT_EQ(3U, uut.GetLevel());
  ASSERT_EQ(TerminalRxParser::Result::PgDn, uut.Input('~'));
  ASSERT_EQ(4U, uut.GetLevel());
  uut.Clear();
  ASSERT_EQ(0U, uut.GetLevel());
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('A'));
  ASSERT_EQ(1U, uut.GetLevel());
  char const * const pBuffer = uut.Output();
  ASSERT_EQ('A',  pBuffer[0]);
  ASSERT_EQ(0x00, pBuffer[1]);
  ASSERT_EQ(0U, uut.GetLevel());
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, RemoveNonPrintableCharacters)
{
  TerminalRxParser uut;
  char const * pBuffer;

  // -- single printable character --
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('A'));
  uut.RemoveNonPrintableCharacters();
  ASSERT_EQ(1U, uut.GetLevel());
  pBuffer = uut.Output();
  ASSERT_EQ('A',  pBuffer[0]);
  ASSERT_EQ(0x00, pBuffer[1]);

  // -- single non-printable character --
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input(0x15)); // (NAK)
  uut.RemoveNonPrintableCharacters();
  ASSERT_EQ(0U, uut.GetLevel());
  pBuffer = uut.Output();
  ASSERT_EQ(0x00, pBuffer[0]);

  // -- removal of non-printable character (not at end of sequence) --
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input('X'));
  uut.RemoveNonPrintableCharacters();
  ASSERT_EQ(2U, uut.GetLevel());
  pBuffer = uut.Output();
  ASSERT_EQ('[',  pBuffer[0]);
  ASSERT_EQ('X',  pBuffer[1]);
  ASSERT_EQ(0x00, pBuffer[2]);

  // -- non-printable character at end of sequence (not removed) --
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('6'));
  ASSERT_EQ(TerminalRxParser::Result::NoCommand, uut.Input(0x1B));
  uut.RemoveNonPrintableCharacters();
  ASSERT_EQ(3U, uut.GetLevel());
  pBuffer = uut.Output();
  ASSERT_EQ('[',  pBuffer[0]);
  ASSERT_EQ('6',  pBuffer[1]);
  ASSERT_EQ(0x1B, pBuffer[2]);
  ASSERT_EQ(0x00, pBuffer[3]);
}

TEST(gpcc_cli_internal_TerminalRxParser_Tests, CopyConstruction)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));

  TerminalRxParser uut2(uut);
  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut.Input('D'));
  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut2.Input('D'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, MoveConstruction)
{
  TerminalRxParser uut;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));

  TerminalRxParser uut2(std::move(uut));
  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut2.Input('D'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, CopyAssignment)
{
  TerminalRxParser uut;
  TerminalRxParser uut2;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut2.Input(0x1B));
  uut2 = uut;

  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut.Input('D'));
  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut2.Input('D'));
}
TEST(gpcc_cli_internal_TerminalRxParser_Tests, MoveAssignment)
{
  TerminalRxParser uut;
  TerminalRxParser uut2;

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input(0x1B));
  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut.Input('['));

  ASSERT_EQ(TerminalRxParser::Result::NeedMoreData, uut2.Input(0x1B));
  uut2 = std::move(uut);

  ASSERT_EQ(TerminalRxParser::Result::ArrowLeft, uut2.Input('D'));
}

} // namespace internal
} // namespace cli
} // namespace gpcc_tests
