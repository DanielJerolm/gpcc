/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "gpcc/src/cood/remote_access/requests_and_responses/PingResponse.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

using gpcc::container::IntrusiveDList;
using gpcc::cood::PingResponse;
using gpcc::cood::ResponseBase;
using gpcc::cood::ReturnStackItem;

// Test fixture for testing class PingResponse.
// Services offered by base class ResponseBase are tested in TestResponseBase.cpp.
class gpcc_cood_PingResponse_TestsF: public Test
{
  public:
    gpcc_cood_PingResponse_TestsF(void);

  protected:
    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<ReturnStackItem> emptyReturnStack;
    std::vector<ReturnStackItem> const twoItemReturnStack;

    std::unique_ptr<PingResponse> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

gpcc_cood_PingResponse_TestsF::gpcc_cood_PingResponse_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, emptyReturnStack()
, twoItemReturnStack{ rsi1, rsi2 }
, spUUT()
{
}

void gpcc_cood_PingResponse_TestsF::SetUp(void)
{
}

void gpcc_cood_PingResponse_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_PingResponse_DeathTestsF = gpcc_cood_PingResponse_TestsF;

TEST_F(gpcc_cood_PingResponse_TestsF, CTOR)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<PingResponse>());
  EXPECT_EQ(spUUT->GetType(), ResponseBase::ResponseTypes::pingResponse);
}

TEST_F(gpcc_cood_PingResponse_TestsF, Copy_CTOR)
{
  spUUT = std::make_unique<PingResponse>();

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<PingResponse>(*spUUT);

  // check that spUUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetType(), ResponseBase::ResponseTypes::pingResponse);
  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check copy-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetType(), ResponseBase::ResponseTypes::pingResponse);
  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_PingResponse_TestsF, Move_CTOR)
{
  spUUT = std::make_unique<PingResponse>();

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<PingResponse>(std::move(*spUUT));

  // check that spUUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetType(), ResponseBase::ResponseTypes::pingResponse);
  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check move-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetType(), ResponseBase::ResponseTypes::pingResponse);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  auto rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_PingResponse_TestsF, GetBinarySize)
{
  // (1) empty return stack
  spUUT = std::make_unique<PingResponse>();

  size_t const binSize = spUUT->GetBinarySize();
  EXPECT_NE(binSize, 0U);
  EXPECT_LT(binSize, ResponseBase::minimumUsefulResponseSize);

  // (2) two items on return stack
  spUUT = std::make_unique<PingResponse>();

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  EXPECT_EQ(spUUT->GetBinarySize(), binSize + (2U * ReturnStackItem::binarySize));
}

TEST_F(gpcc_cood_PingResponse_TestsF, SerializeAndDeserialize)
{
  // create a ping response
  auto spUUT1 = std::make_unique<PingResponse>();

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

  // check type and cast to PingResponse
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::pingResponse);
  PingResponse const * const pUUT2 = &(dynamic_cast<PingResponse&>(*spUUT2Base));
  (void)pUUT2;
}

TEST_F(gpcc_cood_PingResponse_TestsF, ToString)
{
  spUUT = std::make_unique<PingResponse>();

  auto const s = spUUT->ToString();

  EXPECT_STREQ(s.c_str(), "Ping response");
}

} // namespace gpcc_tests
} // namespace cood
