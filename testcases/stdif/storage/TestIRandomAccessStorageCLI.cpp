/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/stdif/storage/IRandomAccessStorageCLI.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/Command.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/stdif/storage/IRandomAccessStorage.hpp>
#include "testcases/fakes/cli/FakeTerminal.hpp"
#include "testcases/file_systems/eeprom_section_system/FakeEEPROM.hpp"
#include "gtest/gtest.h"
#include <functional>
#include <iostream>
#include <cstddef>
#include <cstdint>

namespace gpcc_tests
{
namespace stdif
{

class GPCC_stdif_IRandomAccessStorageCLI_Tests: public testing::Test
{
  public:
    static const size_t storageSize     = 4U * 1024UL;
    static const size_t storagePageSize = 32U;

    GPCC_stdif_IRandomAccessStorageCLI_Tests(void);
    virtual ~GPCC_stdif_IRandomAccessStorageCLI_Tests(void);

  protected:
    file_systems::eeprom_section_system::FakeEEPROM fakeStorage;
    cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliNeedsStop;

    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

GPCC_stdif_IRandomAccessStorageCLI_Tests::GPCC_stdif_IRandomAccessStorageCLI_Tests(void)
: Test()
, fakeStorage(storageSize, storagePageSize)
, terminal(80U, 8U)
, cli(terminal, 80U, 8U, "CLI", nullptr)
, cliNeedsStop(false)
{
}

GPCC_stdif_IRandomAccessStorageCLI_Tests::~GPCC_stdif_IRandomAccessStorageCLI_Tests(void)
{
}

void GPCC_stdif_IRandomAccessStorageCLI_Tests::SetUp(void)
{
  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliNeedsStop = true;

  terminal.WaitForInputProcessed();

  cli.AddCommand(gpcc::cli::Command::Create("ReadRAS", "\nHelp text",
                                            std::bind(&gpcc::stdif::CliCmdReadIRandomAccessStorage,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &fakeStorage)));
  cli.AddCommand(gpcc::cli::Command::Create("WriteRAS", "\nHelp text",
                                            std::bind(&gpcc::stdif::CliCmdWriteIRandomAccessStorage,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &fakeStorage)));
}

void GPCC_stdif_IRandomAccessStorageCLI_Tests::TearDown(void)
{
  if (cliNeedsStop)
    cli.Stop();

  if (HasFailure())
  {
    std::cout << "*****************************************************" << std::endl
              << "Content of fake terminal's screen" << std::endl
              << "*****************************************************" << std::endl;
    std::cout << terminal.GetScreenContent() << std::endl;
  }
}

void GPCC_stdif_IRandomAccessStorageCLI_Tests::Login(void)
{
  terminal.Input("login");

  for (int i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, InstantiationAndLogin)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">",
   "",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_WrongNbOfParams0)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_WrongNbOfParams1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_WrongNbOfParams3)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 0 0",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 0 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_AddressNotHex)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0 0",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"0\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_AddressBadChars)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xXYZ 0",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"0xXYZ\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0xXYZ 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_NbOfBytesNegative)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 -1",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Value '-1' is out of range [0;1024]",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 -1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_NbOfBytesBadChars)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 XYZ",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"XYZ\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 XYZ");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_NbOfBytesTooLarge)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 1025",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Value '1025' is out of range [0;1024]",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 1025");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_AddressOutOf32Bit)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFFFFFF0 17",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0xFFFFFFF0 17");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_AddressIn32BitButOutOfBounds)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFFFFFF0 16",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0xFFFFFFF0 16");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_ZeroBytes)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x100 0",
   ">",
   "",
   "",
   "",
   ""
  };

  uint8_t const data[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x13, 0x14, 0x15 };
  fakeStorage.Write(0x100, 8, data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x100 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_OneByte)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x100 1",
   "Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF",
   "0x00000100: DE                                              .",
   ">",
   "",
   ""
  };

  uint8_t const data[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x13, 0x14, 0x15 };
  fakeStorage.Write(0x100, 8, data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x100 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_12Byte)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x100 12",
   "Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF",
   "0x00000100: DE AD BE EF 12 13 14 15 00 00 00 00             ............",
   ">",
   "",
   ""
  };

  uint8_t const data[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x13, 0x14, 0x15 };
  fakeStorage.Write(0x100, 8, data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x100 12");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_17Byte)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x100 17",
   "Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF",
   "0x00000100: DE AD BE EF 12 13 14 15 16 17 18 19 01 02 03 04 ................",
   "0x00000110: FF                                              .",
   ">",
   ""
  };

  uint8_t const data[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x01, 0x02, 0x03, 0x04, 0xFF };
  fakeStorage.Write(0x100, sizeof(data), data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x100 17");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_UnalignedAddress17Bytes)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x101 17",
   "Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF",
   "0x00000101: AD BE EF 12 13 14 15 16 17 18 19 01 02 03 04 FF ................",
   "0x00000111: 00                                              .",
   ">",
   ""
  };

  uint8_t const data[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x01, 0x02, 0x03, 0x04, 0xFF };
  fakeStorage.Write(0x100, sizeof(data), data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x101 17");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_LastByteOfStorage)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFF 1",
   "Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF",
   "0x00000FFF: EE                                              .",
   ">",
   "",
   ""
  };

  uint8_t const data[] = { 0xEE };
  fakeStorage.Write(0xFFF, sizeof(data), data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0xFFF 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_BeyondEndOfStorage1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFF 2",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  uint8_t const data[] = { 0xEE };
  fakeStorage.Write(0xFFF, sizeof(data), data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0xFFF 2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Read_BeyondEndOfStorage2)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x1000 1",
   "",
   "Invalid arguments. Try 'ReadRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  uint8_t const data[] = { 0xEE };
  fakeStorage.Write(0xFFF, sizeof(data), data);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x1000 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_WrongNbOfParams0)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_WrongNbOfParams1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x0",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_AddressNotHex)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0 0",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"0\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_AddressBadChars)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xXYZ 0",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"0xXYZ\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0xXYZ 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_AddressOutOf32Bit)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFFFFFFF 0 0",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0xFFFFFFFF 0 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_AddressIn32BitButOutOfBounds)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFFFFFFF 0",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0xFFFFFFFF 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_OneByte)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 5",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);

  uint8_t data[1];
  uint8_t const expectedData[1] = {0x05};
  fakeStorage.Read(0x100, sizeof(data), data);

  ASSERT_TRUE(memcmp(data, expectedData, sizeof(data)) == 0);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_10Byte)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 5 6 7 8 9 10 11 12 13 14 15",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 5 6 7 8 9 10 11 12 13 14 15");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);

  uint8_t data[11];
  uint8_t const expectedData[11] = {0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
  fakeStorage.Read(0x100, sizeof(data), data);

  ASSERT_TRUE(memcmp(data, expectedData, sizeof(data)) == 0);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_DifferentNumberFormats)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 5 0xEF 'A'",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 5 0xEF 'A'");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);

  uint8_t data[3];
  uint8_t const expectedData[3] = {0x05, 0xEF, 'A'};
  fakeStorage.Read(0x100, sizeof(data), data);

  ASSERT_TRUE(memcmp(data, expectedData, sizeof(data)) == 0);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_UnalignedAddress)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x101 5 6 7 8 9 10 11 12 13 14 15",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x101 5 6 7 8 9 10 11 12 13 14 15");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);

  uint8_t data[12];
  uint8_t const expectedData[12] = {0x00, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
  fakeStorage.Read(0x100, sizeof(data), data);

  ASSERT_TRUE(memcmp(data, expectedData, sizeof(data)) == 0);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_LastByte)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFF 5",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0xFFF 5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);

  uint8_t data[1];
  uint8_t const expectedData[1] = {0x05};
  fakeStorage.Read(0xFFF, sizeof(data), data);

  ASSERT_TRUE(memcmp(data, expectedData, sizeof(data)) == 0);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BeyondEndOfStorage1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFF 5 6",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0xFFF 5 6");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BeyondEndOfStorage2)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x1000 5",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Address out of bounds",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x1000 5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat1)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 -5",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Value '-5' is out of range [0;255]",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 -5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat2)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 -0x05",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"-0x05\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 -0x05");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat3)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 0xABCD",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Value '0xABCD' is out of range [0;255]",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 0xABCD");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat4)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 257",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Value '257' is out of range [0;255]",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 257");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat5)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 'AB'",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"'AB'\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 'AB'");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat6)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 A",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"A\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 A");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}
TEST_F(GPCC_stdif_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat7)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 \"A\"",
   "",
   "Invalid arguments. Try 'WriteRAS help'.",
   "Details:",
   "0: User entered invalid arguments.",
   "1: Invalid number/format: \"\"A\"\"",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("WriteRAS 0x100 \"A\"");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
}

} // namespace stdif
} // namespace gpcc_tests
