/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/PingRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::container::IntrusiveDList;
using gpcc::cood::PingRequest;
using gpcc::cood::RequestBase;
using gpcc::cood::ReturnStackItem;

// Test fixture for testing class PingRequest.
// Services offered by base class RequestBase are tested in TestRequestBase.cpp.
class gpcc_cood_PingRequest_TestsF: public Test
{
  public:
    gpcc_cood_PingRequest_TestsF(void);

  protected:
    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 1024U;

    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::unique_ptr<PingRequest> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_PingRequest_TestsF::stdMaxResponseSize;

gpcc_cood_PingRequest_TestsF::gpcc_cood_PingRequest_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, spUUT()
{
}

void gpcc_cood_PingRequest_TestsF::SetUp(void)
{
}

void gpcc_cood_PingRequest_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_PingRequest_DeathTestsF = gpcc_cood_PingRequest_TestsF;

TEST_F(gpcc_cood_PingRequest_TestsF, CTOR)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<PingRequest>(stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::pingRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);
}

TEST_F(gpcc_cood_PingRequest_TestsF, Copy_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<PingRequest>(stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<PingRequest>(*spUUT);

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);

  // check copy-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetType(), RequestBase::RequestTypes::pingRequest);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_PingRequest_TestsF, Move_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<PingRequest>(stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<PingRequest>(std::move(*spUUT));

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::pingRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_TRUE(rs.empty());

  // check move-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetType(), RequestBase::RequestTypes::pingRequest);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_PingRequest_TestsF, GetBinarySize)
{
  spUUT = std::make_unique<PingRequest>(stdMaxResponseSize);

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

TEST_F(gpcc_cood_PingRequest_TestsF, SerializeAndDeserialize)
{
  auto spUUT1 = std::make_unique<PingRequest>(stdMaxResponseSize);

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

  // check type and cast to PingRequest
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::pingRequest);
  PingRequest* const pUUT2 = &(dynamic_cast<PingRequest&>(*spUUT2Base));

  // check deserialized object
  EXPECT_EQ(pUUT2->GetMaxResponseSize(), stdMaxResponseSize);
}

TEST_F(gpcc_cood_PingRequest_TestsF, ToString)
{
  spUUT = std::make_unique<PingRequest>(stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_STREQ(s.c_str(), "Ping request");
}

} // namespace gpcc_tests
} // namespace cood
