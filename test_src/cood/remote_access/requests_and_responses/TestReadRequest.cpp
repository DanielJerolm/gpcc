/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ReadRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include <gpcc/string/tools.hpp>
#include "gtest/gtest.h"
#include <limits>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::Object;
using gpcc::cood::ResponseBase;
using gpcc::cood::RequestBase;
using gpcc::cood::ReadRequest;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for testing class ReadRequest.
// Services offered by base class RequestBase are tested in TestRequestBase.cpp.
class gpcc_cood_ReadRequest_TestsF: public Test
{
  public:
    gpcc_cood_ReadRequest_TestsF(void);

  protected:
    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 1024U;

    // offset of "accessType" in binary
    static size_t const accessTypeOffset = 7U;

    // offset of "subindex" in binary
    static size_t const subindexOffset = 10U;

    // offset of "permission" in binary
    static size_t const permissionOffset = 11U;

    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::unique_ptr<ReadRequest> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_ReadRequest_TestsF::stdMaxResponseSize;
size_t const gpcc_cood_ReadRequest_TestsF::accessTypeOffset;
size_t const gpcc_cood_ReadRequest_TestsF::subindexOffset;
size_t const gpcc_cood_ReadRequest_TestsF::permissionOffset;

gpcc_cood_ReadRequest_TestsF::gpcc_cood_ReadRequest_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, spUUT()
{
}

void gpcc_cood_ReadRequest_TestsF::SetUp(void)
{
}

void gpcc_cood_ReadRequest_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_ReadRequest_DeathTestsF = gpcc_cood_ReadRequest_TestsF;

TEST_F(gpcc_cood_ReadRequest_TestsF, CTOR_OK)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                                        0x1002U, 12U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::readRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                                        0x1002U, 0U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::readRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::completeAccess_SI0_8bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                                        0x1002U, 1U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::readRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::completeAccess_SI0_8bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 1U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                                        0x1002U, 0U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::readRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::completeAccess_SI0_16bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                                        0x1002U, 1U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::readRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::completeAccess_SI0_16bit);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 1U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, CTOR_SubindexAndAccessTypeIncompatible)
{
  EXPECT_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                                     0x1000U, 2U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a ReadRequest with access type 'complete access' and subindex > 1 should be impossible";

  EXPECT_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                                     0x1000U, 2U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a ReadRequest with access type 'complete access' and subindex > 1 should be impossible";
}

TEST_F(gpcc_cood_ReadRequest_TestsF, CTOR_invalid_permissions)
{
  EXPECT_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                                     0x1000U, 12U, 0U, stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a ReadRequest with no permissions should not be possible";

  EXPECT_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                                     0x1000U, 12U, Object::attr_ACCESS_WR_PREOP, stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a ReadRequest with write-permission should be impossible";

  EXPECT_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                                     0x1000U, 12U, Object::attr_ACCESS_RD_PREOP |
                                                                   Object::attr_ACCESS_WR_PREOP,
                                                                   stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a ReadRequest with write-permission should be impossible";

  EXPECT_THROW(spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                                     0x1000U, 12U, Object::attr_BACKUP |
                                                                   Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize), std::invalid_argument) <<
    "Creation of a ReadRequest with attribute-bits other than '...ACCESS...' should be impossible";
}

TEST_F(gpcc_cood_ReadRequest_TestsF, Copy_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                        0x1002U, 12U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<ReadRequest>(*spUUT);

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);

  // check copy-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetAccessType(), ReadRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT2->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT2->GetPermissions(), Object::attr_ACCESS_RD_PREOP);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, Move_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                        0x1002U, 12U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<ReadRequest>(std::move(*spUUT));

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetAccessType(), ReadRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT->GetPermissions(), Object::attr_ACCESS_RD_PREOP);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_TRUE(rs.empty());

  // check move-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetAccessType(), ReadRequest::AccessType::singleSubindex);
  EXPECT_EQ(spUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT2->GetSubIndex(), 12U);
  EXPECT_EQ(spUUT2->GetPermissions(), Object::attr_ACCESS_RD_PREOP);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, CalcMaxDataPayloadInResponse)
{
  static size_t const base = 10U;

  // test corner case at minumum size
  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(base - 1U, false), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(base,      false), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(base + 1U, false), 1U);

  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(base - 1U + 8U, true), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(base      + 8U, true), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(base + 1U + 8U, true), 1U);

  // test maximum
  EXPECT_EQ(gpcc::cood::ReadRequest::CalcMaxDataPayloadInResponse(ResponseBase::maxResponseSize, false), std::numeric_limits<uint16_t>::max());
}

TEST_F(gpcc_cood_ReadRequest_TestsF, GetBinarySize)
{
  spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                        0x1000U, 12U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize);

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

TEST_F(gpcc_cood_ReadRequest_TestsF, SerializeAndDeserialize)
{
  // create a read request
  auto spUUT1 = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                              0x1002U, 12U, Object::attr_ACCESS_RD, stdMaxResponseSize);

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

  // check type and cast to ReadRequest
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::readRequest);
  ReadRequest* const pUUT2 = &(dynamic_cast<ReadRequest&>(*spUUT2Base));

  // check deserialized object
  EXPECT_EQ(pUUT2->GetAccessType(), ReadRequest::AccessType::singleSubindex);
  EXPECT_EQ(pUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(pUUT2->GetSubIndex(), 12U);
  EXPECT_EQ(pUUT2->GetPermissions(), Object::attr_ACCESS_RD);
  EXPECT_EQ(pUUT2->GetMaxResponseSize(), stdMaxResponseSize);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, Deserialize_InvalidAccessType)
{
  // create a read request
  auto spUUT1 = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                              0x1002U, 0U, Object::attr_ACCESS_RD, stdMaxResponseSize);

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

  // manipulate binary: Set access type to invalid value
  storage[accessTypeOffset] = 99U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, Deserialize_SubindexAndAccessTypeIncompatible_A)
{
  // create a read request
  auto spUUT1 = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                              0x1002U, 0U, Object::attr_ACCESS_RD, stdMaxResponseSize);

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
  storage[subindexOffset] = 2U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, Deserialize_SubindexAndAccessTypeIncompatible_B)
{
  // create a read request
  auto spUUT1 = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                              0x1002U, 0U, Object::attr_ACCESS_RD, stdMaxResponseSize);

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
  storage[subindexOffset] = 2U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ReadRequest_TestsF, Deserialize_InvalidPermission)
{
  // create a read request
  auto spUUT1 = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                              0x1002U, 12U, Object::attr_ACCESS_RD, stdMaxResponseSize);

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

TEST_F(gpcc_cood_ReadRequest_TestsF, ToString_SingleSubindex)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                        0x1000U, 12U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Read request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*(single subindex)*", false)) << "Information about access type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000:12*", true)) << "Object's index and subindex is missing";

  auto const permission_str = "*" + gpcc::string::ToHex(static_cast<uint16_t>(Object::attr_ACCESS_RD_PREOP), 4U) + "*";
  EXPECT_TRUE(TestSimplePatternMatch(s, permission_str.c_str(), true)) << "Information about permissions is missing";
}

TEST_F(gpcc_cood_ReadRequest_TestsF, ToString_CompleteAccess_8Bit)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                        0x1000U, 0U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Read request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*(CA, SI0 8bit)*", false)) << "Information about access type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000:0*", true)) << "Object's index and subindex is missing";

  auto const permission_str = "*" + gpcc::string::ToHex(static_cast<uint16_t>(Object::attr_ACCESS_RD_PREOP), 4U) + "*";
  EXPECT_TRUE(TestSimplePatternMatch(s, permission_str.c_str(), true)) << "Information about permissions is missing";
}

TEST_F(gpcc_cood_ReadRequest_TestsF, ToString_CompleteAccess_16Bit)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                        0x1000U, 0U, Object::attr_ACCESS_RD_PREOP, stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Read request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*(CA, SI0 16bit)*", false)) << "Information about access type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000:0*", true)) << "Object's index and subindex is missing";

  auto const permission_str = "*" + gpcc::string::ToHex(static_cast<uint16_t>(Object::attr_ACCESS_RD_PREOP), 4U) + "*";
  EXPECT_TRUE(TestSimplePatternMatch(s, permission_str.c_str(), true)) << "Information about permissions is missing";
}

} // namespace gpcc_tests
} // namespace cood
