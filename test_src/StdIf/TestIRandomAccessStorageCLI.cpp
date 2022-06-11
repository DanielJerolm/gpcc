/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#include "gpcc/test_src/file_systems/EEPROMSectionSystem/FakeEEPROM.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/cli/Command.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/StdIf/IRandomAccessStorage.hpp"
#include "gpcc/src/StdIf/IRandomAccessStorageCLI.hpp"
#include "gtest/gtest.h"
#include <functional>
#include <cstdint>
#include <cstddef>

namespace gpcc_tests
{
namespace StdIf
{

class GPCC_StdIf_IRandomAccessStorageCLI_Tests: public testing::Test
{
  public:
    static const size_t storageSize     = 4*1024;
    static const size_t storagePageSize = 32;

    GPCC_StdIf_IRandomAccessStorageCLI_Tests(void);
    virtual ~GPCC_StdIf_IRandomAccessStorageCLI_Tests(void);

  protected:
    file_systems::EEPROMSectionSystem::FakeEEPROM fakeStorage;
    cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliNeedsStop;

    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

GPCC_StdIf_IRandomAccessStorageCLI_Tests::GPCC_StdIf_IRandomAccessStorageCLI_Tests(void)
: Test()
, fakeStorage(storageSize, storagePageSize)
, terminal(80, 8)
, cli(terminal, 80, 8, "CLI", nullptr)
, cliNeedsStop(false)
{
}
GPCC_StdIf_IRandomAccessStorageCLI_Tests::~GPCC_StdIf_IRandomAccessStorageCLI_Tests(void)
{
}

void GPCC_StdIf_IRandomAccessStorageCLI_Tests::SetUp(void)
{
  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliNeedsStop = true;

  terminal.WaitForInputProcessed();

  cli.AddCommand(gpcc::cli::Command::Create("ReadRAS", "\nHelp text",
                                            std::bind(&gpcc::StdIf::CliCmdReadIRandomAccessStorage,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &fakeStorage)));
  cli.AddCommand(gpcc::cli::Command::Create("WriteRAS", "\nHelp text",
                                            std::bind(&gpcc::StdIf::CliCmdWriteIRandomAccessStorage,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &fakeStorage)));
}
void GPCC_StdIf_IRandomAccessStorageCLI_Tests::TearDown(void)
{
  if (cliNeedsStop)
    cli.Stop();
}

void GPCC_StdIf_IRandomAccessStorageCLI_Tests::Login(void)
{
  terminal.Input("login");

  for (int i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, InstantiationAndLogin)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_WrongNbOfParams0)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS",
   "Error: 2 parameters expected!",
   "Try 'rdeeprom help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_WrongNbOfParams1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0",
   "Error: 2 parameters expected!",
   "Try 'rdeeprom help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_WrongNbOfParams3)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 0 0",
   "Error: 2 parameters expected!",
   "Try 'rdeeprom help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 0 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_AddressNotHex)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0 0",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_AddressBadChars)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xXYZ 0",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0xXYZ 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_NbOfBytesNegative)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 -1",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 -1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_NbOfBytesBadChars)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 XYZ",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 XYZ");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_NbOfBytesTooLarge)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x0 1025",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadRAS 0x0 1025");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_AddressOutOf32Bit)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFFFFFF0 17",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_AddressIn32BitButOutOfBounds)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFFFFFF0 16",
   "Error: Attempt to read out of bounds",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_ZeroBytes)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_OneByte)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_12Byte)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_17Byte)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_UnalignedAddress17Bytes)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_LastByteOfStorage)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_BeyondEndOfStorage1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0xFFF 2",
   "Error: Attempt to read out of bounds",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Read_BeyondEndOfStorage2)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">ReadRAS 0x1000 1",
   "Error: Attempt to read out of bounds",
   ">",
   "",
   "",
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

TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_WrongNbOfParams0)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS",
   "Error: At least 2 parameters expected!",
   "Try 'wreeprom help'",
   ">",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_WrongNbOfParams1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x0",
   "Error: At least 2 parameters expected!",
   "Try 'wreeprom help'",
   ">",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_AddressNotHex)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0 0",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_AddressBadChars)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xXYZ 0",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_AddressOutOf32Bit)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFFFFFFF 0 0",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_AddressIn32BitButOutOfBounds)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFFFFFFF 0",
   "Error: Attempt to write out of bounds",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_OneByte)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_10Byte)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_DifferentNumberFormats)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_UnalignedAddress)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_LastByte)
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BeyondEndOfStorage1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0xFFF 5 6",
   "Error: Attempt to write out of bounds",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BeyondEndOfStorage2)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x1000 5",
   "Error: Attempt to write out of bounds",
   ">",
   "",
   "",
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat1)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 -5",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat2)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 -0x05",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat3)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 0xABCD",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat4)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 257",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat5)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 'AB'",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat6)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 A",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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
TEST_F(GPCC_StdIf_IRandomAccessStorageCLI_Tests, Write_BadNumberFormat7)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">WriteRAS 0x100 \"A\"",
   "Error: Invalid parameter(s)",
   ">",
   "",
   "",
   ""
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

} // namespace StdIf
} // namespace gpcc_tests
