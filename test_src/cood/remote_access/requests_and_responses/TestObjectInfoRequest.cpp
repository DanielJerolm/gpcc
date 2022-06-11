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

#include "gpcc/src/cood/remote_access/requests_and_responses/ObjectInfoRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"
#include <limits>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::ObjectInfoRequest;
using gpcc::cood::RequestBase;
using gpcc::cood::ReturnStackItem;
using gpcc::container::IntrusiveDList;

// Test fixture for testing class ObjectInfoRequest.
// Services offered by base class RequestBase are tested in TestRequestBase.cpp.
class gpcc_cood_ObjectInfoRequest_TestsF: public Test
{
  public:
    gpcc_cood_ObjectInfoRequest_TestsF(void);

  protected:
    // Standard value for maximum response size used in this test-fixture.
    static size_t const stdMaxResponseSize = 1024U;

    // Offset of "firstSubindex" in binary
    static size_t const offsetOfFirstSubindex = 9U;


    ReturnStackItem const rsi1;
    ReturnStackItem const rsi2;

    std::unique_ptr<ObjectInfoRequest> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;
};

size_t const gpcc_cood_ObjectInfoRequest_TestsF::stdMaxResponseSize;
size_t const gpcc_cood_ObjectInfoRequest_TestsF::offsetOfFirstSubindex;

gpcc_cood_ObjectInfoRequest_TestsF::gpcc_cood_ObjectInfoRequest_TestsF(void)
: Test()
, rsi1(0U, 1U)
, rsi2(2U, 3U)
, spUUT()
{
}

void gpcc_cood_ObjectInfoRequest_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectInfoRequest_TestsF::TearDown(void)
{
  spUUT.reset();
}

// alias for death tests
using gpcc_cood_ObjectInfoRequest_DeathTestsF = gpcc_cood_ObjectInfoRequest_TestsF;

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_OK_fullObj)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 0U, 255U, true, true, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 255U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), true);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), true);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_OK_fullObj_noNames)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 0U, 255U, false, true, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 255U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), false);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), true);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_OK_fullObj_noNames_noAsm)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 0U, 255U, false, false, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 255U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), false);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), false);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_OK_singleSI)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 10U, 10U, true, true, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 10U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 10U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), true);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), true);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_OK_singleSI_noNames)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 10U, 10U, false, true, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 10U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 10U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), false);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), true);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_OK_singleSI_noNames_noAsm)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 10U, 10U, false, false, stdMaxResponseSize));

  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), stdMaxResponseSize);

  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 10U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 10U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), false);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), false);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, CTOR_invalid_subindices)
{
  EXPECT_THROW(spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 10U, 9U, true, true, stdMaxResponseSize), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, Copy_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 0U, 255U, true, true, stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<ObjectInfoRequest>(*spUUT);

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 0U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 255U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), true);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), true);
  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);

  // check copy-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT2->GetFirstSubIndex(), 0U);
  EXPECT_EQ(spUUT2->GetLastSubIndex(), 255U);
  EXPECT_EQ(spUUT2->IsInclusiveNames(), true);
  EXPECT_EQ(spUUT2->IsInclusiveAppSpecificMetaData(), true);
  EXPECT_EQ(spUUT2->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, Move_CTOR)
{
  uint32_t const expectedMaxResponseSize = stdMaxResponseSize + (2U * gpcc::cood::ReturnStackItem::binarySize);

  spUUT = std::make_unique<ObjectInfoRequest>(0x1002U, 10U, 20, false, false, stdMaxResponseSize);
  spUUT->Push(rsi1);
  spUUT->Push(rsi2);

  auto spUUT2 = std::make_unique<ObjectInfoRequest>(std::move(*spUUT));

  // check that UUT is OK
  // ======================================================
  EXPECT_EQ(spUUT->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT->GetFirstSubIndex(), 10U);
  EXPECT_EQ(spUUT->GetLastSubIndex(), 20U);
  EXPECT_EQ(spUUT->IsInclusiveNames(), false);
  EXPECT_EQ(spUUT->IsInclusiveAppSpecificMetaData(), false);
  EXPECT_EQ(spUUT->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT->GetReturnStackSize(), 0U);
  std::vector<ReturnStackItem> rs;
  spUUT->ExtractReturnStack(rs);
  ASSERT_TRUE(rs.empty());

  // check move-constructed object
  // ======================================================
  EXPECT_EQ(spUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(spUUT2->GetFirstSubIndex(), 10U);
  EXPECT_EQ(spUUT2->GetLastSubIndex(), 20U);
  EXPECT_EQ(spUUT2->IsInclusiveNames(), false);
  EXPECT_EQ(spUUT2->IsInclusiveAppSpecificMetaData(), false);
  EXPECT_EQ(spUUT2->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(spUUT2->GetMaxResponseSize(), expectedMaxResponseSize);

  EXPECT_EQ(spUUT2->GetReturnStackSize(), 2U * gpcc::cood::ReturnStackItem::binarySize);
  spUUT2->ExtractReturnStack(rs);
  ASSERT_EQ(rs.size(), 2U);
  EXPECT_TRUE(rs[0] == rsi1);
  EXPECT_TRUE(rs[1] == rsi2);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, GetBinarySize)
{
  spUUT = std::make_unique<ObjectInfoRequest>(0x1000U, 0U, 255U, true, true, stdMaxResponseSize);

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

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, SerializeAndDeserialize)
{
  // create a request
  auto spUUT1 = std::make_unique<ObjectInfoRequest>(0x1002U, 0U, 255U, true, true, stdMaxResponseSize);

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

  // check type and cast to ObjectInfoRequest
  ASSERT_EQ(spUUT2Base->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  ObjectInfoRequest* const pUUT2 = &(dynamic_cast<ObjectInfoRequest&>(*spUUT2Base));

  // check deserialized object
  EXPECT_EQ(pUUT2->GetIndex(), 0x1002U);
  EXPECT_EQ(pUUT2->GetFirstSubIndex(), 0U);
  EXPECT_EQ(pUUT2->GetLastSubIndex(), 255U);
  EXPECT_EQ(pUUT2->IsInclusiveNames(), true);
  EXPECT_EQ(pUUT2->IsInclusiveAppSpecificMetaData(), true);
  EXPECT_EQ(pUUT2->GetType(), RequestBase::RequestTypes::objectInfoRequest);
  EXPECT_EQ(pUUT2->GetMaxResponseSize(), stdMaxResponseSize);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, Deserialize_InvalidSubindices)
{
  // create a request
  auto spUUT1 = std::make_unique<ObjectInfoRequest>(0x1002U, 10U, 10U, true, true, stdMaxResponseSize);

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

  // manipulate binary: Set first subindex to 11, which is larger than last subindex -> invalid serialized object
  storage[offsetOfFirstSubindex] = 11U;

  // try to deserialize
  gpcc::Stream::MemStreamReader msr(storage, reqSize, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_THROW((void)RequestBase::FromBinary(msr), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectInfoRequest_TestsF, ToString)
{
  using gpcc::string::TestSimplePatternMatch;

  spUUT = std::make_unique<ObjectInfoRequest>(0x1000U, 10U, 11U, true, true, stdMaxResponseSize);

  auto const s = spUUT->ToString();

  EXPECT_TRUE(TestSimplePatternMatch(s, "*info request*", false)) << "Information about request type is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*0x1000*", true)) << "Object's index is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*10..11*", true)) << "Subindex range is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*incl. names*", true)) << "Inclusive names is missing";
  EXPECT_TRUE(TestSimplePatternMatch(s, "*incl. asm*", true)) << "Inclusive asm is missing";
}

} // namespace gpcc_tests
} // namespace cood
