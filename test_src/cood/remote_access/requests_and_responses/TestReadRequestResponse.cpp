/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/cood/remote_access/requests_and_responses/ReadRequestResponse.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"
#include <limits>

namespace gpcc_tests {
namespace cood       {

using namespace testing;

using gpcc::cood::SDOAbortCode;
using gpcc::cood::ResponseBase;
using gpcc::cood::ReadRequestResponse;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for testing class ReadRequestResponse.
// Services offered by base class ResponseBase are tested in TestResponseBase.cpp.
class gpcc_cood_ReadRequestResponse_TestsF: public Test
{
  public:
    gpcc_cood_ReadRequestResponse_TestsF(void);

  protected:
    // offset of "result" in binary
    static size_t const resultOffset = 3U;

    // offset of "b" in binary
    static size_t const bOffset = 9U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<ReturnStackItem> emptyReturnStack;
    std::vector<ReturnStackItem> const twoItemReturnStack;

    std::vector<uint8_t> someData;

    std::unique_ptr<ReadRequestResponse> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_ReadRequestResponse_TestsF::resultOffset;
size_t const gpcc_cood_ReadRequestResponse_TestsF::bOffset;

gpcc_cood_ReadRequestResponse_TestsF::gpcc_cood_ReadRequestResponse_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, emptyReturnStack()
, twoItemReturnStack{ rsi1, rsi2 }
, someData{ 0x56U, 0x89U }
, spUUT()
{
}

void gpcc_cood_ReadRequestResponse_TestsF::SetUp(void)
{
}

void gpcc_cood_ReadRequestResponse_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_ReadRequestResponse_DeathTestsF = gpcc_cood_ReadRequestResponse_TestsF;

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, CTOR)
{
  EXPECT_THROW(spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::OK), std::invalid_argument);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError));
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, Copy_CTOR_pos)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), _data.size() * 8U);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ReadRequestResponse>(*spUUT);

  // check that spUUT is OK
  // ======================================================
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(spUUT->GetData() == someData);

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check copy-constructed object
  // ======================================================
  EXPECT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(spUUT2->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(spUUT2->GetData() == someData);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, Copy_CTOR_neg)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ReadRequestResponse>(*spUUT);

  // check that spUUT is OK
  // ======================================================
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check copy-constructed object
  // ======================================================
  EXPECT_TRUE(spUUT2->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT2->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetData(), std::logic_error);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, Move_CTOR_pos)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), _data.size() * 8U);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ReadRequestResponse>(std::move(*spUUT));

  // check that spUUT is OK
  // ======================================================
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), 0U);
  EXPECT_TRUE(spUUT->GetData().empty());
  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check move-constructed object
  // ======================================================
  EXPECT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(spUUT2->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(spUUT2->GetData() == someData);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  auto rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, Move_CTOR_neg)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ReadRequestResponse>(std::move(*spUUT));

  // check that spUUT is OK
  // ======================================================
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check move-constructed object
  // ======================================================
  EXPECT_TRUE(spUUT2->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT2->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetData(), std::logic_error);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  auto rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, CalcMaxDataPayload)
{
  static size_t const base = 10U;

  // test corner case at minumum size
  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(base - 1U, 0U), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(base,      0U), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(base + 1U, 0U), 1U);

  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(base - 1U + 8U, ReturnStackItem::binarySize), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(base      + 8U, ReturnStackItem::binarySize), 0U);
  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(base + 1U + 8U, ReturnStackItem::binarySize), 1U);

  // test maximum
  EXPECT_EQ(gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(ResponseBase::maxResponseSize, 0U), std::numeric_limits<uint16_t>::max());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, GetBinarySize)
{
  // Create a read response containing 8 bytes of data payload. The binary size of this response shall still fit into
  // the minimum useful response size. A payload of 8 bytes is choosen because it allows to read all primitive
  // CANopen data types.
  someData.resize(8U);
  auto _data = someData;

  // (1) empty return stack
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(_data), _data.size() * 8U);

  size_t const binSize = spUUT->GetBinarySize();
  EXPECT_GT(binSize, 8U);
  EXPECT_LT(binSize, ResponseBase::minimumUsefulResponseSize);

  // (2) two items on return stack
  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  EXPECT_EQ(spUUT->GetBinarySize(), binSize + (2U * ReturnStackItem::binarySize));
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SerializeAndDeserialize_BadStatus)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

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

  // check type and cast to ReadRequestResponse
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::readRequestResponse);
  ReadRequestResponse const * const pUUT2 = &(dynamic_cast<ReadRequestResponse&>(*spUUT2Base));

  // check deserialized object
  EXPECT_TRUE(pUUT2->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)pUUT2->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)pUUT2->GetData(), std::logic_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SerializeAndDeserialize_NoData)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  someData.clear();
  auto _data = someData;
  spUUT1->SetData(std::move(_data), _data.size() * 8U);

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

  // check type and cast to ReadRequestResponse
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::readRequestResponse);
  ReadRequestResponse const * const pUUT2 = &(dynamic_cast<ReadRequestResponse&>(*spUUT2Base));

  // check deserialized object
  EXPECT_TRUE(pUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(pUUT2->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(pUUT2->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SerializeAndDeserialize_WithData)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  auto _data = someData;
  spUUT1->SetData(std::move(_data), _data.size() * 8U);

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

  // check type and cast to ReadRequestResponse
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::readRequestResponse);
  ReadRequestResponse const * const pUUT2 = &(dynamic_cast<ReadRequestResponse&>(*spUUT2Base));

  // check deserialized object
  EXPECT_TRUE(pUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(pUUT2->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(pUUT2->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SerializeAndDeserialize_WithDataAndSomeBits)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  auto _data = someData;
  spUUT1->SetData(std::move(_data), (_data.size() * 8U) - 4U);

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

  // check type and cast to ReadRequestResponse
  ASSERT_EQ(spUUT2Base->GetType(), ResponseBase::ResponseTypes::readRequestResponse);
  ReadRequestResponse const * const pUUT2 = &(dynamic_cast<ReadRequestResponse&>(*spUUT2Base));

  // check deserialized object
  EXPECT_TRUE(pUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_EQ(pUUT2->GetDataSize(), (someData.size() * 8U) - 4U);
  EXPECT_TRUE(pUUT2->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, Deserialize_InvalidSDOAbortCode)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

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

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SerializeAndDeserialize_InvalidNbOfBits_WithData)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  auto _data = someData;
  spUUT1->SetData(std::move(_data), _data.size() * 8U);

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

  // manipulate binary: Set number of bits in last byte to zero
  storage[bOffset + 0U] = 0U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);

  // manipulate binary: Set number of bits in last byte to 9
  storage[bOffset + 0U] = 9U;

  // try to deserialize
  msr = gpcc::Stream::MemStreamReader(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SerializeAndDeserialize_InvalidNbOfBits_NoData)
{
  // create a read request response
  auto spUUT1 = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  someData.clear();
  auto _data = someData;
  spUUT1->SetData(std::move(_data), _data.size() * 8U);

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

  // manipulate binary: Set number of bits in last byte to one
  storage[bOffset + 0U] = 1U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, ToString_GoodStatus)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(someData), someData.size() * 8U);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Read request response*", false)) << "Information about response type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*OK*", true)) << "Result is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*2.0 byte(s)*", false)) << "Information about amount of data is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x56 0x89", true)) << "Data is missing";
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, ToString_GoodStatus_SomeBits)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(someData), (someData.size() * 8U) - 4U);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Read request response*", false)) << "Information about response type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*OK*", true)) << "Result is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*1.4 byte(s)*", false)) << "Information about amount of data is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x56 0x89", true)) << "Data is missing";
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, ToString_BadStatus)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Read request response*", false)) << "Information about response type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*General error*", false)) << "Result is missing";
  EXPECT_FALSE(TestSimplePatternMatch(s, "*data*", false)) << "Data should not be present";
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetError_UpdateErrorCode)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);

  // check: error code can be updated to a different code
  spUUT->SetError(SDOAbortCode::GeneralParamIncompatibility);
  EXPECT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralParamIncompatibility);
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetError_RemoveData)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(someData), someData.size() * 8U);

  // check: Setting an error code removed the data from the response
  spUUT->SetError(SDOAbortCode::UnsupportedAccessToObject);
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::UnsupportedAccessToObject);
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_Initial)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  // set data
  auto _data = someData;
  spUUT->SetData(std::move(_data), _data.size() * 8U);
  EXPECT_TRUE(_data.empty()) << "Data was not consumed";

  // check: data was set and result was set to OK
  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(spUUT->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_Initial_SomeBits)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  // set data
  auto _data = someData;
  spUUT->SetData(std::move(_data), (_data.size() * 8U) - 4U);
  EXPECT_TRUE(_data.empty()) << "Data was not consumed";

  // check: data was set and result was set to OK
  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), (someData.size() * 8U) - 4U);
  EXPECT_TRUE(spUUT->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_ReplacePreviousData)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  // (1) set data
  std::vector<uint8_t> zeroData(8U);
  spUUT->SetData(std::move(zeroData), zeroData.size() * 8U);

  // (2) set differenty data
  auto _data = someData;
  spUUT->SetData(std::move(_data), _data.size() * 8U);
  EXPECT_TRUE(_data.empty()) << "_data was not consumed";

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), someData.size() * 8U);
  EXPECT_TRUE(spUUT->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_ReplacePreviousData_SomeBits)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  // (1) set data
  std::vector<uint8_t> zeroData(8U);
  spUUT->SetData(std::move(zeroData), zeroData.size() * 8U);

  // (2) set differenty data
  auto _data = someData;
  spUUT->SetData(std::move(_data), (_data.size() * 8U) - 4U);
  EXPECT_TRUE(_data.empty()) << "_data was not consumed";

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), (someData.size() * 8U) - 4U);
  EXPECT_TRUE(spUUT->GetData() == someData);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_Zero)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  std::vector<uint8_t> data;
  ASSERT_NO_THROW(spUUT->SetData(std::move(data), data.size() * 8U));

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), 0U);
  EXPECT_TRUE(spUUT->GetData().empty());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_Max)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  std::vector<uint8_t> data(std::numeric_limits<uint16_t>::max());
  ASSERT_NO_THROW(spUUT->SetData(std::move(data), data.size() * 8U));

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->GetDataSize(), std::numeric_limits<uint16_t>::max() * 8UL);
  EXPECT_EQ(spUUT->GetData().size(), std::numeric_limits<uint16_t>::max());
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_MaxPlus1)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  std::vector<uint8_t> data(std::numeric_limits<uint16_t>::max() + 1U);
  ASSERT_THROW(spUUT->SetData(std::move(data), data.size() * 8U), std::invalid_argument);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError) << "Error status was modified";
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);
}

TEST_F(gpcc_cood_ReadRequestResponse_TestsF, SetData_NbOfBitsDoesNotMatch)
{
  spUUT = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);

  // set data
  auto _data = someData;
  ASSERT_THROW(spUUT->SetData(std::move(_data), (_data.size() * 8U) + 1U), std::logic_error);
  EXPECT_TRUE(_data == someData);

  EXPECT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError) << "Error status was modified";
  EXPECT_THROW((void)spUUT->GetDataSize(), std::logic_error);
  EXPECT_THROW((void)spUUT->GetData(), std::logic_error);
}

} // namespace gpcc_tests
} // namespace cood
