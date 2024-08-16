/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019, 2024 Daniel Jerolm
*/

#include <gpcc/cood/cli/CLIAdapterForCANOpen.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include <gpcc_test/cli/FakeTerminal.hpp>
#include "testcases/cood/remote_access/infrastructure/TestbenchThreadBasedRAS.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;
using namespace testing;

// Test fixture for class CLIAdapterForCANOpen
// We are using an instance of TestbenchThreadBasedRAS to get:
// - an object dictionary
// - some objects
// - a log facility and a logger intended to be used by the test case
// Further we add a CLI and a FakeTerminal.
// Last but not least we have the UUT.
// The RemoteAccessServer contained in TestbenchThreadBasedRAS is unused. It is instantiated, but it won't be started.
class gpcc_cood_CLIAdapterForCANOpen_TestsF: public Test
{
  public:
    gpcc_cood_CLIAdapterForCANOpen_TestsF(void);
    ~gpcc_cood_CLIAdapterForCANOpen_TestsF(void);

  protected:
    // CLI and fake terminal.
    gpcc_tests::cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliNeedsStop;

    // OD, objects and log facility
    TestbenchThreadBasedRAS rasAndCommonStuff;

    // UUT
    std::unique_ptr<CLIAdapterForCANOpen> spUUT;


    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

gpcc_cood_CLIAdapterForCANOpen_TestsF::gpcc_cood_CLIAdapterForCANOpen_TestsF(void)
: Test()
, terminal(180U, 8U)
, cli(terminal, 180U, 8U, "CLI", nullptr)
, cliNeedsStop(false)
, rasAndCommonStuff()
, spUUT()
{
  terminal.EnableRecordingOfDroppedOutLines();
}

gpcc_cood_CLIAdapterForCANOpen_TestsF::~gpcc_cood_CLIAdapterForCANOpen_TestsF(void)
{
}

void gpcc_cood_CLIAdapterForCANOpen_TestsF::SetUp(void)
{
  // note: TearDown() will be invoked even if this throws

  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliNeedsStop = true;
  terminal.WaitForInputProcessed();

  spUUT = std::make_unique<CLIAdapterForCANOpen>(rasAndCommonStuff.od, cli, "OD");
}

void gpcc_cood_CLIAdapterForCANOpen_TestsF::TearDown(void)
{
  try
  {
    if (cliNeedsStop)
      cli.Stop();

    spUUT.reset();

    if (HasFailure())
    {
      std::cout << "*****************************************************" << std::endl
                << "Recorded log messages" << std::endl
                << "*****************************************************" << std::endl;
      #if 1
      rasAndCommonStuff.PrintLogMessagesToStdout();
      #else
      std::cout << "If required, then enable this in gpcc_cood_CLIAdapterForCANOpen_TestsF::TearDown" << std::endl;
      #endif

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
      std::string str = "gpcc_cood_CLIAdapterForCANOpen_TestsF::TearDown: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-tests are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("gpcc_cood_CLIAdapterForCANOpen_TestsF::TearDown: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("gpcc_cood_CLIAdapterForCANOpen_TestsF::TearDown: Caught an unknown exception");
  }
}

void gpcc_cood_CLIAdapterForCANOpen_TestsF::Login(void)
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

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, InstantiateAndDestroy)
{
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

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CheckCommandPresent)
{
  Login();

  terminal.Input("help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*OD*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CheckSubCommandsMentionedInHelp)
{
  Login();

  terminal.Input("OD help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n- enum *\n- info *\n- read*\n- write*\n- caread*\n- cawrite*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate)
{
  Login();

  terminal.Input("OD enum");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // There are many objects. Lets take two random samples.
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n0x1000*VAR*UNSIGNED32*\"Testobject 1\"\n*", true)) << "Object 0x1000 (random sample) is missing in output.";
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n0x1001*VAR*UNSIGNED32*\"Testobject 2\"\n*", true)) << "Object 0x1001 (random sample) is missing in output.";
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_RangeWithOneIndex)
{
  Login();

  terminal.Input("OD enum 0x1001-0x1001");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "0x1001*VAR*UNSIGNED32*\"Testobject 2\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_NoObjs)
{
  rasAndCommonStuff.od.Clear();

  Login();

  terminal.Input("OD enum");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nNo objects\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_NoObjsInRange)
{
  Login();

  terminal.Input("OD enum 0x0100-0x200");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nNo objects\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_ObjsInRangeA)
{
  Login();

  terminal.Input("OD enum 0x0100-0x1001");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // we exactly know that there are 2 objects
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "0x1000*VAR*UNSIGNED32*\"Testobject 1\"\n"
  "0x1001*VAR*UNSIGNED32*\"Testobject 2\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_ObjsInRangeB)
{
  Login();

  terminal.Input("OD enum 0x1002-0x1004");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // we exactly know that there are 3 objects
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "0x1002*VAR*UNSIGNED32*\"Testobject 3\"\n"
  "0x1003*VAR*OCTET_STRING*\"Testobject 4\"\n"
  "0x1004*VAR*UNSIGNED32*\"Testobject 5\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_ObjsInRangeC)
{
  Login();

  terminal.Input("OD enum 0x2000-0x4000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // we exactly know that there are 2 objects
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "0x2000*ARRAY*UNSIGNED8*\"Testobject 8\"\n"
  "0x3000*RECORD*DOMAIN*\"Testobject 9\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_BadRange)
{
  Login();

  terminal.Input("OD enum 0x1001-0x1000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_BadParams1)
{
  Login();

  terminal.Input("OD enum 0x1000 0x1001");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Enumerate_BadParams2)
{
  Login();

  terminal.Input("OD enum 1000-0x1001");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_ObjectNotExisting)
{
  Login();

  terminal.Input("OD info 0x0007");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Error: No object with given index\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_InvalidParams1)
{
  Login();

  terminal.Input("OD info 0xXYZA");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_InvalidParams2)
{
  Login();

  terminal.Input("OD info 0x0500 0x12");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_InvalidParams3)
{
  Login();

  terminal.Input("OD info 0x0500 ASM");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_VAR_Obj_NoASMReq)
{
  Login();

  terminal.Input("OD info 0x1000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nObject 0x1000: VAR (UNSIGNED32) \"Testobject 1\"\n"
                                                        "  Subindex 0: UNSIGNED32*rw*4.0*\"Testobject 1\"\n"
                                                        ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_VAR_Obj_asmReq_ObjWithASM)
{
  Login();

  terminal.Input("OD info 0x1000 asm");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nObject 0x1000: VAR (UNSIGNED32) \"Testobject 1\"\n"
                                                        "  Subindex 0: UNSIGNED32*rw*4.0*\"Testobject 1\"\n"
                                                        "              4 byte(s) of ASM: DE AD BE EF\n"
                                                        ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_VAR_Obj_asmReq_ObjHasNoASM)
{
  Login();

  terminal.Input("OD info 0x1003 asm");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nObject 0x1003: VAR (OCTET_STRING) \"Testobject 4\"\n"
                                                        "  Subindex 0: OCTET_STRING*rw*128.0*\"Testobject 4\"\n"
                                                        "              No app-specific meta data.\n"
                                                        ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_ARRAY_Obj_NoASMReq)
{
  Login();

  terminal.Input("OD info 0x2000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*\nObject 0x2000: ARRAY (UNSIGNED8) \"Testobject 8\"\n"
  "  Subindex      0: UNSIGNED8       rw        1.0 Byte(s) \"Number of subindices\"\n"
  "  Subindex 1..255: UNSIGNED8       rw        1.0 Byte(s) \"Subindex 1\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_ARRAY_Obj_asmReq_ObjHasNoASM)
{
  Login();

  terminal.Input("OD info 0x2000 asm");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*\nObject 0x2000: ARRAY (UNSIGNED8) \"Testobject 8\"\n"
  "  Subindex   0: UNSIGNED8       rw        1.0 Byte(s) \"Number of subindices\"\n"
  "                No app-specific meta data.\n"
  "  Subindex   1: UNSIGNED8       rw        1.0 Byte(s) \"Subindex 1\"\n"
  "                No app-specific meta data.\n"
  "*\n"
  "  Subindex 255: UNSIGNED8       rw        1.0 Byte(s) \"Subindex 255\"\n"
  "                No app-specific meta data.\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_RECORD_Obj_NoASMReq)
{
  Login();

  terminal.Input("OD info 0x3000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "Object 0x3000: RECORD (DOMAIN) \"Testobject 9\"\n"
  "  Subindex  0: UNSIGNED8       ro        1.0 Byte(s) \"Number of subindices\"\n"
  "  Subindex  1: BOOLEAN         rw        0.1 Byte(s) \"Data Bool\"\n"
  "  Subindex  2: INTEGER8        rw        1.0 Byte(s) \"Data i8\"\n"
  "  Subindex  3: UNSIGNED8       rw        1.0 Byte(s) \"Data ui8\"\n"
  "  Subindex  4: UNSIGNED32      rw        4.0 Byte(s) \"Data ui32a\"\n"
  "  Subindex  5: BIT1            rw        0.1 Byte(s) \"Bit 0\"\n"
  "  Subindex  6: BIT2            rw        0.2 Byte(s) \"Bit 7..8\"\n"
  "  Subindex  7: BIT1            rw        0.1 Byte(s) \"Bit 1\"\n"
  "  Subindex  8: BIT4            rw        0.4 Byte(s) \"Bit 28..31\"\n"
  "  Subindex  9: VISIBLE_STRING  rw        8.0 Byte(s) \"Text\"\n"
  "  Subindex 10: UNSIGNED32      ro        4.0 Byte(s) \"Data ui32b\"\n"
  "  Subindex 11: OCTET_STRING    rw        4.0 Byte(s) \"Octet str\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Info_RECORD_Obj_asmReq_ObjHasNoASM)
{
  Login();

  terminal.Input("OD info 0x3000 asm");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "Object 0x3000: RECORD (DOMAIN) \"Testobject 9\"\n"
  "  Subindex  0: UNSIGNED8       ro        1.0 Byte(s) \"Number of subindices\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  1: BOOLEAN         rw        0.1 Byte(s) \"Data Bool\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  2: INTEGER8        rw        1.0 Byte(s) \"Data i8\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  3: UNSIGNED8       rw        1.0 Byte(s) \"Data ui8\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  4: UNSIGNED32      rw        4.0 Byte(s) \"Data ui32a\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  5: BIT1            rw        0.1 Byte(s) \"Bit 0\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  6: BIT2            rw        0.2 Byte(s) \"Bit 7..8\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  7: BIT1            rw        0.1 Byte(s) \"Bit 1\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  8: BIT4            rw        0.4 Byte(s) \"Bit 28..31\"\n"
  "               No app-specific meta data.\n"
  "  Subindex  9: VISIBLE_STRING  rw        8.0 Byte(s) \"Text\"\n"
  "               No app-specific meta data.\n"
  "  Subindex 10: UNSIGNED32      ro        4.0 Byte(s) \"Data ui32b\"\n"
  "               No app-specific meta data.\n"
  "  Subindex 11: OCTET_STRING    rw        4.0 Byte(s) \"Octet str\"\n"
  "               No app-specific meta data.\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_byteBased)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x1000 = 0xDEADBEEFUL;
  }

  Login();

  terminal.Input("OD read 0x1000:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "3735928559 (0xDEADBEEF)\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_bitBased_0)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_bool = false;
  }

  Login();

  terminal.Input("OD read 0x3000:1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "FALSE\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_bitBased_1)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_bool = true;
  }

  Login();

  terminal.Input("OD read 0x3000:1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "TRUE\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_visiblestring_empty)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_GT(sizeof(rasAndCommonStuff.data0x1010), 0U);
    rasAndCommonStuff.data0x1010[0] = 0U;
  }

  Login();

  terminal.Input("OD read 0x1010:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "\"\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_visiblestring_someChars)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_GT(sizeof(rasAndCommonStuff.data0x1010), 4U);
    strcpy(rasAndCommonStuff.data0x1010, "Test");
  }

  Login();

  terminal.Input("OD read 0x1010:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "\"Test\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_visiblestring_full)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(sizeof(rasAndCommonStuff.data0x1010), 32U + 1U);
    memset(rasAndCommonStuff.data0x1010, 'x', 32U);
  }

  Login();

  terminal.Input("OD read 0x1010:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n"
  "\"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"\n"
  ">\n", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_IndexNotExisting)
{
  Login();

  terminal.Input("OD read 0x0999:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Error: No object with given index\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_SubIndexNotExisting)
{
  Login();

  terminal.Input("OD read 0x1000:1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Error: Subindex does not exist\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_InvalidParams1)
{
  Login();

  terminal.Input("OD read 0x1001:b");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Read_InvalidParams2)
{
  Login();

  terminal.Input("OD read 0x1001:0 3");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_byteBased)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x1000 = 0U;
  }

  Login();

  terminal.Input("OD write 0x1000:0 0x12345678");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>\n", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_EQ(rasAndCommonStuff.data0x1000, 0x12345678UL);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_bitBased)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_bool = false;
  }

  Login();

  terminal.Input("OD write 0x3000:1 true");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>\n", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_TRUE(rasAndCommonStuff.data0x3000.data_bool);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_visiblestring_empty)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_GT(sizeof(rasAndCommonStuff.data0x1010), 4U);
    strcpy(rasAndCommonStuff.data0x1010, "Test");
  }

  Login();

  terminal.Input("OD write 0x1010:0 \"\"");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>\n", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_TRUE(rasAndCommonStuff.data0x1010[0] == 0);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_visiblestring_someChars)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_GT(sizeof(rasAndCommonStuff.data0x1010), 4U);
    strcpy(rasAndCommonStuff.data0x1010, "Test");
  }

  Login();

  terminal.Input("OD write 0x1010:0 \"ABCDEF\"");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>\n", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(rasAndCommonStuff.data0x1010[6], 0);
    EXPECT_STREQ(rasAndCommonStuff.data0x1010, "ABCDEF");
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_visiblestring_full)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(sizeof(rasAndCommonStuff.data0x1010), 33U) << "Testcase needs to be updated to size of data0x1010";
    rasAndCommonStuff.data0x1010[0] = 0;
  }

  Login();

  terminal.Input("OD write 0x1010:0 \"1234567890abcdefghij123456789012\"");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>\n", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(rasAndCommonStuff.data0x1010[32], 0);
    EXPECT_STREQ(rasAndCommonStuff.data0x1010, "1234567890abcdefghij123456789012");
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_visiblestring_tooManyChars)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(sizeof(rasAndCommonStuff.data0x1010), 33U) << "Testcase needs to be updated to size of data0x1010";
    strcpy(rasAndCommonStuff.data0x1010, "Test");
  }

  Login();

  terminal.Input("OD write 0x1010:0 \"1234567890abcdefghij123456789012X\"");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*length of service parameter too large*", false));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(rasAndCommonStuff.data0x1010[4], 0);
    EXPECT_STREQ(rasAndCommonStuff.data0x1010, "Test");
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_octetstring)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_octectstring[0] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[1] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[2] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[3] = 0U;
  }

  Login();

  terminal.Input("OD write 0x3000:11 DE AD BE EF");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>\n", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[0], 0xDEU);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[1], 0xADU);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[2], 0xBEU);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[3], 0xEFU);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_octetstring_tooManyBytes)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_octectstring[0] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[1] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[2] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[3] = 0U;
  }

  Login();

  terminal.Input("OD write 0x3000:11 DE AD BE EF 55");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*length of service parameter too large*", false));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[0], 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[1], 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[2], 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[3], 0U);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_octetstring_tooFewBytes)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_octectstring[0] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[1] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[2] = 0U;
    rasAndCommonStuff.data0x3000.data_octectstring[3] = 0U;
  }

  Login();

  terminal.Input("OD write 0x3000:11 DE AD BE");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*length of service parameter too small*", false));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[0], 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[1], 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[2], 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[3], 0U);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_IndexNotExisting)
{
  Login();

  terminal.Input("OD write 0x0999:0 5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Error: No object with given index\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_SubIndexNotExisting)
{
  Login();

  terminal.Input("OD write 0x1000:1 5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Sub-index does not exist*", false) ||
              gpcc::string::TestSimplePatternMatch(str, "*Subindex is not existing or empty*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_InvalidParams1)
{
  Login();

  terminal.Input("OD write 0x1001:b 5");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, Write_InvalidParams2)
{
  Login();

  terminal.Input("OD write 0x1000:0 3 4");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_record)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_bool = true;
    rasAndCommonStuff.data0x3000.data_i8 = 55;
    rasAndCommonStuff.data0x3000.data_ui8 = 200;
    rasAndCommonStuff.data0x3000.data_ui32a = 0xDEADBEEFUL;
    rasAndCommonStuff.data0x3000.data_bitX[0] = 0x00U;
    rasAndCommonStuff.data0x3000.data_bitX[1] = 0x00U;
    rasAndCommonStuff.data0x3000.data_bitX[2] = 0x00U;
    rasAndCommonStuff.data0x3000.data_bitX[3] = 0x00U;
    rasAndCommonStuff.data0x3000.data_visiblestring[0] = 'A';
    rasAndCommonStuff.data0x3000.data_visiblestring[1] = 'B';
    rasAndCommonStuff.data0x3000.data_visiblestring[2] = 'C';
    rasAndCommonStuff.data0x3000.data_visiblestring[3] = 'D';
    rasAndCommonStuff.data0x3000.data_visiblestring[4] = 0;
    rasAndCommonStuff.data0x3000.data_visiblestring[5] = 0;
    rasAndCommonStuff.data0x3000.data_visiblestring[6] = 0;
    rasAndCommonStuff.data0x3000.data_visiblestring[7] = 0;
    rasAndCommonStuff.data0x3000.data_ui32b = 0xCAFEAFFEUL;
    rasAndCommonStuff.data0x3000.data_octectstring[0] = 1;
    rasAndCommonStuff.data0x3000.data_octectstring[1] = 2;
    rasAndCommonStuff.data0x3000.data_octectstring[2] = 3;
    rasAndCommonStuff.data0x3000.data_octectstring[3] = 4;
  }

  Login();

  terminal.Input("OD caread 0x3000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*\nSI 0 : 11\n"
  "SI 1 : TRUE\n"
  "SI 2 : 55\n"
  "SI 3 : 200 (0xC8)\n"
  "SI 4 : 3735928559 (0xDEADBEEF)\n"
  "SI 5 : 0b0\n"
  "SI 6 : 0b00\n"
  "SI 7 : 0b0\n"
  "SI 8 : 0b0000\n"
  "SI 9 : \"ABCD\"\n"
  "SI 10: 3405688830 (0xCAFEAFFE)\n"
  "SI 11: (hex) 01 02 03 04\n"
  ">\n*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_record_verbose)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000.data_bool = true;
    rasAndCommonStuff.data0x3000.data_i8 = 55;
    rasAndCommonStuff.data0x3000.data_ui8 = 200;
    rasAndCommonStuff.data0x3000.data_ui32a = 0xDEADBEEFUL;
    rasAndCommonStuff.data0x3000.data_bitX[0] = 0x00U;
    rasAndCommonStuff.data0x3000.data_bitX[1] = 0x00U;
    rasAndCommonStuff.data0x3000.data_bitX[2] = 0x00U;
    rasAndCommonStuff.data0x3000.data_bitX[3] = 0x00U;
    rasAndCommonStuff.data0x3000.data_visiblestring[0] = 'A';
    rasAndCommonStuff.data0x3000.data_visiblestring[1] = 'B';
    rasAndCommonStuff.data0x3000.data_visiblestring[2] = 'C';
    rasAndCommonStuff.data0x3000.data_visiblestring[3] = 'D';
    rasAndCommonStuff.data0x3000.data_visiblestring[4] = 0;
    rasAndCommonStuff.data0x3000.data_visiblestring[5] = 0;
    rasAndCommonStuff.data0x3000.data_visiblestring[6] = 0;
    rasAndCommonStuff.data0x3000.data_visiblestring[7] = 0;
    rasAndCommonStuff.data0x3000.data_ui32b = 0xCAFEAFFEUL;
    rasAndCommonStuff.data0x3000.data_octectstring[0] = 1;
    rasAndCommonStuff.data0x3000.data_octectstring[1] = 2;
    rasAndCommonStuff.data0x3000.data_octectstring[2] = 3;
    rasAndCommonStuff.data0x3000.data_octectstring[3] = 4;
  }

  Login();

  terminal.Input("OD caread 0x3000 v");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // look for one line to see if all informaton is present
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nSI 0*UNSIGNED8*Number*: 11\n*", true));

  // look for all the data
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*11\n"
  "*TRUE\n"
  "*55\n"
  "*200 (0xC8)\n"
  "*(0xDEADBEEF)\n"
  "*0b0\n"
  "*0b00\n"
  "*0b0\n"
  "*0b0000\n"
  "*\"ABCD\"\n"
  "*(0xCAFEAFFE)\n"
  "*01 02 03 04\n"
  ">\n*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_array)
{
  rasAndCommonStuff.Set0x2000_SI0(3);
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x2000[0] = 12;
    rasAndCommonStuff.data0x2000[1] = 13;
    rasAndCommonStuff.data0x2000[2] = 14;
  }

  Login();

  terminal.Input("OD caread 0x2000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*\nSI 0: 3\n"
  "SI 1: 12 (0x0C)\n"
  "SI 2: 13 (0x0D)\n"
  "SI 3: 14 (0x0E)\n"
  ">\n*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_array_verbose)
{
  rasAndCommonStuff.Set0x2000_SI0(3);
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x2000[0] = 12;
    rasAndCommonStuff.data0x2000[1] = 13;
    rasAndCommonStuff.data0x2000[2] = 14;
  }

  Login();

  terminal.Input("OD caread 0x2000 v");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // look for one line to see if all informaton is present
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nSI 0*UNSIGNED8*Number*: 3\n*", true));

  // look for all the data
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str,
  "*12 (0x0C)\n"
  "*13 (0x0D)\n"
  "*14 (0x0E)\n"
  ">\n*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_array_si0zero)
{
  rasAndCommonStuff.Set0x2000_SI0(0);

  Login();

  terminal.Input("OD caread 0x2000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nSI 0: 0\n>*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_array_si0zero_verbose)
{
  rasAndCommonStuff.Set0x2000_SI0(0);

  Login();

  terminal.Input("OD caread 0x2000 v");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nSI 0*UNSIGNED8*Number*: 0\n>*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_variable)
{
  Login();

  terminal.Input("OD caread 0x1000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Unsupported access to an object*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_variable_verbose)
{
  Login();

  terminal.Input("OD caread 0x1000 v");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Unsupported access to an object*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_IndexNotExisting)
{
  Login();

  terminal.Input("OD caread 0x0999");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Error: No object with given index\n*", false));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CARead_InvalidParams1)
{
  Login();

  terminal.Input("OD caread 0x1001:0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Invalid arguments. Try 'OD help'*", true));
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CAWrite_record)
{
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    rasAndCommonStuff.data0x3000 = TestbenchBase::Data0x3000();
    rasAndCommonStuff.data0x3000.data_bitX[0] = 0x01U;
  }

  Login();

  terminal.Input("OD cawrite 0x3000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 1
  terminal.Input("TRUE");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 2
  terminal.Input("55");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 3
  terminal.Input("200");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 4
  terminal.Input("0xDEADBEEF");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 5
  terminal.Input("0b0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 6
  terminal.Input("0b11");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 7
  terminal.Input("0b1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 8
  terminal.Input("0b1011");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 9
  terminal.Input("\"ABCD\"");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 10 - skipped - pure RO

  // SI 11
  terminal.Input("01 02 03 04");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.Input("y");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // look for OK
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>*", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    EXPECT_TRUE(rasAndCommonStuff.data0x3000.data_bool);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_i8, 55);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_ui8, 200);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_ui32a, 0xDEADBEEFUL);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_bitX[0], 0x82U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_bitX[1], 0x01U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_bitX[2], 0x00U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_bitX[3], 0xB0U);
    ASSERT_EQ(rasAndCommonStuff.data0x3000.data_visiblestring[4], 0);
    EXPECT_STREQ(rasAndCommonStuff.data0x3000.data_visiblestring, "ABCD");
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_ui32b, 0U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[0], 1U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[1], 2U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[2], 3U);
    EXPECT_EQ(rasAndCommonStuff.data0x3000.data_octectstring[3], 4U);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CAWrite_array)
{
  rasAndCommonStuff.Set0x2000_SI0(100);
  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    memset(rasAndCommonStuff.data0x2000, 0, sizeof(rasAndCommonStuff.data0x2000));
  }

  Login();

  terminal.Input("OD cawrite 0x2000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 0
  terminal.Input("4");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 1
  terminal.Input("0xDE");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 2
  terminal.Input("0xAD");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 3
  terminal.Input("0xBE");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // SI 4
  terminal.Input("0xEF");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.Input("y");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetDroppedOutLinesPlusCurrentScreenContent();

  // look for OK
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nOK\n>*", true));

  {
    gpcc::osal::MutexLocker dataMutexLocker(rasAndCommonStuff.dataMutex);
    ASSERT_EQ(rasAndCommonStuff.GetNbOfSI0x2000(), 1U + 4U);
    EXPECT_EQ(rasAndCommonStuff.data0x2000[0], 0xDEU);
    EXPECT_EQ(rasAndCommonStuff.data0x2000[1], 0xADU);
    EXPECT_EQ(rasAndCommonStuff.data0x2000[2], 0xBEU);
    EXPECT_EQ(rasAndCommonStuff.data0x2000[3], 0xEFU);
  }
}

TEST_F(gpcc_cood_CLIAdapterForCANOpen_TestsF, CAWrite_variable)
{
  Login();

  terminal.Input("OD cawrite 0x1000");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto const str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\nObject type not supported.\n>*", false));
}

} // namespace cood
} // namespace gpcc_tests
