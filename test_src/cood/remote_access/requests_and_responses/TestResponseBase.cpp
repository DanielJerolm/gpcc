/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include <gpcc/string/tools.hpp>
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

using gpcc::cood::SDOAbortCode;
using gpcc::cood::ResponseBase;
using gpcc::cood::WriteRequestResponse;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for class ResponseBase.
// Since class ResponseBase is pure virtual, all tests are conducted using derived class WriteRequestResponse.
// The following functionality of base class ResponseBase is tested:
// - stack of ReturnStackItems
// - attempt to deserialize invalid binary
//
// Anything else (e.g. serialization/deserialization, copy/move CTOR & assignment operator etc.) is tested by the unit
// tests of the derived classes.
class gpcc_cood_ResponseBase_TestsF: public Test
{
  public:
    gpcc_cood_ResponseBase_TestsF(void);

  protected:
    // offset of "version" in binary
    static size_t const versionOffset = 0U;

    // offset of "type" in binary
    static size_t const typeOffset = 1U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<ReturnStackItem> emptyReturnStack;
    std::vector<ReturnStackItem> const twoItemReturnStack;

    std::unique_ptr<WriteRequestResponse> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_ResponseBase_TestsF::versionOffset;
size_t const gpcc_cood_ResponseBase_TestsF::typeOffset;

gpcc_cood_ResponseBase_TestsF::gpcc_cood_ResponseBase_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, emptyReturnStack()
, twoItemReturnStack{ rsi1, rsi2 }
, spUUT()
{
}

void gpcc_cood_ResponseBase_TestsF::SetUp(void)
{
}

void gpcc_cood_ResponseBase_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_ResponseBase_DeathTestsF = gpcc_cood_ResponseBase_TestsF;

TEST_F(gpcc_cood_ResponseBase_TestsF, Param_CTOR_OK)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK));

  EXPECT_EQ(spUUT->GetType(), ResponseBase::ResponseTypes::writeRequestResponse);
  EXPECT_TRUE(spUUT->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, SerializeAndDeserialize_OK_WithoutRSI)
{
  // create a write request response...
  auto spUUT1 = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);

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

  // deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = ResponseBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check deserialized object
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::writeRequestResponse);
  EXPECT_TRUE(spUUT2Base->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, SerializeAndDeserialize_OK_WithRSI)
{
  // create a write request response...
  auto rs = twoItemReturnStack;
  auto spUUT1 = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);
  spUUT1->SetReturnStack(std::move(rs));

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

  // deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = ResponseBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check deserialized object
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::writeRequestResponse);
  ASSERT_FALSE(spUUT2Base->IsReturnStackEmpty());

  auto rsi = spUUT2Base->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2Base->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2Base->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, FromBinary_InvalidVersion)
{
  // create a write request response...
  auto spUUT1 = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);

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
  EXPECT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ResponseBase_TestsF, FromBinary_InvalidType)
{
  // create a write request response...
  auto spUUT1 = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);

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
  EXPECT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ResponseBase_TestsF, DList)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  IntrusiveDList<ResponseBase> list;

  list.push_back(spUUT.get());
  list.clear();
}

TEST_F(gpcc_cood_ResponseBase_TestsF, DTOR_withNonEmptyReturnStack)
{
  auto rs = twoItemReturnStack;
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);
  spUUT->SetReturnStack(std::move(rs));
  spUUT.reset();
}

TEST_F(gpcc_cood_ResponseBase_DeathTestsF, DISABLED_DTOR_ObjectStillInDList)
{
  // Disabled, because IntrusiveDList currently does not reliabe allow an item to figure out itself if it is in a
  // IntrusiveDList or not. Reason: If there is only one item in the DList, then the prev/next-pointers of the item
  // are nullptr.

  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  IntrusiveDList<ResponseBase> list;
  list.push_back(spUUT.get());

  EXPECT_DEATH(spUUT.reset(), ".*Object in IntrusiveDList.*");

  list.clear();
}

TEST_F(gpcc_cood_ResponseBase_TestsF, SetReturnStack_OK)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto rs = twoItemReturnStack;
  ASSERT_NO_THROW(spUUT->SetReturnStack(std::move(rs)));
  EXPECT_TRUE(rs.empty());

  EXPECT_FALSE(spUUT->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, SetReturnStack_OK_EmptyStack)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  ASSERT_NO_THROW(spUUT->SetReturnStack(std::move(emptyReturnStack)));
  EXPECT_TRUE(emptyReturnStack.empty());

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, SetReturnStack_StackNotEmpty)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  // precondition established, stack is not empty

  ASSERT_THROW(spUUT->SetReturnStack(std::move(emptyReturnStack)), std::logic_error);
  EXPECT_TRUE(emptyReturnStack.empty());

  // check postconditions

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, SetReturnStack_TooManyItems)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  std::vector<ReturnStackItem> rs(256U, ReturnStackItem(0U, 0U));
  ASSERT_THROW(spUUT->SetReturnStack(std::move(rs)), std::invalid_argument);
  EXPECT_EQ(rs.size(), 256U);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ResponseBase_TestsF, PopReturnStack_OK)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());
  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  ASSERT_TRUE(spUUT->IsReturnStackEmpty());
  EXPECT_THROW(rsi = spUUT->PopReturnStack(), std::runtime_error);
}

TEST_F(gpcc_cood_ResponseBase_TestsF, PopReturnStack_StackNeverFilled)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  ASSERT_TRUE(spUUT->IsReturnStackEmpty());
  EXPECT_THROW((void)spUUT->PopReturnStack(), std::runtime_error);
}

} // namespace gpcc_tests
} // namespace cood
