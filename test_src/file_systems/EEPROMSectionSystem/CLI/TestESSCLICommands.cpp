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

#include "../EEPROMSectionSystemTestFixture.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/cli/Command.hpp"
#include "gpcc/src/file_systems/EEPROMSectionSystem/CLI/ESSCLICommands.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <functional>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

using namespace gpcc_tests::cli;

class GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF: public EEPROMSectionSystemTestFixture
{
  public:
    GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF(void);
    virtual ~GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF(void);

  protected:
    FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliRunning;

    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF::GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF(void)
: EEPROMSectionSystemTestFixture()
, terminal(80, 8)
, cli(terminal, 80, 8, "CLI", nullptr)
, cliRunning(false)
{
}
GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF::~GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF(void)
{
}

void GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF::SetUp(void)
{
  EEPROMSectionSystemTestFixture::SetUp();

  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliRunning = true;

  terminal.WaitForInputProcessed();

  cli.AddCommand(gpcc::cli::Command::Create("GetState", "\nHelp text",
                                            std::bind(&gpcc::file_systems::EEPROMSectionSystem::CLICMDGetState,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &uut)));
  cli.AddCommand(gpcc::cli::Command::Create("Format", "\nHelp text",
                                            std::bind(&gpcc::file_systems::EEPROMSectionSystem::CLICMDFormat,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &uut,
                                                      fakeStorage.GetPageSize())));
  cli.AddCommand(gpcc::cli::Command::Create("Unmount", "\nHelp text",
                                            std::bind(&gpcc::file_systems::EEPROMSectionSystem::CLICMDUnmount,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &uut)));
  cli.AddCommand(gpcc::cli::Command::Create("Mount", "\nHelp text",
                                            std::bind(&gpcc::file_systems::EEPROMSectionSystem::CLICMDMount,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      &uut)));
}
void GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF::TearDown(void)
{
  try
  {
    if (cliRunning)
      cli.Stop();

    EEPROMSectionSystemTestFixture::TearDown();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

void GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF::Login(void)
{
  terminal.Input("login");

  for (int i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, InstantiationAndLogin)
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
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDGetState)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">GetState",
   "not_mounted",
   ">GetState",
   "mounted",
   ">",
   ""
  };

  terminal.Input("GetState");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  Format(storagePageSize);

  terminal.Input("GetState");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDGetState_unexpectedParams)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">GetState x",
   "Error: No parameters expected",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("GetState x");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDFormat_no)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Format",
   "Format storage, all data will be lost! Sure? (yes/no):no",
   "Aborted. Storage has not been touched.",
   ">",
   "",
   ""
  };

  terminal.Input("Format");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("no");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDFormat_unexpectedParams)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Format x",
   "Error: No parameters expected",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("Format x");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDFormat_not_unmounted)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Format",
   "Format storage, all data will be lost! Sure? (yes/no):yes",
   "Error: EEPROMSectionSystem must be unmounted!",
   ">",
   "",
   ""
  };

  Format(storagePageSize);
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Format");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("yes");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDFormat_OK)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Format",
   "Format storage, all data will be lost! Sure? (yes/no):yes",
   "Formatting EEPROMSectionSystem with block size 128 bytes.",
   "This make take a few seconds...",
   "Done",
   ">"
  };

  terminal.Input("Format");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("yes");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDUnmount_unexpectedParams)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Unmount x",
   "Error: No parameters expected",
   ">",
   "",
   "",
   ""
  };

  Format(storagePageSize);
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Unmount x");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDUnmount_notMounted)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Unmount",
   "Unmounted",
   ">",
   "",
   "",
   ""
  };

  terminal.Input("Unmount");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDUnmount_OK)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Unmount",
   "Unmounted",
   ">",
   "",
   "",
   ""
  };

  Format(storagePageSize);
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Unmount");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDMount_unexpectedParams)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Mount x",
   "Error: No parameters expected",
   ">",
   "",
   "",
   ""
  };

  Format(storagePageSize);
  uut.Unmount();
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Mount x");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDMount_unmountedBefore)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Mount",
   "Mounting for ro-access...",
   "Mounted for ro-access.",
   "Mounting for rw-access...",
   "Mounted for rw-access.",
   ">"
  };

  Format(storagePageSize);
  uut.Unmount();
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Mount");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDMount_ro_mountedBefore)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Mount",
   "Mounting for rw-access...",
   "Mounted for rw-access.",
   ">",
   "",
   ""
  };

  Format(storagePageSize);
  uut.Unmount();
  uut.MountStep1();
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Mount");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_CLI_TestsF, CLICMDMount_mountedBefore)
{
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">Mount",
   "Mounting for rw-access...",
   "Mounted for rw-access.",
   ">",
   "",
   ""
  };

  Format(storagePageSize);
  fakeStorage.writeAccessCnt = 0;

  terminal.Input("Mount");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_TRUE(terminal.Compare(expected));

  uut.Unmount();
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
