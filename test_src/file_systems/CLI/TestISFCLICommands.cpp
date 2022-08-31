/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/Command.hpp>
#include "../EEPROMSectionSystem/EEPROMSectionSystemTestFixture.hpp"
#include "../EEPROMSectionSystem/RandomData.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gpcc/src/file_systems/CLI/ISFCLICommands.hpp"
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <algorithm>
#include <functional>
#include <list>
#include <stdexcept>
#include <string>

namespace gpcc_tests
{
namespace file_systems
{

using namespace gpcc_tests::cli;
using namespace gpcc::file_systems;

class GPCC_FileSystems_CLI_TestsF: public EEPROMSectionSystem::EEPROMSectionSystemTestFixture
{
  public:
    GPCC_FileSystems_CLI_TestsF(void);
    virtual ~GPCC_FileSystems_CLI_TestsF(void);

  protected:
    FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliRunning;

    EEPROMSectionSystem::RandomData rndData1;
    EEPROMSectionSystem::RandomData rndData2;


    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

GPCC_FileSystems_CLI_TestsF::GPCC_FileSystems_CLI_TestsF(void)
: EEPROMSectionSystemTestFixture()
, terminal(80, 8)
, cli(terminal, 80, 8, "CLI", nullptr)
, cliRunning(false)
, rndData1(64, 64)
, rndData2(130, 130)
{
}
GPCC_FileSystems_CLI_TestsF::~GPCC_FileSystems_CLI_TestsF(void)
{
}

void GPCC_FileSystems_CLI_TestsF::SetUp(void)
{
  EEPROMSectionSystemTestFixture::SetUp();

  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliRunning = true;

  terminal.WaitForInputProcessed();

  cli.AddCommand(gpcc::cli::Command::Create("Delete", "\nHelp text",
                                            std::bind(&gpcc::file_systems::CLICMDDelete,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      static_cast<IFileStorage*>(&uut))));
  cli.AddCommand(gpcc::cli::Command::Create("Rename", "\nHelp text",
                                            std::bind(&gpcc::file_systems::CLICMDRename,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      static_cast<IFileStorage*>(&uut))));
  cli.AddCommand(gpcc::cli::Command::Create("Enumerate", "\nHelp text",
                                            std::bind(&gpcc::file_systems::CLICMDEnumerate,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      static_cast<IFileStorage*>(&uut))));
  cli.AddCommand(gpcc::cli::Command::Create("FreeSpace", "\nHelp text",
                                            std::bind(&gpcc::file_systems::CLICMDFreeSpace,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      static_cast<IFileStorage*>(&uut))));
  cli.AddCommand(gpcc::cli::Command::Create("Dump", "\nHelp text",
                                            std::bind(&gpcc::file_systems::CLICMDDump,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      static_cast<IFileStorage*>(&uut))));
  cli.AddCommand(gpcc::cli::Command::Create("Copy", "\nHelp text",
                                            std::bind(&gpcc::file_systems::CLICMDCopy,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      static_cast<IFileStorage*>(&uut))));

  Format(fakeStorage.GetPageSize());

  rndData1.Write("rndData1", false, uut);
  rndData2.Write("rndData2", false, uut);

  fakeStorage.writeAccessCnt = 0;
}
void GPCC_FileSystems_CLI_TestsF::TearDown(void)
{
  try
  {
    if (uut.GetState() != gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted)
      uut.Unmount();

    if (cliRunning)
      cli.Stop();

    EEPROMSectionSystemTestFixture::TearDown();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

void GPCC_FileSystems_CLI_TestsF::Login(void)
{
  terminal.Input("login");

  for (int i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(GPCC_FileSystems_CLI_TestsF, InstantiationAndLogin)
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
TEST_F(GPCC_FileSystems_CLI_TestsF, Delete_NoParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Delete",
   "Error: At least one parameter expected!",
   "Try 'file_delete help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Delete");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Delete_NoSuchFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Delete File1",
   "Error, no such file: File1",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Delete File1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Delete_NoSuchFiles)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Delete File1 File2",
   "Error, no such file: File1",
   "Error, no such file: File2",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Delete File1 File2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Delete_OneFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Delete rndData1",
   "Deleted: rndData1",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Delete rndData1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(1U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData2") != sections.end());
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Delete_TwoFiles)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Delete rndData1 rndData2",
   "Deleted: rndData1",
   "Deleted: rndData2",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Delete rndData1 rndData2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(0U, sections.size());
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Delete_TwoFiles_OneNotExist)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Delete Bla rndData2",
   "Error, no such file: Bla",
   "Deleted: rndData2",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Delete Bla rndData2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(1U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData1") != sections.end());
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Rename_NoParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Rename",
   "Error: Two arguments expected!",
   "Try 'file_rename help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Rename");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Rename_OneParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Rename rndData1",
   "Error: Two arguments expected!",
   "Try 'file_rename help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Rename rndData1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Rename_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Rename rndData1 rndData3",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Rename rndData1 rndData3");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(2U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData2") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData3") != sections.end());
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Enumerate_BadParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Enumerate X",
   "Error: Bad arguments!",
   "Try 'file_enumerate help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Enumerate X");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Enumerate_NoParams)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Enumerate",
   "rndData1",
   "rndData2",
   "2 files",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Enumerate");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Enumerate_Option_s)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Enumerate -s",
   "rndData1 (72 byte)",
   "rndData2 (138 byte)",
   "2 files",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Enumerate -s");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, FreeSpace_BadParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">FreeSpace X",
   "Error: No arguments expected!",
   "Try 'file_freespace help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("FreeSpace X");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, FreeSpace_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">FreeSpace",
   "13794 byte",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("FreeSpace");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Dump_NoParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Dump",
   "",
   "Error! Caught an exception:",
   "0: EEPROMSectionSystem::Open: Invalid name",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Dump");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Dump_NoSuchFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Dump Test",
   "",
   "Error! Caught an exception:",
   "0: File \"Test\" is not existing.",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Dump Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Dump_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Dump File1",
   "Offset      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF",
   "0x00000000: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ................",
   "0x00000010: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ................",
   "Dumped 32 byte",
   ">"
  };

  auto fw = uut.Create("File1", false);
  ON_SCOPE_EXIT() { try { fw->Close(); } catch (std::exception const &) {}; };
  for (uint_fast8_t i = 0; i < 32; i++)
    fw->Write_uint8(i);
  ON_SCOPE_EXIT_DISMISS();
  fw->Close();
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Dump File1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Dump_StopAfter1024)
{
  char const * expected[8] =
  {
   "0x000003B0: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ................",
   "0x000003C0: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ................",
   "0x000003D0: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ................",
   "0x000003E0: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ................",
   "0x000003F0: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ................",
   "Continue? (no = stop, anything else = continue):no",
   "aborted",
   ">"
  };

  auto fw = uut.Create("File1", false);
  ON_SCOPE_EXIT() { try { fw->Close(); } catch (std::exception const &) {}; };
  for (uint_fast16_t i = 0; i < 1025; i++)
    fw->Write_uint8(i % 32U);
  ON_SCOPE_EXIT_DISMISS();
  fw->Close();
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Dump File1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("no");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Dump_ContinueAfter1024)
{
  char const * expected[8] =
  {
   "0x000003C0: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ................",
   "0x000003D0: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ................",
   "0x000003E0: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ................",
   "0x000003F0: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F ................",
   "Continue? (no = stop, anything else = continue):y",
   "0x00000400: FF                                              .",
   "Dumped 1025 byte",
   ">"
  };

  auto fw = uut.Create("File1", false);
  ON_SCOPE_EXIT() { try { fw->Close(); } catch (std::exception const &) {}; };
  for (uint_fast16_t i = 0; i < 1024; i++)
    fw->Write_uint8(i % 32U);
  fw->Write_uint8(0xFFU);
  ON_SCOPE_EXIT_DISMISS();
  fw->Close();
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Dump File1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("y");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_NoParameters)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy",
   "Error: Two arguments expected!",
   "Try 'file_copy help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_OneParameter)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy a",
   "Error: Two arguments expected!",
   "Try 'file_copy help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy a");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_ThreeParameter)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy a b c",
   "Error: Two arguments expected!",
   "Try 'file_copy help'",
   ">",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy a b c");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy rndData1 Copy",
   "Copy done",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy rndData1 Copy");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(3U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData1") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData2") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Copy") != sections.end());

  rndData1.Compare("Copy", uut);
  rndData1.Compare("rndData1", uut);
  rndData2.Compare("rndData2", uut);
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_FileAlreadyExisting)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy rndData1 rndData2",
   "",
   "Error! Caught an exception:",
   "0: File \"rndData2\" is already existing.",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy rndData1 rndData2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(2U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData1") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData2") != sections.end());

  rndData1.Compare("rndData1", uut);
  rndData2.Compare("rndData2", uut);
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_SrcAndDestEqual)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy rndData1 rndData1",
   "Error: Cannot copy file to itself",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy rndData1 rndData1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(2U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData1") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData2") != sections.end());

  rndData1.Compare("rndData1", uut);
  rndData2.Compare("rndData2", uut);
}
TEST_F(GPCC_FileSystems_CLI_TestsF, Copy_SrcNotExisting)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Copy rndData3 rndData4",
   "",
   "Error! Caught an exception:",
   "0: File \"rndData3\" is not existing.",
   ">",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Copy rndData3 rndData4");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_TRUE(terminal.Compare(expected));

  auto sections = uut.Enumerate();
  ASSERT_EQ(2U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData1") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "rndData2") != sections.end());

  rndData1.Compare("rndData1", uut);
  rndData2.Compare("rndData2", uut);
}

} // namespace file_systems
} // namespace gpcc_tests
