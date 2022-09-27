/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/RequestBase.hpp>
#include <gpcc/cood/exceptions.hpp>
#include <gpcc/cood/ObjectARRAY.hpp>
#include <gpcc/cood/ObjectRECORD.hpp>
#include <gpcc/cood/ObjectVAR.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/stream/MemStreamReader.hpp>
#include <gpcc/stream/MemStreamWriter.hpp>
#include <gpcc/string/tools.hpp>
#include "test_src/cood/ObjectVARwithASM.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <limits>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::ObjectInfoResponse;
using gpcc::cood::ResponseBase;
using gpcc::cood::RequestBase;
using gpcc::cood::ReturnStackItem;
using gpcc::cood::ObjectARRAY;
using gpcc::cood::ObjectRECORD;
using gpcc::cood::ObjectVAR;
using gpcc::cood::Object;
using gpcc::cood::SDOAbortCode;
using gpcc::cood::DataType;
using gpcc::container::IntrusiveDList;


// Test fixture for testing class ObjectInfoResponse.
// Services offered by base class ResponseBase are tested in TestResponseBase.cpp.
class gpcc_cood_ObjectInfoResponse_TestsF: public Test
{
  public:
    gpcc_cood_ObjectInfoResponse_TestsF(void);

  protected:
    // RECORD object description: All RW, one gap.
    static ObjectRECORD::SubIdxDescr const recordObjectSiDescr[12];

    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 8192U;

    // Minimum response size that allows to encapsulate one subindex description (without names).
    static size_t const minimumResponseSize = 3U + 13U + 6U;

    // Offset of "maxNbOfSubindices" in binary (without names).
    static size_t const offsetOfMaxNbOfSubindices = 3U + 8U;

    // Offset of "firstSubindex" in binary (without names).
    static size_t const offsetOfFirstSubindex = 3U + 10U;

    // Offset of number of subindex descriptions in binary (without names).
    static size_t const offsetOfNbOfSI = 3U + 11U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<ReturnStackItem> emptyReturnStack;
    std::vector<ReturnStackItem> const twoItemReturnStack;

    gpcc::osal::Mutex objDataMutex;

    uint8_t u8;
    std::unique_ptr<ObjectVAR> spObjVAR;
    std::unique_ptr<ObjectVARwithASM> spObjVARwithASM;

    uint16_t u16arr[255];
    std::unique_ptr<ObjectARRAY> spObjArrayM1;   // maxSubindex = 1
    std::unique_ptr<ObjectARRAY> spObjArrayM13;  // maxSubindex = 13
    std::unique_ptr<ObjectARRAY> spObjArrayM256; // maxSubindex = 256

    struct RecordData
    {
      bool data_bool;
      int8_t data_i8;
      uint8_t data_ui8;
      uint32_t data_ui32a;
      uint8_t data_bitX[4];
      char data_visiblestring[8];
      uint32_t data_ui32b;
      uint8_t data_octectstring[4];
    } recordData;

    std::unique_ptr<ObjectRECORD> spObjRecord;

    std::unique_ptr<ObjectInfoResponse> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;

    void CheckObjectMetaData(ObjectInfoResponse const & oir, Object const & obj);
    void CheckObjectMetaDataForLogicError(ObjectInfoResponse const & oir);
    void CheckSubindexMetaDataForLogicError(ObjectInfoResponse const & oir, uint8_t const si);
    void CheckSubindexMetaDataForLogicError(ObjectInfoResponse const & oir, uint8_t const first_si, uint8_t const last_si);
    void CheckSubindexMetaDataForSubindexNotExistingError(ObjectInfoResponse const & oir, uint8_t const si);
    void CheckSubindexMetaDataForSubindexNotExistingError(ObjectInfoResponse const & oir, uint8_t const first_si, uint8_t const last_si);
    void CheckSubindexMetaData(ObjectInfoResponse const & oir, Object const & obj, uint8_t const si);
    void CheckSubindexMetaData(ObjectInfoResponse const & oir, Object const & obj, uint8_t const first_si, uint8_t const last_si);

    std::unique_ptr<ObjectInfoResponse> SerializeAndDeserialize(ObjectInfoResponse const & oir);
};

ObjectRECORD::SubIdxDescr const gpcc_cood_ObjectInfoResponse_TestsF::recordObjectSiDescr[12] =
{
    // name,        type,                     attributes,               nElements, byteOffset,                                                                    bitOffset
    { "Data Bool",  DataType::boolean,        Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_bool),          0},
    { "Data i8",    DataType::integer8,       Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,      Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_ui8),           0},
    { "Align",      DataType::null,           Object::attr_ACCESS_RW,   8,         0,                                                                             0},
    { "Data ui32a", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_ui32a),         0},
    { "Bit 0",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_bitX),          0},
    { "Bit 7..8",   DataType::bit2,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_bitX),          7},
    { "Bit 1",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_bitX),          1},
    { "Bit 28..31", DataType::bit4,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_bitX) + 3,      4},
    { "Text",       DataType::visible_string, Object::attr_ACCESS_RW,   8,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,   Object::attr_ACCESS_RW,   4,         offsetof(gpcc_cood_ObjectInfoResponse_TestsF::RecordData, data_octectstring),  0}
};

size_t const gpcc_cood_ObjectInfoResponse_TestsF::stdMaxResponseSize;
size_t const gpcc_cood_ObjectInfoResponse_TestsF::minimumResponseSize;
size_t const gpcc_cood_ObjectInfoResponse_TestsF::offsetOfMaxNbOfSubindices;
size_t const gpcc_cood_ObjectInfoResponse_TestsF::offsetOfFirstSubindex;
size_t const gpcc_cood_ObjectInfoResponse_TestsF::offsetOfNbOfSI;

gpcc_cood_ObjectInfoResponse_TestsF::gpcc_cood_ObjectInfoResponse_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, emptyReturnStack()
, twoItemReturnStack{ rsi1, rsi2 }
, objDataMutex()
, u8(0U)
, spObjVAR()
, spObjVARwithASM()
, u16arr()
, spObjArrayM1()
, spObjArrayM13()
, spObjArrayM256()
, spObjRecord()
, spUUT()
{
  spObjVAR = std::make_unique<ObjectVAR>("Test1",
                                         DataType::unsigned8,
                                         1U,
                                         Object::attr_ACCESS_RD,
                                         &u8,
                                         nullptr,
                                         nullptr);

  std::vector<uint8_t> appSpecMetaData{0xDEU, 0xADU, 0xBEU, 0xEFU};
  spObjVARwithASM = std::make_unique<ObjectVARwithASM>("Test2",
                                                       DataType::unsigned8,
                                                       1U,
                                                       Object::attr_ACCESS_RD,
                                                       &u8,
                                                       nullptr,
                                                       nullptr,
                                                       std::move(appSpecMetaData));

  spObjArrayM1 = std::make_unique<ObjectARRAY>("Test2_M1",
                                               Object::attr_ACCESS_RD,
                                               0U,
                                               0U,
                                               0U,
                                               DataType::unsigned16,
                                               Object::attr_ACCESS_RW,
                                               &u16arr,
                                               &objDataMutex,
                                               nullptr);

  spObjArrayM13 = std::make_unique<ObjectARRAY>("Test2_M13",
                                                Object::attr_ACCESS_RD,
                                                10U,
                                                0U,
                                                12U,
                                                DataType::unsigned16,
                                                Object::attr_ACCESS_RW,
                                                &u16arr,
                                                &objDataMutex,
                                                nullptr);

  spObjArrayM256 = std::make_unique<ObjectARRAY>("Test2_M256",
                                                 Object::attr_ACCESS_RD,
                                                 10U,
                                                 0U,
                                                 255U,
                                                 DataType::unsigned16,
                                                 Object::attr_ACCESS_RW,
                                                 &u16arr,
                                                 &objDataMutex,
                                                 nullptr);

  spObjRecord = std::make_unique<ObjectRECORD>("Test3",
                                               12U,
                                               &recordData,
                                               sizeof(recordData),
                                               &objDataMutex,
                                               recordObjectSiDescr,
                                               nullptr);
}

void gpcc_cood_ObjectInfoResponse_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectInfoResponse_TestsF::TearDown(void)
{
  spUUT.reset();
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckObjectMetaData(ObjectInfoResponse const & oir, Object const & obj)
{
  EXPECT_EQ(oir.GetObjectCode(), obj.GetObjectCode());
  EXPECT_EQ(oir.GetObjectDataType(), obj.GetObjectDataType());
  if (oir.IsInclusiveNames())
  {
    EXPECT_STREQ(oir.GetObjectName().c_str(), obj.GetObjectName().c_str());
  }
  EXPECT_EQ(oir.GetMaxNbOfSubindices(), obj.GetMaxNbOfSubindices());
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckObjectMetaDataForLogicError(ObjectInfoResponse const & oir)
{
  EXPECT_THROW(oir.GetObjectCode(), std::logic_error);
  EXPECT_THROW(oir.GetObjectDataType(), std::logic_error);
  EXPECT_THROW(oir.GetObjectName(), std::logic_error);
  EXPECT_THROW(oir.GetMaxNbOfSubindices(), std::logic_error);
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckSubindexMetaDataForLogicError(ObjectInfoResponse const & oir,
                                                                             uint8_t const si)
{
  EXPECT_THROW(oir.IsSubIndexEmpty(si), std::logic_error);
  EXPECT_THROW(oir.GetSubIdxDataType(si), std::logic_error);
  EXPECT_THROW(oir.GetSubIdxAttributes(si), std::logic_error);
  EXPECT_THROW(oir.GetSubIdxMaxSize(si), std::logic_error);
  EXPECT_THROW(oir.GetSubIdxName(si), std::logic_error);
  EXPECT_THROW(oir.GetAppSpecificMetaDataSize(si), std::logic_error);
  EXPECT_THROW(oir.GetAppSpecificMetaData(si), std::logic_error);
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckSubindexMetaDataForLogicError(ObjectInfoResponse const & oir,
                                                                             uint8_t const first_si,
                                                                             uint8_t const last_si)
{
  if (HasFailure())
  {
    std::cout << "Skipped CheckSubindexMetaDataForLogicError(...) due to non-fatal failures." << std::endl;
    return;
  }

  for (uint_fast16_t i = first_si; i <= last_si; ++i)
  {
    CheckSubindexMetaDataForLogicError(oir, i);
    if (HasFailure())
    {
      std::cout << "CheckSubindexMetaDataForLogicError(...) failed at subindex " << i << std::endl;
      break;
    }
  }
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckSubindexMetaDataForSubindexNotExistingError(ObjectInfoResponse const & oir,
                                                                                           uint8_t const si)
{
  EXPECT_THROW(oir.IsSubIndexEmpty(si), gpcc::cood::SubindexNotExistingError);
  EXPECT_THROW(oir.GetSubIdxDataType(si), gpcc::cood::SubindexNotExistingError);
  EXPECT_THROW(oir.GetSubIdxAttributes(si), gpcc::cood::SubindexNotExistingError);
  EXPECT_THROW(oir.GetSubIdxMaxSize(si), gpcc::cood::SubindexNotExistingError);
  EXPECT_THROW(oir.GetSubIdxName(si), gpcc::cood::SubindexNotExistingError);
  EXPECT_THROW(oir.GetAppSpecificMetaDataSize(si), gpcc::cood::SubindexNotExistingError);
  EXPECT_THROW(oir.GetAppSpecificMetaData(si), gpcc::cood::SubindexNotExistingError);
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckSubindexMetaDataForSubindexNotExistingError(ObjectInfoResponse const & oir,
                                                                                           uint8_t const first_si,
                                                                                           uint8_t const last_si)
{
  if (HasFailure())
  {
    std::cout << "Skipped CheckSubindexMetaDataForSubindexNotExistingError(...) due to non-fatal failures." << std::endl;
    return;
  }

  for (uint_fast16_t i = first_si; i <= last_si; ++i)
  {
    CheckSubindexMetaDataForSubindexNotExistingError(oir, i);
    if (HasFailure())
    {
      std::cout << "CheckSubindexMetaDataForSubindexNotExistingError(...) failed at subindex " << i << std::endl;
      break;
    }
  }
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckSubindexMetaData(ObjectInfoResponse const & oir,
                                                                Object const & obj,
                                                                uint8_t const si)
{
  EXPECT_EQ(oir.IsSubIndexEmpty(si),     obj.IsSubIndexEmpty(si));
  EXPECT_EQ(oir.GetSubIdxDataType(si),   obj.GetSubIdxDataType(si));
  EXPECT_EQ(oir.GetSubIdxAttributes(si), obj.GetSubIdxAttributes(si));
  EXPECT_EQ(oir.GetSubIdxMaxSize(si),    obj.GetSubIdxMaxSize(si));

  if (oir.IsInclusiveNames())
  {
    EXPECT_EQ(oir.GetSubIdxName(si),     obj.GetSubIdxName(si));
  }

  if (oir.IsInclusiveAppSpecificMetaData())
  {
    auto const asm_size_oir = oir.GetAppSpecificMetaDataSize(si);
    auto const asm_size_obj = obj.GetAppSpecificMetaDataSize(si);

    if (asm_size_oir == asm_size_obj)
    {
      if (asm_size_oir != 0U)
      {
        auto const asm_oir = oir.GetAppSpecificMetaData(si);
        auto const asm_obj = obj.GetAppSpecificMetaData(si);
        EXPECT_EQ(asm_oir.size(), asm_size_oir);
        EXPECT_EQ(asm_obj.size(), asm_size_obj);
        EXPECT_TRUE(asm_oir == asm_obj);
      }
    }
    else
    {
      ADD_FAILURE() << "Size of ASM differs";
    }
  }
}

void gpcc_cood_ObjectInfoResponse_TestsF::CheckSubindexMetaData(ObjectInfoResponse const & oir,
                                                                Object const & obj,
                                                                uint8_t const first_si,
                                                                uint8_t const last_si)
{
  if (HasFailure())
  {
    std::cout << "Skipped CheckSubindexMetaData(...) due to non-fatal failures." << std::endl;
    return;
  }

  for (uint_fast16_t i = first_si; i <= last_si; ++i)
  {
    CheckSubindexMetaData(oir, obj, i);
    if (HasFailure())
    {
      std::cout << "CheckSubindexMetaData(...) failed at subindex " << i << std::endl;
      break;
    }
  }
}

std::unique_ptr<ObjectInfoResponse> gpcc_cood_ObjectInfoResponse_TestsF::SerializeAndDeserialize(ObjectInfoResponse const & oir)
{
  size_t const reqSize = oir.GetBinarySize();
  if (reqSize == 0U)
    throw std::logic_error("gpcc_cood_ObjectInfoResponse_TestsF::SerializeAndDeserialize: oir.GetBinarySize() returns zero");

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  // serialize
  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  oir.ToBinary(msw);
  msw.AlignToByteBoundary(false);
  if (msw.GetState() != gpcc::stream::IStreamWriter::States::full)
    throw std::logic_error("gpcc_cood_ObjectInfoResponse_TestsF::SerializeAndDeserialize: msw was not fully used.");
  msw.Close();

  // deserialize
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  auto spUUT2Base = ResponseBase::FromBinary(msr);
  if (msr.GetState() != gpcc::stream::IStreamReader::States::empty)
    throw std::logic_error("gpcc_cood_ObjectInfoResponse_TestsF::SerializeAndDeserialize: Stream was not completely consumed");
  msr.Close();

  // check type and cast to ObjectInfoRequest
  ObjectInfoResponse * const pRet = &dynamic_cast<ObjectInfoResponse&>(*spUUT2Base);
  spUUT2Base.release();

  return std::unique_ptr<ObjectInfoResponse>(pRet);
}


// alias for death tests
using gpcc_cood_ObjectInfoResponse_DeathTestsF = gpcc_cood_ObjectInfoResponse_TestsF;


TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_negativeResult_rejectsStatusOK)
{
  ASSERT_THROW(spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::OK), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_negativeResult)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_FALSE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_THROW((void)spUUT->GetFirstQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetLastQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);

  uint8_t dummy;
  EXPECT_THROW((void)spUUT->IsComplete(&dummy), std::logic_error);

  CheckObjectMetaDataForLogicError(*spUUT);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_ASM1)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0U), 0U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_ASM2)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVARwithASM,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVARwithASM);
  CheckSubindexMetaData(*spUUT, *spObjVARwithASM, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);

  ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(0U), 4U);
  auto appSpecMetaData = spUUT->GetAppSpecificMetaData(0U);
  ASSERT_EQ(appSpecMetaData.size(), 4U);
  EXPECT_EQ(appSpecMetaData[0], 0xDEU);
  EXPECT_EQ(appSpecMetaData[1], 0xADU);
  EXPECT_EQ(appSpecMetaData[2], 0xBEU);
  EXPECT_EQ(appSpecMetaData[3], 0xEFU);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_ASM3)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);

  EXPECT_THROW((void)spUUT->GetAppSpecificMetaDataSize(0U), std::logic_error);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_ASM4)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVARwithASM,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVARwithASM);
  CheckSubindexMetaData(*spUUT, *spObjVARwithASM, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);

  EXPECT_THROW((void)spUUT->GetAppSpecificMetaDataSize(0U), std::logic_error);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_NoNames)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               false, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_NoName_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               false, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_QuerySi0Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjVAR_QuerySi1Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI1..1 has been requested, but SI0 is included, because at least one SI must be in the response

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M1)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M1_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M1_QuerySi1Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI1..1 is requested, but SI0 is included, because there is no SI1 and at least one SI must be
  // included in the response

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M1_QuerySi1Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI1..1 is requested, but SI0 is included, because there is no SI1 and at least one SI must be
  // included in the response

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi0Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi0Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 0U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi1Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // Expectation: SI1..1 is requested, and only SI1 is queried. Though an ARRAY object is queried the subindices
  // are not all the same because application specific meta data is included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 2U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi1Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // Expectation: SI1..1 is requested, but all SIs are included because they are all the same because application
  // specific meta data is not included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi2Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // Expectation: SI2..2 is requested, and only SI2 is queried. Though an ARRAY object is queried the subindices
  // are not all the same because application specific meta data is included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 1U);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT, 3U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi2Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // Expectation: SI2..2 is requested, but all SIs are included because they are all the same because application
  // specific meta data is not included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi255Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               255U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // Expectation: SI255..255 is requested, but only SI12 is included because there must be at least one in the response
  // and because application specific meta data is included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 12U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 11U);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 12U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M13_QuerySi255Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               255U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI255..255 is requested, but all SIs are included because there must be at least one in the response
  // and becasue they are all the same and because application specific meta data is not included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_QuerySi0Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_QuerySi0Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 0U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_QuerySi1Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI1..1 is requested, and only SI1 is included.
  // Inlusion of application specific meta data into the query prevents to treat the SIs all the same.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 1U, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT, 2U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_QuerySi1Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI1..1 is requested, but all SIs are included because they are all the same and because
  // application specific meta data is not included in the query.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 1U, 255U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_QuerySi2Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               2U, 2U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI2..2 is requested, and only SI2 is included in the response because inclusion of application
  // specific meta data prevents treating them all the same.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 1U);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT, 3U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjARR_M256_QuerySi2Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               2U, 2U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI2..2 is requested, but all SIs are included because there must be at least one in the response
  // and becasue they are all the same. Exclusion of application specific meta data from the query allows to treat
  // all SIs the same.

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 1U, 255U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_QuerySi0Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_QuerySi0Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 0U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_QuerySi1Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT, 2U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_QuerySi1Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT, 2U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_QuerySi13Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               13U, 13U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI13..13 is requested, but SI12 is contained in the response because there must be at least one
  // subindex in the response

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 12U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 11U);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, CTOR_ObjRECORD_QuerySi13Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               13U, 13U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  // expectation: SI13..13 is requested, but SI12 is contained in the response because there must be at least one
  // subindex in the response

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 12U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 11U);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_negativeResult)
{
  spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError);

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_FALSE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_THROW((void)spUUT->GetFirstQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetLastQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);

  uint8_t dummy;
  EXPECT_THROW((void)spUUT->IsComplete(&dummy), std::logic_error);

  CheckObjectMetaDataForLogicError(*spUUT);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 255U);

  // check copy
  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_FALSE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_THROW((void)spUUT2->GetFirstQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetLastQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT2->IsComplete(nullptr), std::logic_error);

  EXPECT_THROW((void)spUUT2->IsComplete(&dummy), std::logic_error);

  CheckObjectMetaDataForLogicError(*spUUT2);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_negativeResult)
{
  spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError);

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_FALSE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_THROW((void)spUUT2->GetFirstQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetLastQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT2->IsComplete(nullptr), std::logic_error);

  uint8_t dummy;
  EXPECT_THROW((void)spUUT2->IsComplete(&dummy), std::logic_error);

  CheckObjectMetaDataForLogicError(*spUUT2);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_ObjVAR)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);

  // check copy
  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_ObjVAR_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  ASSERT_TRUE(spUUT->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjVAR);
  CheckSubindexMetaData(*spUUT, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 1U, 255U);

  // check copy
  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_ObjVAR)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_ObjVAR_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_ObjARR_M13)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);

  // check copy
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_ObjARR_M13_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);

  // check copy
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_ObjARR_M13)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_ObjARR_M13_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_ObjARR_M13_QuerySi2Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U, 1U);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT, 3U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);

  // check copy
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 1U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 3U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_ObjARR_M13_QuerySi2Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);

  // check copy
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_ObjARR_M13_QuerySi2Only)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 1U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 3U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_ObjARR_M13_QuerySi2Only_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_Record)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_TRUE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U, 255U);

  // check copy
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Copy_CTOR_Record_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(*spUUT);

  // check that UUT is OK
  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U, 255U);

  // check copy
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_Record)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, Move_CTOR_Record_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = std::make_unique<ObjectInfoResponse>(std::move(*spUUT));

  // check move-created object
  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, ResponseSizeLimitation_1)
{
  ASSERT_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                            0U, 255U,
                                                            false, false,
                                                            minimumResponseSize - 1U,
                                                            0U), std::runtime_error)
    << "Response size should have been too small, but it worked. That was not anticipated.";

  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               false, false,
                                                               minimumResponseSize,
                                                               0U));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 0U);

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT, 1U, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, ResponseSizeLimitation_2)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               false, false,
                                                               minimumResponseSize + 6U,
                                                               0U));

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());
  EXPECT_FALSE(spUUT->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 1U);

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT, 2U, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_negativeResult_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_FALSE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_THROW((void)spUUT2->GetFirstQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetLastQueriedSubindex(), std::logic_error);
  EXPECT_THROW((void)spUUT2->IsComplete(nullptr), std::logic_error);

  uint8_t dummy;
  EXPECT_THROW((void)spUUT2->IsComplete(&dummy), std::logic_error);

  CheckObjectMetaDataForLogicError(*spUUT2);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjVAR_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjVAR_NoNames_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               false, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjVAR_NoNames_NoASM_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 255U,
                                                               false, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetType() == ResponseBase::ResponseTypes::objectInfoResponse);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjVAR_QuerySi0Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjVAR_QuerySi1Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjVAR);
  CheckSubindexMetaData(*spUUT2, *spObjVAR, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M1_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M1_QuerySi1Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M1_QuerySi1Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM1,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM1);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM1, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 1U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi0Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 1U, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi0Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 0U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 1U, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi1Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 2U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi1Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi2Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 1U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 3U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi2Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               2U, 2U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi255Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               255U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 12U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 11U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M13_QuerySi255Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               255U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM13, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_QuerySi0Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_QuerySi0Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               0U, 0U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 1U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_QuerySi1Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 1U, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 2U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_QuerySi1Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 1U, 255U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_QuerySi2Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               2U, 2U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 2U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 2U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 3U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 1U);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 2U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 3U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjARR_M256_QuerySi2Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256,
                                                               2U, 2U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT2, *spObjArrayM256, 1U, 255U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_QuerySi0Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 0U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_QuerySi0Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               0U, 0U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 0U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 1U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 0U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 1U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_QuerySi1Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               1U, 1U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 2U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_QuerySi1Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               1U, 1U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 1U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 1U);
  EXPECT_FALSE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_FALSE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 2U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 1U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 2U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_QuerySi13Only_serializeDeserialize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               13U, 13U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_TRUE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 12U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 11U);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, RespObj_ObjRECORD_QuerySi13Only_serializeDeserialize_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                                               13U, 13U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  EXPECT_EQ(spUUT2->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsInclusiveNames());
  EXPECT_FALSE(spUUT2->IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(spUUT2->GetFirstQueriedSubindex(), 12U);
  EXPECT_EQ(spUUT2->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));

  uint8_t nsi = 83U;
  EXPECT_TRUE(spUUT2->IsComplete(&nsi));
  EXPECT_EQ(nsi, 83U);

  CheckObjectMetaData(*spUUT2, *spObjRecord);
  CheckSubindexMetaDataForLogicError(*spUUT2, 0U, 11U);
  CheckSubindexMetaData(*spUUT2, *spObjRecord, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT2, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, ToString_negativeResult)
{
  spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*object info response*", false));
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*General error*", false));
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, ToString_M13)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, true,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto const s = spUUT->ToString();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*object info response*", false));
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*OK*", false));
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*13 subindex desc*", false));
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, ToString_M13_NoASM)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                                               0U, 255U,
                                                               true, false,
                                                               stdMaxResponseSize,
                                                               3U * ReturnStackItem::binarySize));

  auto const s = spUUT->ToString();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*object info response*", false));
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*OK*", false));
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(s, "*2 subindex desc*", false));
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DeserializeInvalidBinaryData_MaxNbOfSI)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                               0U, 255U,
                                               false, true,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_NE(reqSize, 0U);

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_TRUE(msw.GetState() == gpcc::stream::IStreamWriter::States::full);
  msw.Close();

  // manipulate binary (maxNbOfSubindices = 0)
  spStorage[offsetOfMaxNbOfSubindices + 0U] = 0U;
  spStorage[offsetOfMaxNbOfSubindices + 1U] = 0U;

  // try to deserialize it
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (maxNbOfSubindices = 0x101 (257))
  spStorage[offsetOfMaxNbOfSubindices + 0U] = 1U;
  spStorage[offsetOfMaxNbOfSubindices + 1U] = 1U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DeserializeInvalidBinaryData_FirstSubindex_Record)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 255U,
                                               false, true,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_NE(reqSize, 0U);

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_TRUE(msw.GetState() == gpcc::stream::IStreamWriter::States::full);
  msw.Close();

  // manipulate binary (firstSubindex = 1)
  spStorage[offsetOfFirstSubindex] = 1U;

  // try to deserialize it
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (firstSubindex = 13)
  spStorage[offsetOfFirstSubindex] = 13U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DeserializeInvalidBinaryData_FirstSubindex_Array)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                               0U, 255U,
                                               false, true,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_NE(reqSize, 0U);

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_TRUE(msw.GetState() == gpcc::stream::IStreamWriter::States::full);
  msw.Close();

  // manipulate binary (firstSubindex = 1)
  spStorage[offsetOfFirstSubindex] = 1U;

  // try to deserialize it
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (firstSubindex = 13)
  spStorage[offsetOfFirstSubindex] = 13U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DeserializeInvalidBinaryData_FirstSubindex_Array_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                               0U, 255U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_NE(reqSize, 0U);

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_TRUE(msw.GetState() == gpcc::stream::IStreamWriter::States::full);
  msw.Close();

  // manipulate binary (firstSubindex = 1)
  spStorage[offsetOfFirstSubindex] = 1U;

  // try to deserialize it
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (firstSubindex = 2)
  spStorage[offsetOfFirstSubindex] = 2U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DeserializeInvalidBinaryData_NumberOfSIs_Record)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 255U,
                                               false, true,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_NE(reqSize, 0U);

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_TRUE(msw.GetState() == gpcc::stream::IStreamWriter::States::full);
  msw.Close();

  // manipulate binary (number of included SIs = 0U)
  spStorage[offsetOfNbOfSI + 0U] = 0U;
  spStorage[offsetOfNbOfSI + 1U] = 0U;

  // try to deserialize it
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (number of included SIs = 257)
  spStorage[offsetOfNbOfSI + 0U] = 1U;
  spStorage[offsetOfNbOfSI + 1U] = 1U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (number of included SIs = 14)
  spStorage[offsetOfNbOfSI + 0U] = 14U;
  spStorage[offsetOfNbOfSI + 1U] = 0U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DeserializeInvalidBinaryData_NumberOfSIs_Array_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13,
                                               0U, 255U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_NE(reqSize, 0U);

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  gpcc::stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_TRUE(msw.GetState() == gpcc::stream::IStreamWriter::States::full);
  msw.Close();

  // manipulate binary (number of included SIs = 0U)
  spStorage[offsetOfNbOfSI + 0U] = 0U;
  spStorage[offsetOfNbOfSI + 1U] = 0U;

  // try to deserialize it
  gpcc::stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (number of included SIs = 257)
  spStorage[offsetOfNbOfSI + 0U] = 1U;
  spStorage[offsetOfNbOfSI + 1U] = 1U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();

  // manipulate binary (number of included SIs = 3)
  spStorage[offsetOfNbOfSI + 0U] = 3U;
  spStorage[offsetOfNbOfSI + 1U] = 0U;

  // try to deserialize it
  msr = gpcc::stream::MemStreamReader(spStorage.get(), reqSize, gpcc::stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DefragResponses_OK_Record_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord, 0U, 255U, false, false, minimumResponseSize, 0U);

  uint_fast8_t loops = 0U;
  uint8_t firstSubindex = 0U;
  while (!spUUT->IsComplete(&firstSubindex))
  {
    loops++;

    auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, firstSubindex, 255U, false, false, minimumResponseSize, 0U);
    spUUT->AddFragment(std::move(*spFragment));
  }

  EXPECT_EQ(loops, 12U);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));

  CheckObjectMetaData(*spUUT, *spObjRecord);
  CheckSubindexMetaData(*spUUT, *spObjRecord, 0U, 12U);
  CheckSubindexMetaDataForLogicError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DefragResponses_OK_Array13_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM13, 0U, 255U, false, false, minimumResponseSize, 0U);

  uint_fast8_t loops = 0U;
  uint8_t firstSubindex = 0U;
  while (!spUUT->IsComplete(&firstSubindex))
  {
    loops++;

    auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjArrayM13, firstSubindex, 255U, false, false, minimumResponseSize, 0U);
    spUUT->AddFragment(std::move(*spFragment));
  }

  EXPECT_EQ(loops, 1U);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 12U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));


  CheckObjectMetaData(*spUUT, *spObjArrayM13);
  CheckSubindexMetaData(*spUUT, *spObjArrayM13, 0U, 12U);
  CheckSubindexMetaDataForSubindexNotExistingError(*spUUT, 13U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, DefragResponses_OK_ArrayM256_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjArrayM256, 0U, 255U, false, false, minimumResponseSize, 0U);

  uint_fast8_t loops = 0U;
  uint8_t firstSubindex = 0U;
  while (!spUUT->IsComplete(&firstSubindex))
  {
    loops++;

    auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjArrayM256, firstSubindex, 255U, false, false, minimumResponseSize, 0U);
    spUUT->AddFragment(std::move(*spFragment));
  }

  EXPECT_EQ(loops, 1U);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsInclusiveNames());

  EXPECT_EQ(spUUT->GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(spUUT->GetLastQueriedSubindex(), 255U);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));


  CheckObjectMetaData(*spUUT, *spObjArrayM256);
  CheckSubindexMetaData(*spUUT, *spObjArrayM256, 0U, 255U);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_BadStatus_This_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError);
  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, 5U, 255U, false, false, minimumResponseSize, 0U);
  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::logic_error);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_VariableObject_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjVAR,
                                               0U, 255U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  ASSERT_TRUE(spUUT->IsComplete(nullptr));

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, 5U, 255U, false, false, minimumResponseSize, 0U);
  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::logic_error);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_AlreadyComplete_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 255U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  ASSERT_TRUE(spUUT->IsComplete(nullptr));

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, 5U, 255U, false, false, minimumResponseSize, 0U);
  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::logic_error);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_BadStatus_Other_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 0U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  uint8_t nextSI;
  ASSERT_FALSE(spUUT->IsComplete(&nextSI));
  ASSERT_EQ(nextSI, 1U);

  auto spFragment = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError);
  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_DifferentObjects_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 0U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  uint8_t nextSI;
  ASSERT_FALSE(spUUT->IsComplete(&nextSI));
  ASSERT_EQ(nextSI, 1U);

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjArrayM13, 1U, 1U, false, false, minimumResponseSize, 0U);
  ASSERT_EQ(spFragment->GetResult(), SDOAbortCode::OK);

  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_DifferentInclNames_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 0U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  uint8_t nextSI;
  ASSERT_FALSE(spUUT->IsComplete(&nextSI));
  ASSERT_EQ(nextSI, 1U);

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, 1U, 255U, true, false, stdMaxResponseSize, 0U);
  ASSERT_EQ(spFragment->GetResult(), SDOAbortCode::OK);

  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_DifferentInclASM_NoNames)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 0U,
                                               false, true,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  uint8_t nextSI;
  ASSERT_FALSE(spUUT->IsComplete(&nextSI));
  ASSERT_EQ(nextSI, 1U);

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, 1U, 255U, false, false, stdMaxResponseSize, 0U);
  ASSERT_EQ(spFragment->GetResult(), SDOAbortCode::OK);

  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectInfoResponse_TestsF, AddFragment_Discontinuity_NoASM)
{
  spUUT = std::make_unique<ObjectInfoResponse>(*spObjRecord,
                                               0U, 0U,
                                               false, false,
                                               stdMaxResponseSize,
                                               3U * ReturnStackItem::binarySize);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  uint8_t nextSI;
  ASSERT_FALSE(spUUT->IsComplete(&nextSI));
  ASSERT_EQ(nextSI, 1U);

  auto spFragment = std::make_unique<ObjectInfoResponse>(*spObjRecord, 2U, 255U, false, false, minimumResponseSize, 0U);
  ASSERT_EQ(spFragment->GetResult(), SDOAbortCode::OK);

  ASSERT_THROW(spUUT->AddFragment(std::move(*spFragment)), std::invalid_argument);
}

} // namespace gpcc_tests
} // namespace cood
