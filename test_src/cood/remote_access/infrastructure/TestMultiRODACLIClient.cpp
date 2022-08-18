/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/cood/remote_access/infrastructure/MultiRODACLIClient.hpp"
#include "TestbenchThreadBasedRAS.hpp"
#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace gpcc_tests {
namespace cood       {

using namespace testing;

// Testbench for class MultiRODACLIClient and its base class MultiRODACLIClientBase.
// Base class RODAClientBase is unit tested by TestSingleRODACLIClient.cpp
//
// We are using 3 (nbOfRODAs) instances of TestbenchThreadBasedRAS to get 3 sets of:
// - an object dictionary
// - some objects
// - a remote access server providing a RODA interface
// - a log facility and a logger intended to be used by the test case
// Further we add a CLI and a FakeTerminal.
// Last but not least we have the UUT.
class gpcc_cood_MultiRODACLIClient_TestsF: public Test
{
  public:
    gpcc_cood_MultiRODACLIClient_TestsF(void);
    ~gpcc_cood_MultiRODACLIClient_TestsF(void);

  protected:
    // Number of RODA interfaces used in this test.
    static constexpr size_t nbOfRODAs = 3U;

    // CLI and fake terminal.
    gpcc_tests::cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliNeedsStop;

    // RAS, OD, objects and log facility, nbOfRODAs times.
    TestbenchThreadBasedRAS rasAndCommonStuff[nbOfRODAs];
    bool rasNeedsStop[nbOfRODAs];

    // UUT
    std::unique_ptr<gpcc::cood::MultiRODACLIClient> spUUT;


    void SetUp(void) override;
    void TearDown(void) override;

    void InstantiateUUT_EtherCATStyle(void);
    void InstantiateUUT_CANopenStyle(void);
    void Login(void);
};

typedef gpcc_cood_MultiRODACLIClient_TestsF gpcc_cood_MultiRODACLIClient_DeathTestsF;

gpcc_cood_MultiRODACLIClient_TestsF::gpcc_cood_MultiRODACLIClient_TestsF(void)
: Test()
, terminal(180U, 8U)
, cli(terminal, 180U, 8U, "CLI", nullptr)
, cliNeedsStop(false)
, rasAndCommonStuff()
, rasNeedsStop{false, false, false}
, spUUT()
{
  static_assert(gpcc_cood_MultiRODACLIClient_TestsF::nbOfRODAs >= 3U, "Test cases rely on at least 3 RODA interfaces being available.");

  terminal.EnableRecordingOfDroppedOutLines();
}

gpcc_cood_MultiRODACLIClient_TestsF::~gpcc_cood_MultiRODACLIClient_TestsF(void)
{
}

void gpcc_cood_MultiRODACLIClient_TestsF::SetUp(void)
{
  // note: TearDown() will be invoked even if this throws

  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliNeedsStop = true;
  terminal.WaitForInputProcessed();

  for (size_t i = 0U; i < nbOfRODAs; ++i)
  {
    rasAndCommonStuff[i].StartUUT();
    rasNeedsStop[i] = true;
  }
}

void gpcc_cood_MultiRODACLIClient_TestsF::TearDown(void)
{
  try
  {
    // unregister RODA interfaces from UUT
    if (spUUT != nullptr)
    {
      for (size_t i = 0U; i < nbOfRODAs; ++i)
        spUUT->Unregister(i);
      spUUT.reset();
    }

    // stop all servers
    for (size_t i = 0U; i < nbOfRODAs; ++i)
    {
      if (rasNeedsStop[i])
        rasAndCommonStuff[i].StopUUT();
    }

    if (cliNeedsStop)
      cli.Stop();

    if (HasFailure())
    {
      for (size_t i = 0U; i < nbOfRODAs; ++i)
      {
        std::cout << "*****************************************************" << std::endl
                  << "Recorded log messages RODA #" << i << std::endl
                  << "*****************************************************" << std::endl;
        #if 1
        rasAndCommonStuff[i].PrintLogMessagesToStdout();
        #else
        std::cout << "If required, then enable this in gpcc_cood_MultiRODACLIClient_TestsF::TearDown" << std::endl;
        #endif
      }

      std::cout << "*****************************************************" << std::endl
                << "Content of fake terminal" << std::endl
                << "*****************************************************" << std::endl;
      std::cout << terminal.GetDroppedOutLinesPlusCurrentScreenContent() << std::endl;
    }
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "gpcc_cood_MultiRODACLIClient_TestsF::TearDown: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-tests are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("gpcc_cood_MultiRODACLIClient_TestsF::TearDown: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("gpcc_cood_MultiRODACLIClient_TestsF::TearDown: Caught an unknown exception");
  }
}

void gpcc_cood_MultiRODACLIClient_TestsF::InstantiateUUT_EtherCATStyle(void)
{
  spUUT = std::make_unique<gpcc::cood::MultiRODACLIClient>(cli, "roda", true);
}

void gpcc_cood_MultiRODACLIClient_TestsF::InstantiateUUT_CANopenStyle(void)
{
  spUUT = std::make_unique<gpcc::cood::MultiRODACLIClient>(cli, "roda", false);
}

void gpcc_cood_MultiRODACLIClient_TestsF::Login(void)
{
  terminal.Input("login");

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, InstantiateLoginAndDestroy)
{
  InstantiateUUT_EtherCATStyle();
  Login();

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">"
  };

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cood_MultiRODACLIClient_DeathTestsF, DestroyButOneRODAItfStillRegistered)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  InstantiateUUT_EtherCATStyle();
  Login();

  spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U);

  EXPECT_DEATH(spUUT.reset(), ".*At least one interface still registered.*");
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, CheckCommandPresent)
{
  InstantiateUUT_EtherCATStyle();
  Login();

  terminal.Input("help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*roda*", true));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, CheckSubCommandsMentionedInHelp)
{
  InstantiateUUT_EtherCATStyle();
  Login();

  terminal.Input("roda help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n- enum *\n- info *\n- read*\n- write*\n- caread*\n- cawrite*", true));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, RegisterAndUnregister)
{
  InstantiateUUT_EtherCATStyle();

  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U));
  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[1].GetUUT(), 1U));
  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[2].GetUUT(), 2U));

  ASSERT_NO_THROW(spUUT->Unregister(0U));
  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U)) << "Could not register a RODA using a previously unregistered ID";

  ASSERT_NO_THROW(spUUT->Unregister(0U));
  ASSERT_NO_THROW(spUUT->Unregister(1U));
  ASSERT_NO_THROW(spUUT->Unregister(2U));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Register_IdAlreadyUsed)
{
  InstantiateUUT_EtherCATStyle();

  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U));
  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[1].GetUUT(), 1U));

  ASSERT_THROW(spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U), std::logic_error) << "ID already used, but Register(...) did not throw.";
  ASSERT_THROW(spUUT->Register(rasAndCommonStuff[2].GetUUT(), 0U), std::logic_error) << "ID already used, but Register(...) did not throw.";

  ASSERT_NO_THROW(spUUT->Unregister(0U));
  ASSERT_NO_THROW(spUUT->Unregister(1U));
  ASSERT_NO_THROW(spUUT->Unregister(2U));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Unregister_Twice)
{
  InstantiateUUT_EtherCATStyle();

  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U));
  ASSERT_NO_THROW(spUUT->Unregister(0U));
  ASSERT_NO_THROW(spUUT->Unregister(0U));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Unregister_NeverRegistered)
{
  InstantiateUUT_EtherCATStyle();

  ASSERT_NO_THROW(spUUT->Unregister(0U));
  ASSERT_NO_THROW(spUUT->Unregister(1U));
  ASSERT_NO_THROW(spUUT->Unregister(2U));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Unregister_NoLongerAccessibleViaCLI)
{
  InstantiateUUT_EtherCATStyle();
  Login();

  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U));
  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[1].GetUUT(), 1U));
  ASSERT_NO_THROW(spUUT->Register(rasAndCommonStuff[2].GetUUT(), 2U));

  ASSERT_NO_THROW(spUUT->Unregister(1U));

  terminal.Input("roda 1 enum");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Given RODA interface ID is unknown.*", true));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Access_IdNeverRegistered)
{
  InstantiateUUT_EtherCATStyle();
  Login();

  terminal.Input("roda 27 enum");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Given RODA interface ID is unknown.*", true));
}

#ifndef SKIP_TFC_BASED_TESTS

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Access)
{
  // prepare different data
  for (size_t i = 0U; i < nbOfRODAs; ++i)
  {
    gpcc::osal::MutexLocker locker(rasAndCommonStuff[i].dataMutex);
    rasAndCommonStuff[i].data0x1000 = i;
  }

  InstantiateUUT_EtherCATStyle();
  Login();

  spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U);
  spUUT->Register(rasAndCommonStuff[1].GetUUT(), 1U);
  spUUT->Register(rasAndCommonStuff[2].GetUUT(), 2U);


  terminal.Input("roda 0 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.Input("roda 1 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.Input("roda 2 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // access the same again
  terminal.Input("roda 2 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // access a different one
  terminal.Input("roda 0 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*\n"
  ">roda 0 read 0x1000:0\n"
  "0 (0x00000000)\n"
  ">roda 1 read 0x1000:0\n"
  "1 (0x00000001)\n"
  ">roda 2 read 0x1000:0\n"
  "2 (0x00000002)\n"
  ">roda 2 read 0x1000:0\n"
  "2 (0x00000002)\n"
  ">roda 0 read 0x1000:0\n"
  "0 (0x00000000)\n"
  ">*", true));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Access_RODA_not_ready)
{
  // prepare different data
  for (size_t i = 0U; i < nbOfRODAs; ++i)
  {
    gpcc::osal::MutexLocker locker(rasAndCommonStuff[i].dataMutex);
    rasAndCommonStuff[i].data0x1000 = i;
  }

  InstantiateUUT_EtherCATStyle();
  Login();

  spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U);
  spUUT->Register(rasAndCommonStuff[1].GetUUT(), 1U);
  spUUT->Register(rasAndCommonStuff[2].GetUUT(), 2U);

  rasAndCommonStuff[0].StopUUT();
  rasNeedsStop[0] = false;

  terminal.Input("roda 0 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.Input("roda 1 read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*\n"
  ">roda 0 read 0x1000:0\n"
  "\n"
  "*\n"
  "*Timeout. RODA interface is not ready.\n"
  ">roda 1 read 0x1000:0\n"
  "1 (0x00000001)\n"
  ">*", true));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Style_EtherCAT)
{
  InstantiateUUT_EtherCATStyle();
  Login();

  spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U);


  terminal.Input("roda 0 info 0x1000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nObject 0x1000: VAR (UNSIGNED32) \"Testobject 1\"\n"
                                                        "  Subindex 0: UNSIGNED32*RRRWWW*4.0*\"Testobject 1\"\n"
                                                        ">\n", true));
}

TEST_F(gpcc_cood_MultiRODACLIClient_TestsF, Style_CANopen)
{
  InstantiateUUT_CANopenStyle();
  Login();

  spUUT->Register(rasAndCommonStuff[0].GetUUT(), 0U);


  terminal.Input("roda 0 info 0x1000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nObject 0x1000: VAR (UNSIGNED32) \"Testobject 1\"\n"
                                                        "  Subindex 0: UNSIGNED32*rw*4.0*\"Testobject 1\"\n"
                                                        ">\n", true));
}

#endif // SKIP_TFC_BASED_TESTS

} // namespace cood
} // namespace gpcc_tests
