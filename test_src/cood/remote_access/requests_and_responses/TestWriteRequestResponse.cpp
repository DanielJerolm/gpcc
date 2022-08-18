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
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

using gpcc::cood::SDOAbortCode;
using gpcc::cood::ResponseBase;
using gpcc::cood::WriteRequestResponse;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for testing class WriteRequestResponse.
// Services offered by base class ResponseBase are tested in TestResponseBase.cpp.
class gpcc_cood_WriteRequestResponse_TestsF: public Test
{
  public:
    gpcc_cood_WriteRequestResponse_TestsF(void);

  protected:
    // offset of "result" in binary
    static size_t const resultOffset = 3U;

    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<ReturnStackItem> emptyReturnStack;
    std::vector<ReturnStackItem> const twoItemReturnStack;

    std::unique_ptr<WriteRequestResponse> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_WriteRequestResponse_TestsF::resultOffset;

gpcc_cood_WriteRequestResponse_TestsF::gpcc_cood_WriteRequestResponse_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, emptyReturnStack()
, twoItemReturnStack{ rsi1, rsi2 }
, spUUT()
{
}

void gpcc_cood_WriteRequestResponse_TestsF::SetUp(void)
{
}

void gpcc_cood_WriteRequestResponse_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_WriteRequestResponse_DeathTestsF = gpcc_cood_WriteRequestResponse_TestsF;

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, CTOR)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK));
  EXPECT_EQ(spUUT->GetType(), ResponseBase::ResponseTypes::writeRequestResponse);
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError));
  EXPECT_EQ(spUUT->GetType(), ResponseBase::ResponseTypes::writeRequestResponse);
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, Copy_CTOR)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<WriteRequestResponse>(*spUUT);

  // check that spUUT is OK
  // ======================================================
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check copy-constructed object
  // ======================================================
  EXPECT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, Move_CTOR)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<WriteRequestResponse>(std::move(*spUUT));

  // check that spUUT is OK
  // ======================================================
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check move-constructed object
  // ======================================================
  EXPECT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  auto rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, GetBinarySize)
{
  // (1) empty return stack
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  size_t const binSize = spUUT->GetBinarySize();
  EXPECT_NE(binSize, 0U);
  EXPECT_LT(binSize, ResponseBase::minimumUsefulResponseSize);

  // (2) two items on return stack
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  EXPECT_EQ(spUUT->GetBinarySize(), binSize + (2U * ReturnStackItem::binarySize));
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, SerializeAndDeserialize)
{
  // create a write request response
  auto spUUT1 = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);

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
  auto spUUT2Base = ResponseBase::FromBinary(msr);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Stream was not completely consumed";
  msr.Close();

  // check type and cast to WriteRequestResponse
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::writeRequestResponse);
  WriteRequestResponse const * const pUUT2 = &(dynamic_cast<WriteRequestResponse&>(*spUUT2Base));

  // check deserialized object
  EXPECT_TRUE(pUUT2->GetResult() == SDOAbortCode::GeneralError);
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, Deserialize_InvalidSDOAbortCode)
{
  // create a write request response
  auto spUUT1 = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);

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

  // manipulate binary: Set the SDO abort code to an invalid value
  storage[resultOffset + 0U] = 0xFFU;
  storage[resultOffset + 1U] = 0xFFU;
  storage[resultOffset + 2U] = 0xFFU;
  storage[resultOffset + 3U] = 0xFFU;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, ToString)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Write request response*", false)) << "Information about response type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*OK*", true)) << "Result is missing";
}

TEST_F(gpcc_cood_WriteRequestResponse_TestsF, SetResult)
{
  spUUT = std::make_unique<WriteRequestResponse>(SDOAbortCode::OK);
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);

  spUUT->SetResult(SDOAbortCode::GeneralError);
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
}

} // namespace gpcc_tests
} // namespace cood
