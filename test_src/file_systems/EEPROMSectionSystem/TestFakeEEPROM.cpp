/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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


#include "gtest/gtest.h"
#include "FakeEEPROM.hpp"
#include <cstring>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

using namespace testing;

// Test fixture for FakeEEPROM. Provides an UUT with 4kB RAM and page size zero.
class GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests: public Test
{
  public:
    GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests(void);

  protected:
    FakeEEPROM uut;

    void SetUp(void) override;
    void TearDown(void) override;
};

GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests::GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests(void)
: Test()
, uut(4 * 1024, 0)
{
}

void GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests::SetUp(void)
{
}
void GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests::TearDown(void)
{
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, Constructor)
{
  ASSERT_EQ(4096U, uut.GetSize());
  ASSERT_EQ(0U, uut.GetPageSize());

  FakeEEPROM uut2(1024, 128);
  ASSERT_EQ(1024U, uut2.GetSize());
  ASSERT_EQ(128U, uut2.GetPageSize());

  FakeEEPROM uut3(0, 128);
  ASSERT_EQ(0U, uut3.GetSize());
  ASSERT_EQ(128U, uut3.GetPageSize());

  EXPECT_THROW(FakeEEPROM uut4(16,128), std::invalid_argument);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, CopyConstructor)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.Write(12, sizeof(data), data);

  FakeEEPROM uut2(uut);

  uut.Write(14, sizeof(data), data);
  uut2.Write(10, sizeof(data), data);

  uint8_t readBuf[4];

  uut.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[0], readBuf[2]);
  ASSERT_EQ(data[1], readBuf[3]);
  uut.Read(14, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);

  uut2.Read(10, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
  uut2.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[2], readBuf[0]);
  ASSERT_EQ(data[3], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, MoveConstructor)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.Write(12, sizeof(data), data);

  FakeEEPROM uut2(std::move(uut));
  ASSERT_EQ(0U, uut.GetSize());
  ASSERT_EQ(0U, uut.GetPageSize());

  uint8_t readBuf[4];

  uut2.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, CopyAssignment)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.Write(12, sizeof(data), data);

  FakeEEPROM uut2(32, 0);
  ASSERT_EQ(32U, uut2.GetSize());
  ASSERT_EQ(0U, uut2.GetPageSize());

  uut2 = uut;
  ASSERT_EQ(uut.GetSize(), uut2.GetSize());
  ASSERT_EQ(uut.GetPageSize(), uut2.GetPageSize());

  uut.Write(14, sizeof(data), data);
  uut2.Write(10, sizeof(data), data);

  uint8_t readBuf[4];

  uut.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[0], readBuf[2]);
  ASSERT_EQ(data[1], readBuf[3]);
  uut.Read(14, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);

  uut2.Read(10, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
  uut2.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[2], readBuf[0]);
  ASSERT_EQ(data[3], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, CopyAssignSelf)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.Write(12, sizeof(data), data);

  // this construct avoids complaints from the eclipse indexer
  #define SELFASSIGN uut = uut
  SELFASSIGN;
  #undef SELFASSIGN

  uint8_t readBuf[4];

  uut.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, MoveAssignment)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.Write(12, sizeof(data), data);

  FakeEEPROM uut2(32, 0);
  ASSERT_EQ(32U, uut2.GetSize());
  ASSERT_EQ(0U, uut2.GetPageSize());

  uut2 = std::move(uut);
  ASSERT_EQ(0U, uut.GetSize());
  ASSERT_EQ(0U, uut.GetPageSize());

  ASSERT_EQ(4096U, uut2.GetSize());
  ASSERT_EQ(0U, uut2.GetPageSize());

  uint8_t readBuf[4];

  uut2.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, MoveAssignSelf)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.Write(12, sizeof(data), data);

  uut = std::move(uut);

  uint8_t readBuf[4];

  uut.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, ReadWrite)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.readAccessCnt = 0;
  uut.writeAccessCnt = 0;

  uut.Write(12, sizeof(data), data);
  uut.Write(16, sizeof(data), data);

  ASSERT_EQ(0U, uut.readAccessCnt);
  ASSERT_EQ(2U, uut.writeAccessCnt);

  uint8_t readBuf[4];

  uut.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
  uut.Read(16, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);

  ASSERT_EQ(2U, uut.readAccessCnt);
  ASSERT_EQ(2U, uut.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, ReadOutOfBounds)
{
  uint8_t readBuf[4];
  ASSERT_NO_THROW(uut.Read(4092, sizeof(readBuf), readBuf));
  ASSERT_THROW(uut.Read(4093, sizeof(readBuf), readBuf), std::invalid_argument);
  ASSERT_THROW(uut.Read(4094, sizeof(readBuf), readBuf), std::invalid_argument);
  ASSERT_THROW(uut.Read(4095, sizeof(readBuf), readBuf), std::invalid_argument);
  ASSERT_THROW(uut.Read(4096, sizeof(readBuf), readBuf), std::invalid_argument);
  ASSERT_THROW(uut.Read(4097, sizeof(readBuf), readBuf), std::invalid_argument);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, WriteOutOfBounds)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  ASSERT_NO_THROW(uut.Write(4092, sizeof(data), data));
  ASSERT_THROW(uut.Write(4093, sizeof(data), data), std::invalid_argument);
  ASSERT_THROW(uut.Write(4094, sizeof(data), data), std::invalid_argument);
  ASSERT_THROW(uut.Write(4095, sizeof(data), data), std::invalid_argument);
  ASSERT_THROW(uut.Write(4096, sizeof(data), data), std::invalid_argument);
  ASSERT_THROW(uut.Write(4097, sizeof(data), data), std::invalid_argument);

  uint8_t readBuf[4];
  ASSERT_NO_THROW(uut.Read(4092, sizeof(readBuf), readBuf));
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, ReadWriteAndCheck)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  uut.WriteAndCheck(12, sizeof(data), data, nullptr);
  uut.WriteAndCheck(16, sizeof(data), data, nullptr);

  uint8_t readBuf[4];

  uut.Read(12, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
  uut.Read(16, sizeof(readBuf), readBuf);
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, WriteAndCheckOutOfBounds)
{
  uint8_t const data[4] = { 0x15, 0xB4, 0x89, 0x01 };

  ASSERT_NO_THROW(uut.WriteAndCheck(4092, sizeof(data), data, nullptr));
  ASSERT_THROW(uut.WriteAndCheck(4093, sizeof(data), data, nullptr), std::invalid_argument);
  ASSERT_THROW(uut.WriteAndCheck(4094, sizeof(data), data, nullptr), std::invalid_argument);
  ASSERT_THROW(uut.WriteAndCheck(4095, sizeof(data), data, nullptr), std::invalid_argument);
  ASSERT_THROW(uut.WriteAndCheck(4096, sizeof(data), data, nullptr), std::invalid_argument);
  ASSERT_THROW(uut.WriteAndCheck(4097, sizeof(data), data, nullptr), std::invalid_argument);

  uint8_t readBuf[4];
  ASSERT_NO_THROW(uut.Read(4092, sizeof(readBuf), readBuf));
  ASSERT_EQ(data[0], readBuf[0]);
  ASSERT_EQ(data[1], readBuf[1]);
  ASSERT_EQ(data[2], readBuf[2]);
  ASSERT_EQ(data[3], readBuf[3]);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, Read_StimulatedException)
{
  uut.readAccessesTillThrow = 2;

  uint8_t buffer[8];
  ASSERT_NO_THROW(uut.Read(0, 8, buffer));
  ASSERT_THROW(uut.Read(0, 8, buffer), std::exception);
  ASSERT_NO_THROW(uut.Read(0, 8, buffer));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, Write_StimulatedException)
{
  uut.writeAccessesTillThrow = 2;

  uint8_t buffer[8];
  for (size_t i = 0; i < sizeof(buffer); i++)
    buffer[i] = i;

  ASSERT_NO_THROW(uut.Write(0, 8, buffer));
  ASSERT_THROW(uut.Write(0, 8, buffer), std::exception);
  ASSERT_NO_THROW(uut.Write(0, 8, buffer));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, WriteAndCheck_StimulatedException)
{
  uut.writeAccessesTillThrow = 2;

  uint8_t buffer[8];
  for (size_t i = 0; i < sizeof(buffer); i++)
    buffer[i] = i;

  ASSERT_NO_THROW(uut.WriteAndCheck(0, 8, buffer, nullptr));
  ASSERT_THROW(uut.WriteAndCheck(0, 8, buffer, nullptr), std::exception);
  ASSERT_NO_THROW(uut.WriteAndCheck(0, 8, buffer, nullptr));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, WriteAndCheck_StimulatedFail)
{
  uut.writeAndCheckAccessTillFailure = 2;

  uint8_t buffer[8];
  for (size_t i = 0; i < sizeof(buffer); i++)
    buffer[i] = i;

  ASSERT_TRUE(uut.WriteAndCheck(0, 8, buffer, nullptr));
  ASSERT_FALSE(uut.WriteAndCheck(0, 8, buffer, nullptr));
  ASSERT_TRUE(uut.WriteAndCheck(0, 8, buffer, nullptr));
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, Undo)
{
  uint8_t const data1[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t const data2[] = { 0x4D, 0x2E };
  uint8_t const data3[] = { 0x23, 0xF6 };

  uut.SetEnableUndo(true);
  uut.Write(0, sizeof(data1), data1);
  uut.Write(0, sizeof(data2), data2);
  uut.Write(4, sizeof(data3), data3);

  uint8_t const expectedData1[] = { 0x4D, 0x2E, 0x82, 0xA6, 0x23, 0xF6, 0x32, 0x1A };
  uint8_t readData[sizeof(expectedData1)];
  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);

  uut.Undo(2);

  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(data1, readData, sizeof(readData)) == 0);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, UndoZero)
{
  uint8_t const data1[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t const data2[] = { 0x4D, 0x2E };
  uint8_t const data3[] = { 0x23, 0xF6 };

  uut.SetEnableUndo(true);
  uut.Write(0, sizeof(data1), data1);
  uut.Write(0, sizeof(data2), data2);
  uut.Write(4, sizeof(data3), data3);

  uint8_t const expectedData1[] = { 0x4D, 0x2E, 0x82, 0xA6, 0x23, 0xF6, 0x32, 0x1A };
  uint8_t readData[sizeof(expectedData1)];
  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);

  uut.Undo(0);

  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, UndoButDisabled)
{
  uint8_t const data1[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t const data2[] = { 0x4D, 0x2E };
  uint8_t const data3[] = { 0x23, 0xF6 };

  uut.SetEnableUndo(true);
  uut.Write(0, sizeof(data1), data1);
  uut.Write(0, sizeof(data2), data2);
  uut.Write(4, sizeof(data3), data3);

  uint8_t const expectedData1[] = { 0x4D, 0x2E, 0x82, 0xA6, 0x23, 0xF6, 0x32, 0x1A };
  uint8_t readData[sizeof(expectedData1)];
  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);

  uut.SetEnableUndo(false);
  ASSERT_THROW(uut.Undo(2), std::logic_error);

  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, UndoDisableClearsUndoHistory)
{
  uint8_t const data1[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t const data2[] = { 0x4D, 0x2E };
  uint8_t const data3[] = { 0x23, 0xF6 };

  uut.SetEnableUndo(true);
  uut.Write(0, sizeof(data1), data1);
  uut.Write(0, sizeof(data2), data2);
  uut.Write(4, sizeof(data3), data3);

  uint8_t const expectedData1[] = { 0x4D, 0x2E, 0x82, 0xA6, 0x23, 0xF6, 0x32, 0x1A };
  uint8_t readData[sizeof(expectedData1)];
  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);

  uut.SetEnableUndo(false);
  uut.SetEnableUndo(true);
  ASSERT_THROW(uut.Undo(2), std::logic_error);
  ASSERT_THROW(uut.Undo(1), std::logic_error);
  ASSERT_NO_THROW(uut.Undo(0));

  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, ClearUndo)
{
  uint8_t const data1[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t const data2[] = { 0x4D, 0x2E };
  uint8_t const data3[] = { 0x23, 0xF6 };

  uut.SetEnableUndo(true);
  uut.Write(0, sizeof(data1), data1);
  uut.Write(0, sizeof(data2), data2);
  uut.ClearUndo();
  uut.Write(4, sizeof(data3), data3);

  uint8_t const expectedData1[] = { 0x4D, 0x2E, 0x82, 0xA6, 0x23, 0xF6, 0x32, 0x1A };
  uint8_t const expectedData2[] = { 0x4D, 0x2E, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t readData[sizeof(expectedData1)];
  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData1, readData, sizeof(readData)) == 0);

  ASSERT_THROW(uut.Undo(2), std::invalid_argument);
  ASSERT_NO_THROW(uut.Undo(1));

  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData2, readData, sizeof(readData)) == 0);

  ASSERT_THROW(uut.Undo(1), std::invalid_argument);
  ASSERT_NO_THROW(uut.Undo(0));

  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData2, readData, sizeof(readData)) == 0);
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, Invalidate)
{
  uint8_t const data[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };

  uut.Write(0, sizeof(data), data);
  uut.Invalidate(2, 2);

  uint8_t const expectedData[] = { 0x12, 0x24, 0x28, 0x0C, 0x78, 0x1C, 0x32, 0x1A };
  uint8_t readData[sizeof(expectedData)];
  uut.Read(0, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(expectedData, readData, sizeof(readData)) == 0);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_FakeEEPROM_Tests, InvalidateOutOfBounds)
{
  uint8_t const data[]  = { 0x12, 0x24, 0x82, 0xA6, 0x78, 0x1C, 0x32, 0x1A };

  uut.Write(4096-8, sizeof(data), data);
  ASSERT_THROW(uut.Invalidate(4095, 2), std::invalid_argument);

  uint8_t readData[sizeof(data)];
  uut.Read(4096-8, sizeof(readData), readData);
  ASSERT_TRUE(memcmp(data, readData, sizeof(readData)) == 0);
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
