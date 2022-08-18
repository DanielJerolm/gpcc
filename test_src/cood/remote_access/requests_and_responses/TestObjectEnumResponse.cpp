/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp"
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
using gpcc::cood::ObjectEnumResponse;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for testing class ObjectEnumResponse.
// Services offered by base class ResponseBase are tested in TestResponseBase.cpp.
class gpcc_cood_ObjectEnumResponse_TestsF: public Test
{
  public:
    gpcc_cood_ObjectEnumResponse_TestsF(void);

  protected:
    // Offset of "various bits" in binary
    static size_t const offsetOfVariousBits = 7U;

    // Offset of "size" in binary
    static size_t const offsetOfSize = 8U;

    // Offset of first object index in binary
    static size_t const offsetOfFirstIndex = 10U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::vector<ReturnStackItem> emptyReturnStack;
    std::vector<ReturnStackItem> const twoItemReturnStack;

    std::vector<uint16_t> someData;

    std::unique_ptr<ObjectEnumResponse> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;

    std::unique_ptr<ObjectEnumResponse> SerializeAndDeserialize(ObjectEnumResponse const & oer);
};

size_t const gpcc_cood_ObjectEnumResponse_TestsF::offsetOfVariousBits;
size_t const gpcc_cood_ObjectEnumResponse_TestsF::offsetOfSize;
size_t const gpcc_cood_ObjectEnumResponse_TestsF::offsetOfFirstIndex;

gpcc_cood_ObjectEnumResponse_TestsF::gpcc_cood_ObjectEnumResponse_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, emptyReturnStack()
, twoItemReturnStack{ rsi1, rsi2 }
, someData{ 0x1000U, 0x1001U }
, spUUT()
{
}

void gpcc_cood_ObjectEnumResponse_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectEnumResponse_TestsF::TearDown(void)
{
  spUUT.reset();
}

std::unique_ptr<ObjectEnumResponse> gpcc_cood_ObjectEnumResponse_TestsF::SerializeAndDeserialize(ObjectEnumResponse const & oer)
{
  size_t const reqSize = oer.GetBinarySize();
  if (reqSize == 0U)
    throw std::logic_error("gpcc_cood_ObjectEnumResponse_TestsF::SerializeAndDeserialize: oer.GetBinarySize() returns zero");

  std::unique_ptr<uint8_t[]> spStorage = std::make_unique<uint8_t[]>(reqSize);

  // serialize
  gpcc::Stream::MemStreamWriter msw(spStorage.get(), reqSize, gpcc::Stream::IStreamWriter::Endian::Little);
  oer.ToBinary(msw);
  msw.AlignToByteBoundary(false);
  if (msw.GetState() != gpcc::Stream::IStreamWriter::States::full)
    throw std::logic_error("gpcc_cood_ObjectEnumResponse_TestsF::SerializeAndDeserialize: msw was not fully used.");
  msw.Close();

  // deserialize
  gpcc::Stream::MemStreamReader msr(spStorage.get(), reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  auto spUUT2Base = ResponseBase::FromBinary(msr);
  if (msr.GetState() != gpcc::Stream::IStreamReader::States::empty)
    throw std::logic_error("gpcc_cood_ObjectEnumResponse_TestsF::SerializeAndDeserialize: Stream was not completely consumed");
  msr.Close();

  // check type and cast to ObjectInfoRequest
  ObjectEnumResponse * const pRet = &dynamic_cast<ObjectEnumResponse&>(*spUUT2Base);
  spUUT2Base.release();

  return std::unique_ptr<ObjectEnumResponse>(pRet);
}

// alias for death tests
using gpcc_cood_ObjectEnumResponse_DeathTestsF = gpcc_cood_ObjectEnumResponse_TestsF;


TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, CTOR_OkNotAccepted)
{
  ASSERT_THROW(spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::OK), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, CTOR_badStatus)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError));

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, Copy_CTOR_StatusOK)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), true);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ObjectEnumResponse>(*spUUT);

  // check that spUUT is OK
  // ======================================================
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));
  EXPECT_TRUE(spUUT->GetIndices() == someData);

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check copy-constructed object
  // ======================================================
  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));
  EXPECT_TRUE(spUUT2->GetIndices() == someData);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, Copy_CTOR_StatusBad)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ObjectEnumResponse>(*spUUT);

  // check that spUUT is OK
  // ======================================================
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);

  ASSERT_FALSE(spUUT->IsReturnStackEmpty());

  auto rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check copy-constructed object
  // ======================================================
  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT2->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetIndices(), std::logic_error);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, Move_CTOR_StatusOK)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), true);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ObjectEnumResponse>(std::move(*spUUT));

  // check that spUUT is OK
  // ======================================================
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->GetIndices().empty());
  EXPECT_TRUE(spUUT->IsReturnStackEmpty());

  // check move-constructed object
  // ======================================================
  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));
  EXPECT_TRUE(spUUT2->GetIndices() == someData);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  auto rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, Move_CTOR_StatusBad)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  auto spUUT2 = std::make_unique<ObjectEnumResponse>(std::move(*spUUT));

  // check that spUUT is OK
  // ======================================================
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);

  // check move-constructed object
  // ======================================================
  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT2->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetIndices(), std::logic_error);

  ASSERT_FALSE(spUUT2->IsReturnStackEmpty());

  auto rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi2);

  rsi = spUUT2->PopReturnStack();
  EXPECT_TRUE(rsi == rsi1);

  EXPECT_TRUE(spUUT2->IsReturnStackEmpty());
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, CalcMaxNbOfIndices)
{
  static size_t const base = 10U;

  // test corner case at minumum size
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base - 1U, 0U), 0U);
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base,      0U), 0U);
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base + 1U, 0U), 0U);
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base + 2U, 0U), 1U);

  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base - 1U + 8U, ReturnStackItem::binarySize), 0U);
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base      + 8U, ReturnStackItem::binarySize), 0U);
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base + 1U + 8U, ReturnStackItem::binarySize), 0U);
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(base + 2U + 8U, ReturnStackItem::binarySize), 1U);

  // test maximum
  EXPECT_EQ(ObjectEnumResponse::CalcMaxNbOfIndices(ResponseBase::maxResponseSize, 0U), 0x10000UL);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, GetBinarySize)
{
  // Create a enum response containing 1 object index as payload. The binary size of this response shall still fit into
  // the minimum useful response size.

  someData.resize(1U);

  // (1) empty return stack
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), true);

  size_t const binSize = spUUT->GetBinarySize();
  EXPECT_GT(binSize, 2U);
  EXPECT_LT(binSize, ResponseBase::minimumUsefulResponseSize);

  // (2) two items on return stack
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  _data = someData;
  spUUT->SetData(std::move(_data), true);

  auto rs = twoItemReturnStack;
  spUUT->SetReturnStack(std::move(rs));

  EXPECT_EQ(spUUT->GetBinarySize(), binSize + (2U * ReturnStackItem::binarySize));
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_BadStatusCode)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_EQ(spUUT2->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT2->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT2->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_NoData_Complete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  spUUT->SetData(std::move(someData), true);

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));
  EXPECT_TRUE(spUUT2->GetIndices() == someData);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_WithData_Complete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), true);

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT2->IsComplete(nullptr));
  EXPECT_TRUE(spUUT2->GetIndices() == someData);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_WithData_NotComplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), false);

  auto spUUT2 = SerializeAndDeserialize(*spUUT);

  ASSERT_TRUE(spUUT2->GetResult() == SDOAbortCode::OK);
  uint16_t nextIndex;
  EXPECT_FALSE(spUUT2->IsComplete(&nextIndex));
  EXPECT_EQ(nextIndex, 0x1002U);
  EXPECT_TRUE(spUUT2->GetIndices() == someData);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_ExceedMaxNbOfIndices)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(someData), true);

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

  // manipulate binary
  storage[offsetOfVariousBits] |= 0x2U; // set MSB
  storage[offsetOfSize + 0U] = 0x01U;   // LSB of size = 0x0001
  storage[offsetOfSize + 1U] = 0x00U;

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  ASSERT_EQ(msr.RemainingBytes(), 4U) << "Looks like uut did not throw at expected error check";
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_MaxNbOfIndicesButNotComplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(someData), false);

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

  // manipulate binary
  storage[offsetOfVariousBits] |= 0x2U; // set MSB
  storage[offsetOfSize + 0U] = 0x00U;   // LSB of size = 0x0000
  storage[offsetOfSize + 1U] = 0x00U;

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  ASSERT_EQ(msr.RemainingBytes(), 4U) << "Looks like uut did not throw at expected error check";
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_IndicesNotAscending)
{
  someData.clear();
  someData.push_back(0x0100U);
  someData.push_back(0x0101U);
  someData.push_back(0x0102U);

  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  spUUT->SetData(std::move(someData), false);

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

  // manipulate binary
  storage[offsetOfFirstIndex + 0U] = 0x05U; // index[0] = 0x0005
  storage[offsetOfFirstIndex + 1U] = 0x00U;
  storage[offsetOfFirstIndex + 2U] = 0x04U; // index[1] = 0x0004
  storage[offsetOfFirstIndex + 3U] = 0x00U;

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::open) << "Looks like uut did not throw at expected error check";
  ASSERT_EQ(msr.RemainingBytes(), 2U) << "Looks like uut did not throw at expected error check";
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_LastIndexIncludedButNotComplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x0100U);
  someData.push_back(0xFFFFU);
  spUUT->SetData(std::move(someData), true);

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

  // manipulate binary
  storage[offsetOfVariousBits] &= ~0x1U; // clear "complete"

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Looks like uut did not throw at expected error check";
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SerializeAndDeserialize_EmptyButNotComplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  spUUT->SetData(std::move(someData), true);

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

  // manipulate binary
  storage[offsetOfVariousBits] &= ~0x1U; // clear "complete"

  // try to deserialize it
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  ASSERT_THROW((void)ResponseBase::FromBinary(msr), std::runtime_error);
  ASSERT_TRUE(msr.GetState() == gpcc::Stream::IStreamReader::States::empty) << "Looks like uut did not throw at expected error check";
  msr.Close();
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, ToString_BadStatusCode)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Object enum response*", false)) << "Information about response type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*General error*", false)) << "Error information is missing";
  EXPECT_FALSE(TestSimplePatternMatch(s, "*indices*", false)) << "Due to bad status code, indices are not anticipated";
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, ToString_WithData)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto _data = someData;
  spUUT->SetData(std::move(_data), true);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*Object enum response*", false)) << "Information about response type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*complete*", false));
  EXPECT_TRUE(TestSimplePatternMatch(s, "*indices*", false));
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetError)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  // check that "OK" cannot be set
  ASSERT_THROW(spUUT->SetError(SDOAbortCode::OK), std::invalid_argument);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);

  // check that setting a different error works
  ASSERT_NO_THROW(spUUT->SetError(SDOAbortCode::OBDDynGenFailedOrODNotPresent));

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OBDDynGenFailedOrODNotPresent);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetErrorClearsData)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  spUUT->SetData(std::move(someData), true);

  spUUT->SetError(SDOAbortCode::OBDDynGenFailedOrODNotPresent);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OBDDynGenFailedOrODNotPresent);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetData_TooLarge)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.resize(0x10001UL);

  ASSERT_THROW(spUUT->SetData(std::move(someData), true), std::invalid_argument);
  EXPECT_EQ(someData.size(), 0x10001UL) << "Content of vector has been moved. That was not anticipated.";

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetData_MaxSize)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.reserve(0x10000UL);
  someData.clear();
  for (uint_fast32_t i = 0U; i <= 0xFFFFU; ++i)
    someData.push_back(i);

  auto _someData = someData;

  ASSERT_NO_THROW(spUUT->SetData(std::move(_someData), true));

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));
  EXPECT_TRUE(spUUT->GetIndices() == someData);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetData_IncompleteButEmpty)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  ASSERT_THROW(spUUT->SetData(std::move(someData), false), std::invalid_argument);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetData_IncompleteBut0xFFFFincluded)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x0001U);
  someData.push_back(0xFFFFU);

  ASSERT_THROW(spUUT->SetData(std::move(someData), false), std::invalid_argument);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, SetData_NotProperlySorted)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x0002U);
  someData.push_back(0x0001U);

  ASSERT_THROW(spUUT->SetData(std::move(someData), false), std::invalid_argument);

  ASSERT_EQ(spUUT->GetResult(), SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spUUT->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spUUT->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, IsComplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  // (1) complete
  auto _someData = someData;
  spUUT->SetData(std::move(_someData), true);

  uint16_t next = 0xFFFFU;
  ASSERT_TRUE(spUUT->IsComplete(nullptr));
  ASSERT_TRUE(spUUT->IsComplete(&next));
  EXPECT_EQ(next, 0xFFFFU);

  // (2) not complete
  _someData = someData;
  spUUT->SetData(std::move(_someData), false);

  next = 0xFFFFU;
  ASSERT_FALSE(spUUT->IsComplete(nullptr));
  ASSERT_FALSE(spUUT->IsComplete(&next));
  EXPECT_EQ(next, 0x1002U);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, AddFragment_ThisNotOK)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  auto spOther = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  auto _someData = someData;
  spOther->SetData(std::move(_someData), true);

  EXPECT_THROW(spUUT->AddFragment(*spOther), std::logic_error);

  // check that the 2nd fragment is still OK
  ASSERT_TRUE(spOther->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spOther->IsComplete(nullptr));
  EXPECT_TRUE(spOther->GetIndices() == someData);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, AddFragment_ThisIsAlreadyComplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  auto someData1 = someData;
  spUUT->SetData(std::move(someData), true);

  auto spOther = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x2000U);
  someData.push_back(0x2001U);
  auto someData2 = someData;
  spOther->SetData(std::move(someData), true);

  EXPECT_THROW(spUUT->AddFragment(*spOther), std::logic_error);

  // check that first fragment has not been modified
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));
  EXPECT_TRUE(spUUT->GetIndices() == someData1);

  // check that second fragment has not been modified
  ASSERT_TRUE(spOther->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spOther->IsComplete(nullptr));
  EXPECT_TRUE(spOther->GetIndices() == someData2);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, AddFragment_OtherNotOK)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  auto someData1 = someData;
  spUUT->SetData(std::move(someData), false);

  auto spOther = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  EXPECT_THROW(spUUT->AddFragment(*spOther), std::logic_error);

  // check that first fragment has not been modified
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));
  EXPECT_TRUE(spUUT->GetIndices() == someData1);

  // check that second fragment has not been modified
  ASSERT_TRUE(spOther->GetResult() == SDOAbortCode::GeneralError);
  EXPECT_THROW((void)spOther->IsComplete(nullptr), std::logic_error);
  EXPECT_THROW((void)spOther->GetIndices(), std::logic_error);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, AddFragment_Discontinuity)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  auto someData1 = someData;
  spUUT->SetData(std::move(someData), false);

  auto spOther = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x1001U);
  someData.push_back(0x2001U);
  auto someData2 = someData;
  spOther->SetData(std::move(someData), true);

  EXPECT_THROW(spUUT->AddFragment(*spOther), std::invalid_argument);

  // check that first fragment has not been modified
  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_FALSE(spUUT->IsComplete(nullptr));
  EXPECT_TRUE(spUUT->GetIndices() == someData1);

  // check that second fragment has not been modified
  ASSERT_TRUE(spOther->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spOther->IsComplete(nullptr));
  EXPECT_TRUE(spOther->GetIndices() == someData2);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, AddFragment_OK_incomplete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  auto someData1 = someData;
  spUUT->SetData(std::move(someData), false);

  auto spOther = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x2001U);
  someData.push_back(0x2002U);
  auto someData2 = someData;
  spOther->SetData(std::move(someData), false);

  EXPECT_FALSE(spUUT->IsComplete(nullptr));
  EXPECT_FALSE(spOther->IsComplete(nullptr));

  ASSERT_NO_THROW(spUUT->AddFragment(*spOther));

  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  uint16_t next = 0xFFFFU;
  EXPECT_FALSE(spUUT->IsComplete(nullptr));
  EXPECT_FALSE(spUUT->IsComplete(&next));
  EXPECT_EQ(next, 0x2003U);
  for (auto const i : someData2)
    someData1.push_back(i);
  EXPECT_TRUE(spUUT->GetIndices() == someData1);
}

TEST_F(gpcc_cood_ObjectEnumResponse_TestsF, AddFragment_OK_complete)
{
  spUUT = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  auto someData1 = someData;
  spUUT->SetData(std::move(someData), false);

  auto spOther = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);

  someData.clear();
  someData.push_back(0x2001U);
  someData.push_back(0x2002U);
  auto someData2 = someData;
  spOther->SetData(std::move(someData), true);

  EXPECT_FALSE(spUUT->IsComplete(nullptr));
  EXPECT_TRUE(spOther->IsComplete(nullptr));

  ASSERT_NO_THROW(spUUT->AddFragment(*spOther));

  ASSERT_TRUE(spUUT->GetResult() == SDOAbortCode::OK);
  EXPECT_TRUE(spUUT->IsComplete(nullptr));
  for (auto const i : someData2)
    someData1.push_back(i);
  EXPECT_TRUE(spUUT->GetIndices() == someData1);
}

} // namespace gpcc_tests
} // namespace cood
