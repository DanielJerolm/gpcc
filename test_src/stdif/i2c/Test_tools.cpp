/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/stdif/i2c/II2C_Master.hpp>
#include <gpcc/stdif/i2c/tools.hpp>
#include "gtest/gtest.h"
#include <cstring>

using namespace testing;

namespace gpcc_tests
{
namespace StdIf
{

using namespace gpcc::StdIf;

TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, Nullptr)
{
  ASSERT_FALSE(CheckDescriptor(nullptr, 0));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, InvalidAddress1)
{
  // this test checks proper behavior if bit 7 of the address in the first descriptor is set
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x81;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, InvalidAddress2)
{
  // this test checks proper behavior if bit 7 of the address in the second descriptor is set
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x81;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, ReadGCA1)
{
  // this test checks proper behavior if there is an attempt to read from the GC address in the first descriptor
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x0;
  d1.writeNotRead = false;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, ReadGCA2)
{
  // this test checks proper behavior if there is an attempt to read from the GC address in the second descriptor
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x0;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, DataNullptr1)
{
  // this test checks proper behavior if the data pointer in the first descriptor is a nullptr
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = nullptr;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, DataNullptr2)
{
  // this test checks proper behavior if the data pointer in the second descriptor is a nullptr
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = nullptr;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, ZeroBytes1)
{
  // this test checks proper behavior if the size in the first descriptor is zero
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = 0;
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, ZeroBytes2)
{
  // this test checks proper behavior if the size in the second descriptor is zero
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = 0;
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, TooManyBytes1)
{
  // this test checks proper behavior if the size in the first descriptor exceeds the I2C driver's capability
  uint8_t data1[8];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 4));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, TooManyBytes2)
{
  // this test checks proper behavior if the size in the second descriptor exceeds the I2C driver's capability
  uint8_t data1[4];
  uint8_t data2[8];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 4));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, pNextToSelf1)
{
  // this test checks proper behavior if the next-pointer in the first descriptor references to itself
  uint8_t data1[4];

  II2C_Master::stI2CTransferDescriptor_t d1;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d1;
  d1.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, pNextToSelf2)
{
  // this test checks proper behavior if the next-pointer in the second descriptor references to itself
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = &d2;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, ScatteredTransferAddressChanges)
{
  // this test checks proper behavior if the address among a scattered transfer is not the same
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x12;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, ScatteredTransferRWChanges)
{
  // this test checks proper behavior if the R/W-mode among a scattered transfer is not the same
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_FALSE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_SingleRead)
{
  uint8_t data1[4];

  II2C_Master::stI2CTransferDescriptor_t d1;
  d1.address = 0x11;
  d1.writeNotRead = false;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = nullptr;
  d1.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_SingleWrite)
{
  uint8_t data1[4];

  II2C_Master::stI2CTransferDescriptor_t d1;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = nullptr;
  d1.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_ScatteredSingleRead)
{
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2;
  d1.address = 0x11;
  d1.writeNotRead = false;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_ScatteredSingleWrite)
{
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_DoubleRead)
{
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2;
  d1.address = 0x11;
  d1.writeNotRead = false;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x12;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_DoubleWrite)
{
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x12;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_MixedNonScatteredRW1)
{
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x12;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_MixedNonScatteredRW2)
{
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_MixedRW)
{
  uint8_t data1[4];
  uint8_t data2[4];
  uint8_t data3[4];
  uint8_t data4[4];
  uint8_t data5[4];

  II2C_Master::stI2CTransferDescriptor_t d1,d2,d3,d4,d5;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = false;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = &d3;
  d2.scattered = true;

  d3.address = 0x11;
  d3.writeNotRead = false;
  d3.pData = data3;
  d3.nBytes = sizeof(data3);
  d3.pNext = &d4;
  d3.scattered = false;

  d4.address = 0x12;
  d4.writeNotRead = true;
  d4.pData = data4;
  d4.nBytes = sizeof(data4);
  d4.pNext = &d5;
  d4.scattered = false;

  d5.address = 0x13;
  d5.writeNotRead = false;
  d5.pData = data5;
  d5.nBytes = sizeof(data5);
  d5.pNext = nullptr;
  d5.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}
TEST(GPCC_StdIf_I2CTools_CheckDescriptor_Tests, OK_WriteGCA)
{
  // this test checks proper behavior if the GC address shall be written
  uint8_t data1[4];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x0;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x0;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_TRUE(CheckDescriptor(&d1, 16));
}

TEST(GPCC_StdIf_I2CTools_DetermineTotalTransferSize_Tests, MaxSizeExceeded1)
{
  uint8_t data1[8];
  uint8_t data2[4];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_EQ(5U, DetermineTotalTransferSize(&d1, 4U));
}
TEST(GPCC_StdIf_I2CTools_DetermineTotalTransferSize_Tests, MaxSizeExceeded2)
{
  uint8_t data1[4];
  uint8_t data2[8];

  II2C_Master::stI2CTransferDescriptor_t d1, d2;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = nullptr;
  d2.scattered = false;

  ASSERT_EQ(5U, DetermineTotalTransferSize(&d1, 4U));
}
TEST(GPCC_StdIf_I2CTools_DetermineTotalTransferSize_Tests, OK_SingleTransfer)
{
  uint8_t data1[4];

  II2C_Master::stI2CTransferDescriptor_t d1;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = nullptr;
  d1.scattered = false;

  ASSERT_EQ(4U, DetermineTotalTransferSize(&d1, 128U));
}
TEST(GPCC_StdIf_I2CTools_DetermineTotalTransferSize_Tests, OK_SingleScatteredTransfer)
{
  uint8_t data1[4];
  uint8_t data2[12];
  uint8_t data3[8];

  II2C_Master::stI2CTransferDescriptor_t d1, d2, d3;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = &d3;
  d2.scattered = true;

  d3.address = 0x11;
  d3.writeNotRead = true;
  d3.pData = data3;
  d3.nBytes = sizeof(data3);
  d3.pNext = nullptr;
  d3.scattered = false;

  ASSERT_EQ(24U, DetermineTotalTransferSize(&d1, 128U));
}
TEST(GPCC_StdIf_I2CTools_DetermineTotalTransferSize_Tests, OK_MultiTransfers)
{
  uint8_t data1[4];
  uint8_t data2[8];
  uint8_t data3[6];

  II2C_Master::stI2CTransferDescriptor_t d1, d2, d3;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = false;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = &d3;
  d2.scattered = false;

  d3.address = 0x11;
  d3.writeNotRead = true;
  d3.pData = data3;
  d3.nBytes = sizeof(data3);
  d3.pNext = nullptr;
  d3.scattered = false;

  ASSERT_EQ(4U, DetermineTotalTransferSize(&d1, 128U));
}
TEST(GPCC_StdIf_I2CTools_DetermineTotalTransferSize_Tests, OK_MultiTransfersFirstScattered)
{
  uint8_t data1[4];
  uint8_t data2[8];
  uint8_t data3[6];

  II2C_Master::stI2CTransferDescriptor_t d1, d2, d3;
  d1.address = 0x11;
  d1.writeNotRead = true;
  d1.pData = data1;
  d1.nBytes = sizeof(data1);
  d1.pNext = &d2;
  d1.scattered = true;

  d2.address = 0x11;
  d2.writeNotRead = true;
  d2.pData = data2;
  d2.nBytes = sizeof(data2);
  d2.pNext = &d3;
  d2.scattered = false;

  d3.address = 0x11;
  d3.writeNotRead = true;
  d3.pData = data3;
  d3.nBytes = sizeof(data3);
  d3.pNext = nullptr;
  d3.scattered = false;

  ASSERT_EQ(12U, DetermineTotalTransferSize(&d1, 128U));
}

} // namespace StdIf
} // namespace gpcc_tests
