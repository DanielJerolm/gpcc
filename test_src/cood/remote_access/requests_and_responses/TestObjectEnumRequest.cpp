/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include <gpcc/string/tools.hpp>
#include "gtest/gtest.h"
#include <limits>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::ObjectEnumRequest;
using gpcc::cood::RequestBase;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;


// Test fixture for testing class ObjectEnumRequest.
// Services offered by base class RequestBase are tested in TestRequestBase.cpp.
class gpcc_cood_ObjectEnumRequest_TestsF: public Test
{
  public:
    gpcc_cood_ObjectEnumRequest_TestsF(void);

  protected:
    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 1024U;

    // Offset of "attrFilter" in binary.
    static size_t const offsetOfAttrFilter = 11U;

    // Offset of "startIndex" in binary.
    static size_t const offsetOfStartIndex = 7U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::unique_ptr<ObjectEnumRequest> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_ObjectEnumRequest_TestsF::stdMaxResponseSize;
size_t const gpcc_cood_ObjectEnumRequest_TestsF::offsetOfAttrFilter;
size_t const gpcc_cood_ObjectEnumRequest_TestsF::offsetOfStartIndex;

gpcc_cood_ObjectEnumRequest_TestsF::gpcc_cood_ObjectEnumRequest_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, spUUT()
{
}

void gpcc_cood_ObjectEnumRequest_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectEnumRequest_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_ObjectEnumRequest_DeathTestsF = gpcc_cood_ObjectEnumRequest_TestsF;

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, CTOR_OK)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                                              0xFFFFU,
                                                              gpcc::cood::Object::attr_ACCESS_RW,
                                                              stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetStartIndex(), 0x0000U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0xFFFFU);
  EXPECT_EQ(spUUT->GetAttributeFilter(), gpcc::cood::Object::attr_ACCESS_RW);
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, CTOR_not_OK)
{
  // no access rights
  ASSERT_THROW(spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                                           0xFFFFU,
                                                           0,
                                                           stdMaxResponseSize), std::invalid_argument);

  // last < start
  ASSERT_THROW(spUUT = std::make_unique<ObjectEnumRequest>(0x0001U,
                                                           0x0000U,
                                                           gpcc::cood::Object::attr_ACCESS_RW,
                                                           stdMaxResponseSize), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, Copy_CTOR)
{
  spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                              0xFFFFU,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

  auto spUUT2 = std::make_unique<ObjectEnumRequest>(*spUUT);

  // Check UUT
  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetStartIndex(), 0x0000U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0xFFFFU);
  EXPECT_EQ(spUUT->GetAttributeFilter(), gpcc::cood::Object::attr_ACCESS_RW);

  // Check copy of UUT
  EXPECT_EQ(spUUT2->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT2->GetStartIndex(), 0x0000U);
  EXPECT_EQ(spUUT2->GetLastIndex(), 0xFFFFU);
  EXPECT_EQ(spUUT2->GetAttributeFilter(), gpcc::cood::Object::attr_ACCESS_RW);
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, Move_CTOR)
{
  spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                              0xFFFFU,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

  auto spUUT2 = std::make_unique<ObjectEnumRequest>(std::move(*spUUT));

  // Check UUT
  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetStartIndex(), 0x0000U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0xFFFFU);
  EXPECT_EQ(spUUT->GetAttributeFilter(), gpcc::cood::Object::attr_ACCESS_RW);

  // Check copy of UUT
  EXPECT_EQ(spUUT2->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT2->GetStartIndex(), 0x0000U);
  EXPECT_EQ(spUUT2->GetLastIndex(), 0xFFFFU);
  EXPECT_EQ(spUUT2->GetAttributeFilter(), gpcc::cood::Object::attr_ACCESS_RW);
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, GetBinarySize)
{
  spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                              0xFFFFU,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

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

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, SerializeAndDeserialize)
{
  // create a enum request
  spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                              0xFFFFU,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT.reset();

  // deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = RequestBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check type and cast to WriteRequest
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  ObjectEnumRequest const * const pUUT2 = &(dynamic_cast<ObjectEnumRequest&>(*spUUT2Base));

  // check deserialized object
  EXPECT_EQ(pUUT2->GetType(), RequestBase::RequestTypes::objectEnumRequest);
  EXPECT_EQ(pUUT2->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(pUUT2->GetStartIndex(), 0x0000U);
  EXPECT_EQ(pUUT2->GetLastIndex(), 0xFFFFU);
  EXPECT_EQ(pUUT2->GetAttributeFilter(), gpcc::cood::Object::attr_ACCESS_RW);
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, SerializeAndDeserialize_InvalidAttrFilter)
{
  // create a enum request
  spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                              0xFFFFU,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT.reset();

  // manipulate binary: set attrFilter to zero (illegal value)
  storage[offsetOfAttrFilter + 0U] = 0U;
  storage[offsetOfAttrFilter + 1U] = 0U;

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, SerializeAndDeserialize_InvalidIndices)
{
  // create a enum request
  spUUT = std::make_unique<ObjectEnumRequest>(0x0001U,
                                              0x0002U,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

  // serialize it
  size_t const reqSize = spUUT->GetBinarySize();
  ASSERT_TRUE(reqSize != 0U);
  ASSERT_TRUE(reqSize < 64U);

  uint8_t storage[64U];

  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);
  spUUT->ToBinary(msw);
  msw.AlignToByteBoundary(false);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - reqSize) << "Unexpected number of bytes written";
  msw.Close();

  spUUT.reset();

  // manipulate binary: Set start imndex to 0x0003. End index is 0x0002, so we produce an invalid serialized object.
  storage[offsetOfStartIndex + 0U] = 0U;
  storage[offsetOfStartIndex + 1U] = 3U;

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumRequest_TestsF, ToString)
{
  using gpcc::string::TestSimplePatternMatch;

  // create a enum request
  spUUT = std::make_unique<ObjectEnumRequest>(0x0000U,
                                              0xFFFFU,
                                              gpcc::cood::Object::attr_ACCESS_RW,
                                              stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*enum request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x0000*", false)) << "Information about start index is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0xFFFF*", false)) << "Information about last index is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x043F*", false)) << "Information about attributes is missing";
}

} // namespace gpcc_tests
} // namespace cood
