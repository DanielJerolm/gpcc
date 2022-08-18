/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/cood/remote_access/requests_and_responses/ResponseBase.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequest.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"
#include <limits>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::Object;
using gpcc::cood::ResponseBase;
using gpcc::cood::RequestBase;
using gpcc::cood::WriteRequest;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for testing class WriteRequest.
// Services offered by base class RequestBase are tested in TestRequestBase.cpp.
class gpcc_cood_WriteRequest_TestsF: public Test
{
  public:
    gpcc_cood_WriteRequest_TestsF(void);

  protected:
    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 1024U;

    // offset of "accessType" in binary
    static size_t const accessTypeOffset = 7U;

    // offset of "subindex" in binary
    static size_t const subIndexOffset = 10U;

    // offset of "permission" in binary
    static size_t const permissionOffset = 11U;

    // offset of data size in binary
    static size_t const dataSizeOffset = 13U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<uint8_t> someData;

    std::unique_ptr<WriteRequest> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_WriteRequest_TestsF::stdMaxResponseSize;
size_t const gpcc_cood_WriteRequest_TestsF::accessTypeOffset;
size_t const gpcc_cood_WriteRequest_TestsF::subIndexOffset;
size_t const gpcc_cood_WriteRequest_TestsF::permissionOffset;
size_t const gpcc_cood_WriteRequest_TestsF::dataSizeOffset;

gpcc_cood_WriteRequest_TestsF::gpcc_cood_WriteRequest_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, someData{ 0x56U, 0x89U }
, spUUT()
{
}

void gpcc_cood_WriteRequest_TestsF::SetUp(void)
{
}

void gpcc_cood_WriteRequest_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_WriteRequest_DeathTestsF = gpcc_cood_WriteRequest_TestsF;

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_OK_singleSubIndex)
{
  auto data = someData;
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize));

  EXPECT_TRUE(data.empty()) <<
    "CTOR has moved stuff from data somewhere. 'data' should be empty now, but it is not!";

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  auto const & refToData = spUUT->GetData();
  EXPECT_TRUE(refToData == someData);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_OK_CA_8bit_SI0)
{
  auto data = someData;
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                                         0x1002U, 0U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize));

  EXPECT_TRUE(data.empty()) <<
    "CTOR has moved stuff from data somewhere. 'data' should be empty now, but it is not!";

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::completeAccess_SI0_8bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  auto const & refToData = spUUT->GetData();
  EXPECT_TRUE(refToData == someData);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_OK_CA_8bit_SI1)
{
  auto data = someData;
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                                         0x1002U, 1U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize));

  EXPECT_TRUE(data.empty()) <<
    "CTOR has moved stuff from data somewhere. 'data' should be empty now, but it is not!";

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::completeAccess_SI0_8bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 1U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  auto const & refToData = spUUT->GetData();
  EXPECT_TRUE(refToData == someData);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_OK_CA_16bit_SI0)
{
  auto data = someData;
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                                         0x1002U, 0U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize));

  EXPECT_TRUE(data.empty()) <<
    "CTOR has moved stuff from data somewhere. 'data' should be empty now, but it is not!";

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::completeAccess_SI0_16bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  auto const & refToData = spUUT->GetData();
  EXPECT_TRUE(refToData == someData);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_OK_CA_16bit_SI1)
{
  auto data = someData;
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                                         0x1002U, 1U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize));

  EXPECT_TRUE(data.empty()) <<
    "CTOR has moved stuff from data somewhere. 'data' should be empty now, but it is not!";

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::completeAccess_SI0_16bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 1U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  auto const & refToData = spUUT->GetData();
  EXPECT_TRUE(refToData == someData);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_SubindexAndAccessTypeIncompatible)
{
  std::vector<uint8_t> data;

  data = someData;
  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                                      0x1000U, 2U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with CA and subindex > 1 should be impossible.";
  ASSERT_FALSE(data.empty()) << "CTOR failed, but it cleared 'data', though it provides the strong guarantee!";

  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                                      0x1000U, 2U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with CA and subindex > 1 should be impossible.";
  ASSERT_FALSE(data.empty()) << "CTOR failed, but it cleared 'data', though it provides the strong guarantee!";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_invalid_permissions)
{
  std::vector<uint8_t> data;

  data = someData;
  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1000U, 12U, 0U,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with no permissions should not be possible";
  EXPECT_FALSE(data.empty()) << "CTOR failed, but it cleared 'data', though it provides the strong guarantee!";

  data = someData;
  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1000U, 12U, Object::attr_ACCESS_RD_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with read-permission should be impossible";
  EXPECT_FALSE(data.empty()) << "CTOR failed, but it cleared 'data', though it provides the strong guarantee!";

  data = someData;
  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1000U, 12U, Object::attr_ACCESS_RD_PREOP |
                                                                    Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with read-permission should be impossible";
  EXPECT_FALSE(data.empty()) << "CTOR failed, but it cleared 'data', though it provides the strong guarantee!";

  data = someData;
  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1000U, 12U, Object::attr_BACKUP |
                                                                    Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with attribute-bits other than '...ACCESS...' should be impossible";
  EXPECT_FALSE(data.empty()) << "CTOR failed, but it cleared 'data', though it provides the strong guarantee!";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_noData)
{
  std::vector<uint8_t> data;
  EXPECT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with zero data should not be possible";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_notEnoughDataForCA16bit)
{
  std::vector<uint8_t> data(1U);
  EXPECT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                                      0x1000U, 0U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with one byte of data and CA (16 bit) incl. SI0 should be impossible.";

  EXPECT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                                         0x1000U, 1U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize)) <<
    "Creation of a WriteRequest with one byte of data and CA (16 bit) excl. SI0 should be possible.";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_maxData)
{
  std::vector<uint8_t> data(65535UL);

  EXPECT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                         0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(data), stdMaxResponseSize)) <<
    "Creation of WriteRequest with 65535 bytes of data (this is the maximum) should be possible";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CTOR_tooManyData)
{
  std::vector<uint8_t> data(65536UL); // max + 1

  EXPECT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(data), stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a WriteRequest with data larger than 2^16-1 bytes should not be possible";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Copy_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  auto data = someData;
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(data), stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<WriteRequest>(*spUUT);

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  auto const & refToData = spUUT->GetData();
  EXPECT_TRUE(refToData == someData);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);

  // check copy-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetAccessType(), WriteRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT2->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT2->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  auto const & refToData2 = spUUT2->GetData();
  EXPECT_TRUE(refToData2 == someData);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Move_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  auto data = someData;
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(data), stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<WriteRequest>(std::move(*spUUT));

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetAccessType(), WriteRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_TRUE(rs.empty());

  // check move-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetAccessType(), WriteRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT2->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT2->GetPermissions(), Object::attr_ACCESS_WR_PREOP);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  auto const & refToData2 = spUUT2->GetData();
  EXPECT_TRUE(refToData2 == someData);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, CalcMaxDataPayload)
{
  static size_t const base = 15U;

  // test corner case at minumum size
  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(base - 1U, false), 0U);
  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(base,      false), 0U);
  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(base + 1U, false), 1U);

  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(base - 1U + 8U, true), 0U);
  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(base      + 8U, true), 0U);
  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(base + 1U + 8U, true), 1U);

  // test maximum
  EXPECT_EQ(gpcc::cood::WriteRequest::CalcMaxDataPayload(RequestBase::maxRequestSize, false), std::numeric_limits<uint16_t>::max());
}

TEST_F(gpcc_cood_WriteRequest_TestsF, GetBinarySize)
{
  // Create a write request containing 8 bytes of data payload. The binary size of this request shall still fit into
  // the minimum useful request size. A payload of 8 bytes is choosen because it allows to write to all primitive
  // CANopen data types.
  std::vector<uint8_t> data(8U);
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(data), stdMaxResponseSize);

  // Check binary size. It shall not exceed the minimum useful request size.
  size_t const binSize = spUUT->GetBinarySize();
  EXPECT_LE(binSize, RequestBase::minimumUsefulRequestSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT->Push(rsi1);
  EXPECT_EQ(spUUT->GetBinarySize(), binSize + (1U * gpcc::cood::ReturnStackItem::binarySize));
  EXPECT_EQ(spUUT->GetReturnStackSize(), 1U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT->Push(rsi2);
  EXPECT_EQ(spUUT->GetBinarySize(), binSize + (2U * gpcc::cood::ReturnStackItem::binarySize));
  EXPECT_EQ(spUUT->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, SerializeAndDeserialize)
{
  // create a write request
  auto data = someData;
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(data), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = RequestBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check type and cast to WriteRequest
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::writeRequest);
  WriteRequest const * const pUUT2 = &(dynamic_cast<WriteRequest&>(*spUUT2Base));

  // check deserialized object
  EXPECT_EQ(pUUT2->GetAccessType(), WriteRequest::AccessType::singleSubindex);
  EXPECT_EQ(pUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(pUUT2->GetSubIndex(), 12U);
  EXPECT_EQ(pUUT2->GetPermissions(), Object::attr_ACCESS_WR);
  EXPECT_EQ(pUUT2->GetMaxResponseSize(), stdMaxResponseSize);

  auto const & refToData = pUUT2->GetData();
  EXPECT_TRUE(refToData == someData);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_InvalidAccessType)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                               0x1002U, 0U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set accessType to an invalid value
  storage[accessTypeOffset] = 99U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_SubindexAndAccessTypeIncompatible_A)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                               0x1002U, 0U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set subindex to 2 (illegal in conjunction with complete access)
  storage[subIndexOffset] = 2U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_SubindexAndAccessTypeIncompatible_B)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                               0x1002U, 0U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set subindex to 2 (illegal in conjunction with complete access)
  storage[subIndexOffset] = 2U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_InvalidPermission)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set permission to zero (illegal value)
  storage[permissionOffset + 0U] = 0U;
  storage[permissionOffset + 1U] = 0U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_InvalidNumberOfDataBytes_SingleSiAccess)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set number of data bytes to zero (illegal value)
  storage[dataSizeOffset + 0U] = 0U;
  storage[dataSizeOffset + 1U] = 0U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_InvalidNumberOfDataBytes_CA16Bit)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                               0x1002U, 0U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set number of data bytes to one (illegal in conjunction with CA 16bit)
  storage[dataSizeOffset + 0U] = 1U;
  storage[dataSizeOffset + 1U] = 0U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequest_TestsF, Deserialize_ValidNumberOfDataBytes_CA16Bit)
{
  // create a write request
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                               0x1002U, 1U, Object::attr_ACCESS_WR,
                                               std::move(someData), stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT1->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT1->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT1.reset();

  // manipulate binary: Set number of data bytes to one (valid in conjunction with CA 16bit if SI is one)
  storage[dataSizeOffset + 0U] = 1U;
  storage[dataSizeOffset + 1U] = 0U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_NO_THROW((void)RequestBase::FromBinary(msr));
}

TEST_F(gpcc_cood_WriteRequest_TestsF, ToString_SingleSubindex)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData), stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Write Request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*(single subindex)*", false)) << "Information about access type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000:12*", true)) << "Object's index and subindex is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*2 byte(s)*", true)) << "Information about amount of data is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x56 0x89", true)) << "Data is missing";

  auto const permission_str = "*" + gpcc::string::ToHex(static_cast<uint16_t>(Object::attr_ACCESS_WR_PREOP), 4U) + "*";
  EXPECT_TRUE(TestSimplePatternMatch(s, permission_str.c_str(), true)) << "Information about permissions is missing";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, ToString_CompleteAccess_8Bit)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                         0x1000U, 1U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData), stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Write Request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*(CA, SI0 8bit)*", false)) << "Information about access type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000:1*", true)) << "Object's index and subindex is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*2 byte(s)*", true)) << "Information about amount of data is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x56 0x89", true)) << "Data is missing";

  auto const permission_str = "*" + gpcc::string::ToHex(static_cast<uint16_t>(Object::attr_ACCESS_WR_PREOP), 4U) + "*";
  EXPECT_TRUE(TestSimplePatternMatch(s, permission_str.c_str(), true)) << "Information about permissions is missing";
}

TEST_F(gpcc_cood_WriteRequest_TestsF, ToString_CompleteAccess_16Bit)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                         0x1000U, 0U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData), stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Write Request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*(CA, SI0 16bit)*", false)) << "Information about access type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000:0*", true)) << "Object's index and subindex is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*2 byte(s)*", true)) << "Information about amount of data is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x56 0x89", true)) << "Data is missing";

  auto const permission_str = "*" + gpcc::string::ToHex(static_cast<uint16_t>(Object::attr_ACCESS_WR_PREOP), 4U) + "*";
  EXPECT_TRUE(TestSimplePatternMatch(s, permission_str.c_str(), true)) << "Information about permissions is missing";
}

} // namespace gpcc_tests
} // namespace cood
