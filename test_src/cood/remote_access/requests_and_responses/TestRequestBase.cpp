/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequest.hpp>
#include <gpcc/stream/MemStreamReader.hpp>
#include <gpcc/stream/MemStreamWriter.hpp>
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

// Test fixture for class RequestBase.
// Since class RequestBase is pure virtual, all tests are conducted using derived class WriteRequest.
// The following functionality of base class RequestBase is tested:
// - stack of ReturnStackItems
// - "maximum response size" attribute
// - attempt to deserialize invalid binary
//
// Anything else (e.g. serialization/deserialization, copy/move CTOR & assignment operator etc.) is tested by the unit
// tests of the derived classes.
class gpcc_cood_RequestBase_TestsF: public Test
{
  public:
    gpcc_cood_RequestBase_TestsF(void);

  protected:
    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 1024U;

    // offset of "version" in binary
    static size_t const versionOffset = 0U;

    // offset of "type" in binary
    static size_t const typeOffset = 1U;

    // offset of "maxResponseSize" in binary
    static size_t const maxResponseSizeOffset = 2U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<uint8_t> someData;

    std::unique_ptr<WriteRequest> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_RequestBase_TestsF::stdMaxResponseSize;
size_t const gpcc_cood_RequestBase_TestsF::versionOffset;
size_t const gpcc_cood_RequestBase_TestsF::typeOffset;
size_t const gpcc_cood_RequestBase_TestsF::maxResponseSizeOffset;

gpcc_cood_RequestBase_TestsF::gpcc_cood_RequestBase_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, someData{ 0x56U, 0x89U }
, spUUT()
{
}

void gpcc_cood_RequestBase_TestsF::SetUp(void)
{
}

void gpcc_cood_RequestBase_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_RequestBase_DeathTestsF = gpcc_cood_RequestBase_TestsF;

TEST_F(gpcc_cood_RequestBase_TestsF, Param_CTOR_OK)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(someData),
                                                         stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);
  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U);
}

TEST_F(gpcc_cood_RequestBase_TestsF, Param_CTOR_OK_MinRespSize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(someData),
                                                         ResponseBase::minimumUsefulResponseSize));

  EXPECT_EQ(spUUT->GetMaxResponseSize(), ResponseBase::minimumUsefulResponseSize);
}

TEST_F(gpcc_cood_RequestBase_TestsF, Param_CTOR_OK_MaxRespSize)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                         std::move(someData),
                                                         ResponseBase::maxResponseSize));

  EXPECT_EQ(spUUT->GetMaxResponseSize(), std::numeric_limits<uint32_t>::max());
}

TEST_F(gpcc_cood_RequestBase_TestsF, Param_CTOR_RespSizeTooSmall)
{
  static_assert(ResponseBase::minimumUsefulResponseSize > 0U, "Cannot subtract one from minimum");

  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(someData),
                                                      ResponseBase::minimumUsefulResponseSize - 1U),
                                                      std::invalid_argument);
}

TEST_F(gpcc_cood_RequestBase_TestsF, Param_CTOR_RespSizeTooLarge)
{
  // This test is only reasonable, if we can add 1U to ResponseBase::maxResponseSize without arithmetic overflow
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if (ResponseBase::maxResponseSize > (std::numeric_limits<size_t>::max() - 1U))
    return;
  #pragma GCC diagnostic pop

  ASSERT_THROW(spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                                      0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                                      std::move(someData),
                                                      ResponseBase::maxResponseSize + 1U),
                                                      std::invalid_argument);
}

TEST_F(gpcc_cood_RequestBase_TestsF, SerializeAndDeserialize_OK_WithoutRSI)
{
  size_t const expectedMaxResponseSize = stdMaxResponseSize;

  // create a write request...
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData),
                                               stdMaxResponseSize);

  // ...and serialize it
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

  // deserialize the write request
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = RequestBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check deserialized object
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT2Base->GetMaxResponseSize(), expectedMaxResponseSize);
  EXPECT_EQ(spUUT2Base->GetReturnStackSize(), 0U);

  std::vector<ReturnStackItem> rs;
  spUUT2Base->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 0U);
}

TEST_F(gpcc_cood_RequestBase_TestsF, SerializeAndDeserialize_OK_WithRSI)
{
  size_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  // create a write request...
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData),
                                               stdMaxResponseSize);
  spUUT1->Push(rsi1);
  spUUT1->Push(rsi2);

  // ...and serialize it
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

  // deserialize the write request
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = RequestBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check deserialized object
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::writeRequest);
  EXPECT_EQ(spUUT2Base->GetMaxResponseSize(), expectedMaxResponseSize);
  EXPECT_EQ(spUUT2Base->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);

  std::vector<ReturnStackItem> rs;
  spUUT2Base->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_RequestBase_TestsF, FromBinary_InvalidVersion)
{
  // create a write request...
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData),
                                               stdMaxResponseSize);

  // ...and serialize it
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

  // manipulate binary: Set version to 0xFF
  storage[versionOffset] = 0xFFU;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_RequestBase_TestsF, FromBinary_InvalidType)
{
  // create a write request...
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData),
                                               stdMaxResponseSize);

  // ...and serialize it
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

  // manipulate binary: Set type to 0xFF
  storage[typeOffset] = 0xFFU;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_RequestBase_TestsF, Deserialize_CTOR_MaxResponseSizeTooSmall)
{
  // create a write request...
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData),
                                               stdMaxResponseSize);

  // ...and serialize it
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

  // manipulate binary: Set maximum response size to minimum value minus 1
  static_assert(ResponseBase::minimumUsefulResponseSize > 0U, "Cannot subtract one from minimum");
  uint32_t const v = static_cast<uint32_t>(RequestBase::minimumUsefulRequestSize) - 1U;
  storage[maxResponseSizeOffset + 0U] = (v >> 0U) & 0xFFU;
  storage[maxResponseSizeOffset + 1U] = (v >> 8U) & 0xFFU;
  storage[maxResponseSizeOffset + 2U] = (v >> 16U) & 0xFFU;
  storage[maxResponseSizeOffset + 3U] = (v >> 24U) & 0xFFU;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_RequestBase_TestsF, Deserialize_CTOR_MaxResponseSizeTooLarge)
{
  // This test is only reasonable, if the maximum response size is less than 2^32 - 1, because this is the maximum that
  // could be stored in the binary.
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if (RequestBase::maxRequestSize >= std::numeric_limits<uint32_t>::max())
    return;
  #pragma GCC diagnostic pop

  // create a write request...
  auto spUUT1 = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                               0x1002U, 12U, Object::attr_ACCESS_WR,
                                               std::move(someData),
                                               stdMaxResponseSize);

  // ...and serialize it
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

  // manipulate binary: Set maximum response size to maximum value plus 1
  uint32_t const v = static_cast<uint32_t>(RequestBase::maxRequestSize) + 1U;
  storage[maxResponseSizeOffset + 0U] = (v >> 0U) & 0xFFU;
  storage[maxResponseSizeOffset + 1U] = (v >> 8U) & 0xFFU;
  storage[maxResponseSizeOffset + 2U] = (v >> 16U) & 0xFFU;
  storage[maxResponseSizeOffset + 3U] = (v >> 24U) & 0xFFU;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_RequestBase_TestsF, DList)
{
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData),
                                         stdMaxResponseSize);

  IntrusiveDList<RequestBase> list;

  list.push_back(spUUT.get());
  list.clear();
}

TEST_F(gpcc_cood_RequestBase_TestsF, DTOR_withNonEmptyStack)
{
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData),
                                         stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  spUUT.reset();
}

TEST_F(gpcc_cood_RequestBase_DeathTestsF, DISABLED_DTOR_ObjectStillInDList)
{
  // Disabled, because IntrusiveDList currently does not reliabe allow an item to figure out itself if it is in a
  // IntrusiveDList or not. Reason: If there is only one item in the DList, then the prev/next-pointers of the item
  // are nullptr.

  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1000U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData),
                                         stdMaxResponseSize);

  IntrusiveDList<RequestBase> list;
  list.push_back(spUUT.get());

  EXPECT_DEATH(spUUT.reset(), ".*Object in IntrusiveDList.*");

  list.clear();
}

TEST_F(gpcc_cood_RequestBase_TestsF, PushAndExtractReturnStack)
{
  // create a write request with two ReturnStackItem objects on stack
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1002U, 12U, Object::attr_ACCESS_WR,
                                         std::move(someData),
                                         stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  // check size of serialized stack content
  EXPECT_EQ(spUUT->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);

  // extract and check stack content
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);

  // check size of serialized stack content, should be zero
  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U * gpcc::cood::ReturnStackItem::binarySize);

  // extract stack, should be empty
  spUUT->ExtractReturnStack(rs);
  EXPECT_TRUE(rs.empty());

  // check size of serialized stack content, should be zero
  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U * gpcc::cood::ReturnStackItem::binarySize);
}

TEST_F(gpcc_cood_RequestBase_TestsF, PushIncrementsMaxResponseSize)
{
  size_t const mrs = ResponseBase::maxResponseSize - (1U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData),
                                         mrs);
  ASSERT_EQ(spUUT->GetMaxResponseSize(), mrs);

  spUUT->Push(rsi1);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), mrs + gpcc::cood::ReturnStackItem::binarySize);
}

TEST_F(gpcc_cood_RequestBase_TestsF, PushDoesNotExceedMaxResponseSize)
{
  size_t const mrs = (ResponseBase::maxResponseSize - (1U * gpcc::cood::ReturnStackItem::binarySize)) + 1U;

  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData),
                                         mrs);
  ASSERT_EQ(spUUT->GetMaxResponseSize(), mrs);

  ASSERT_THROW(spUUT->Push(rsi1), std::logic_error);
  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), mrs);
}

TEST_F(gpcc_cood_RequestBase_TestsF, PushDoesNotExceed255Items)
{
  spUUT = std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                         0x1002U, 12U, Object::attr_ACCESS_WR_PREOP,
                                         std::move(someData),
                                         stdMaxResponseSize);

  // push 255 ReturnStackItem objects (info-attribute = 0..254)
  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    ASSERT_NO_THROW(spUUT->Push(ReturnStackItem(0, i)));
  }

  // attempt to push 256th ReturnStackItem object should fail
  ASSERT_THROW(spUUT->Push(ReturnStackItem(0U, 255U)), std::runtime_error);
  EXPECT_EQ(spUUT->GetReturnStackSize(), 255U * ReturnStackItem::binarySize);
}

} // namespace gpcc_tests
} // namespace cood
