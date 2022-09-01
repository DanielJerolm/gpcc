/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include <gpcc/cood/ObjectARRAY_wicb.hpp>
#include <gpcc/cood/exceptions.hpp>
#include <gpcc/osal/Mutex.hpp>
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include "IObjectNotifiableMock.hpp"
#include "gtest/gtest.h"
#include <limits>
#include <memory>
#include <random>
#include <cstring>

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;
using namespace gpcc::Stream;

using namespace testing;

/// Test fixture for gpcc::cood::ObjectARRAY_wicb related tests.
class gpcc_cood_ObjectARRAY_wicb_TestsF: public Test
{
  public:
    gpcc_cood_ObjectARRAY_wicb_TestsF(void);

  protected:
    virtual ~gpcc_cood_ObjectARRAY_wicb_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

    void CreateUUT_BOOLEAN_RWRW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_INTEGER8_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_UNSIGNED8_RORW(uint8_t const SI0);
    void CreateUUT_UNSIGNED8_RORW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_UNSIGNED8_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_UNSIGNED8_RWRO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_UNSIGNED8_RWWO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_UNSIGNED32_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);

    void CreateUUT_BIT1_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_BIT1_RWRO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_BIT1_RWWO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_BIT6_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);
    void CreateUUT_BIT8_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);

    void CreateUUT_BOOLEAN_NATIVE_BIT1_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0);

    void Randomize_data_bool(void);
    void Randomize_data_i8(void);
    void Randomize_data_ui8(void);
    void Randomize_data_ui32(void);
    void Randomize_data_bitX(void);

    void Randomize_readbuffer(void);

    // Mutex protecting the data
    gpcc::osal::Mutex mutex;

    // The data
    bool data_bool[255];
    int8_t data_i8[255];
    uint8_t data_ui8[255];
    uint32_t data_ui32[255];
    uint8_t data_bitX[255];

    // Mock for reception of callbacks
    StrictMock<IObjectNotifiableMock> cbm;

    // Buffers for use with MemStreamReader and MemStreamWriter (255 x 4 + 2)
    uint8_t readBuffer[1022];
    uint8_t writeBuffer[1022];

    // Stream reader/writer for the buffers above
    gpcc::Stream::MemStreamReader readBufferReader;
    gpcc::Stream::MemStreamWriter writeBufferWriter;

    // Random number generator
    std::default_random_engine dre;

    // ...and finally the UUT
    std::unique_ptr<ObjectARRAY_wicb> spUUT;
};

gpcc_cood_ObjectARRAY_wicb_TestsF::gpcc_cood_ObjectARRAY_wicb_TestsF(void)
: Test()
, mutex()
, data_bool()
, data_i8()
, data_ui8()
, data_ui32()
, data_bitX()
, cbm()
, readBuffer()
, writeBuffer()
, readBufferReader(&readBuffer, sizeof(readBuffer), gpcc::Stream::MemStreamReader::Endian::Little)
, writeBufferWriter(&writeBuffer, sizeof(writeBuffer), gpcc::Stream::MemStreamWriter::Endian::Little)
, dre(1)
, spUUT()
{
}

gpcc_cood_ObjectARRAY_wicb_TestsF::~gpcc_cood_ObjectARRAY_wicb_TestsF(void)
{
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::TearDown(void)
{
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BOOLEAN_RWRW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::boolean,
                              Object::attr_ACCESS_RW,
                              data_bool,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_INTEGER8_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::integer8,
                              Object::attr_ACCESS_RW,
                              data_i8,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_UNSIGNED8_RORW(uint8_t const SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD_PREOP,
                              SI0,
                              SI0,
                              SI0,
                              DataType::unsigned8,
                              Object::attr_ACCESS_RD_PREOP | Object::attr_ACCESS_RD_OP | Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_WR_OP,
                              data_ui8,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_UNSIGNED8_RORW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD_PREOP,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::unsigned8,
                              Object::attr_ACCESS_RD_PREOP | Object::attr_ACCESS_RD_OP | Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_WR_OP,
                              data_ui8,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_UNSIGNED8_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD_PREOP | Object::attr_ACCESS_RD_SAFEOP | Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_WR_SAFEOP,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::unsigned8,
                              Object::attr_ACCESS_RD_PREOP | Object::attr_ACCESS_RD_OP | Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_WR_OP,
                              data_ui8,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_UNSIGNED8_RWRO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD_PREOP | Object::attr_ACCESS_RD_SAFEOP | Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_WR_SAFEOP,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::unsigned8,
                              Object::attr_ACCESS_RD_PREOP,
                              data_ui8,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_UNSIGNED8_RWWO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD_PREOP | Object::attr_ACCESS_RD_SAFEOP | Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_WR_SAFEOP,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::unsigned8,
                              Object::attr_ACCESS_WR_PREOP,
                              data_ui8,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_UNSIGNED32_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::unsigned32,
                              Object::attr_ACCESS_RW,
                              data_ui32,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BIT1_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::bit1,
                              Object::attr_ACCESS_RW,
                              data_bitX,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BIT1_RWRO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::bit1,
                              Object::attr_ACCESS_RD,
                              data_bitX,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BIT1_RWWO(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::bit1,
                              Object::attr_ACCESS_WR,
                              data_bitX,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BIT6_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::bit6,
                              Object::attr_ACCESS_RW,
                              data_bitX,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BIT8_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::bit8,
                              Object::attr_ACCESS_RW,
                              data_bitX,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::CreateUUT_BOOLEAN_NATIVE_BIT1_RW(uint8_t const SI0, uint8_t const min_SI0, uint8_t const max_SI0)
{
  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              SI0,
                              min_SI0,
                              max_SI0,
                              DataType::boolean_native_bit1,
                              Object::attr_ACCESS_RW,
                              data_bitX,
                              &mutex,
                              std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                              std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                              std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::Randomize_data_bool(void)
{
  std::uniform_int_distribution<int> uniform_dist(0, 1);

  size_t n = sizeof(data_bool) / sizeof(bool);
  for (size_t i = 0U; i < n; i++)
    data_bool[i] = (uniform_dist(dre) != 0U);
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::Randomize_data_i8(void)
{
  std::uniform_int_distribution<int> uniform_dist(std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());

  size_t n = sizeof(data_i8);
  for (size_t i = 0U; i < n; i++)
    data_i8[i] = uniform_dist(dre);
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::Randomize_data_ui8(void)
{
  std::uniform_int_distribution<int> uniform_dist(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());

  size_t n = sizeof(data_ui8);
  for (size_t i = 0U; i < n; i++)
    data_ui8[i] = uniform_dist(dre);
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::Randomize_data_ui32(void)
{
  std::uniform_int_distribution<int> uniform_dist(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

  size_t n = sizeof(data_ui32) / sizeof(uint32_t);
  for (size_t i = 0U; i < n; i++)
    data_ui32[i] = uniform_dist(dre);
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::Randomize_data_bitX(void)
{
  std::uniform_int_distribution<int> uniform_dist(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());

  size_t n = sizeof(data_bitX);
  for (size_t i = 0U; i < n; i++)
    data_bitX[i] = uniform_dist(dre);
}

void gpcc_cood_ObjectARRAY_wicb_TestsF::Randomize_readbuffer(void)
{
  std::uniform_int_distribution<int> uniform_dist(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());

  size_t n = sizeof(readBuffer);
  for (size_t i = 0U; i < n; i++)
    readBuffer[i] = uniform_dist(dre);
}

typedef gpcc_cood_ObjectARRAY_wicb_TestsF gpcc_cood_ObjectARRAY_wicb_DeathTestsF;

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, InstantiateAndDestroy_SI0rw_1)
{
  CreateUUT_BOOLEAN_RWRW(10U, 0U, 20U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, InstantiateAndDestroy_SI0rw_2)
{
  CreateUUT_BOOLEAN_RWRW(10U, 10U, 10U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, InstantiateAndDestroy_SI0ro_1)
{
  CreateUUT_UNSIGNED8_RORW(10U, 5U, 15U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, InstantiateAndDestroy_SI0ro_2)
{
  CreateUUT_UNSIGNED8_RORW(10U, 10U, 10U);
}

TEST(gpcc_cood_ObjectARRAY_wicb_Tests, Constructor_InvalidArgs)
{
  bool data[15];
  gpcc::osal::Mutex mutex;

  std::unique_ptr<ObjectARRAY_wicb> spUUT;

  // no read permission set for SI0
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_WR_PREOP,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), std::invalid_argument);

  // value of SI0 < min
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           4U,
                                           5U,
                                           15U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), std::invalid_argument);

  // value of SI0 > max
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           16U,
                                           5U,
                                           15U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), std::invalid_argument);
  // value of SI0 min > max
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           15U,
                                           5U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), std::invalid_argument);

  // data type not supported
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::visible_string,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), DataTypeNotSupportedError);

  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::octet_string,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), DataTypeNotSupportedError);

  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::integer40,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), DataTypeNotSupportedError);

  // no permission for array's data
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::boolean,
                                           Object::attr_SETTINGS,
                                           data,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), std::invalid_argument);

  // pData is nullptr
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RW,
                                           nullptr,
                                           &mutex,
                                           nullptr, nullptr, nullptr)), std::invalid_argument);

  // no mutex specified, but SI0 is RW
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RW,
                                           10U,
                                           5U,
                                           15U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RD,
                                           data,
                                           nullptr,
                                           nullptr, nullptr, nullptr)), std::logic_error);

  // no mutex specified, but array's data is RW
  EXPECT_THROW(spUUT.reset(new ObjectARRAY_wicb("ObjName",
                                           Object::attr_ACCESS_RD,
                                           10U,
                                           10U,
                                           10U,
                                           DataType::boolean,
                                           Object::attr_ACCESS_RW,
                                           data,
                                           nullptr,
                                           nullptr, nullptr, nullptr)), std::logic_error);
}

TEST(gpcc_cood_ObjectARRAY_wicb_Tests, SetData_NoMutexSpecified)
{
  bool data[15];

  std::unique_ptr<ObjectARRAY_wicb> spUUT;

  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD,
                              10U,
                              10U,
                              10U,
                              DataType::boolean,
                              Object::attr_ACCESS_RD,
                              data,
                              nullptr,
                              nullptr, nullptr, nullptr));

  EXPECT_THROW(spUUT->SetData(10, data), std::logic_error);
}

TEST(gpcc_cood_ObjectARRAY_wicb_Tests, SetData_SI0_RW_MinMaxViolated)
{
  bool data[15];
  gpcc::osal::Mutex mutex;

  std::unique_ptr<ObjectARRAY_wicb> spUUT;

  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              10U,
                              5U,
                              15U,
                              DataType::boolean,
                              Object::attr_ACCESS_RW,
                              data,
                              &mutex,
                              nullptr, nullptr, nullptr));

  EXPECT_THROW(spUUT->SetData(4, data), std::invalid_argument);
  EXPECT_THROW(spUUT->SetData(16, data), std::invalid_argument);
}

TEST(gpcc_cood_ObjectARRAY_wicb_Tests, SetData_SI0_RO_MinMaxViolated)
{
  bool data[15];
  gpcc::osal::Mutex mutex;

  std::unique_ptr<ObjectARRAY_wicb> spUUT;

  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RD,
                              10U,
                              5U,
                              15U,
                              DataType::boolean,
                              Object::attr_ACCESS_RW,
                              data,
                              &mutex,
                              nullptr, nullptr, nullptr));

  EXPECT_THROW(spUUT->SetData(4, data), std::invalid_argument);
  EXPECT_THROW(spUUT->SetData(16, data), std::invalid_argument);
}

TEST(gpcc_cood_ObjectARRAY_wicb_Tests, SetData_DataPtrNullptr)
{
  bool data[15];
  gpcc::osal::Mutex mutex;

  std::unique_ptr<ObjectARRAY_wicb> spUUT;

  spUUT.reset(new ObjectARRAY_wicb("ObjName",
                              Object::attr_ACCESS_RW,
                              10U,
                              5U,
                              15U,
                              DataType::boolean,
                              Object::attr_ACCESS_RW,
                              data,
                              &mutex,
                              nullptr, nullptr, nullptr));

  EXPECT_THROW(spUUT->SetData(10, nullptr), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, SetData_SameDataNewSI0_SI0_rw)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  Randomize_data_ui8();
  spUUT->SetData(10, data_ui8);

  auto locker(spUUT->LockData());
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 12U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 10U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(data_ui8[i], writeBuffer[2U + i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, SetData_SameDataNewSI0_SI0_ro)
{
  CreateUUT_UNSIGNED8_RORW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  Randomize_data_ui8();
  spUUT->SetData(10, data_ui8);

  auto locker(spUUT->LockData());
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 12U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 10U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(data_ui8[i], writeBuffer[2U + i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, SetData_SameDataSameSI0)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  Randomize_data_ui8();
  spUUT->SetData(100, data_ui8);

  auto locker(spUUT->LockData());
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 102U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], writeBuffer[2U + i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, SetData_NewDataNewSI0)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  Randomize_data_ui8();
  data_ui32[0] = 0x78563412UL;
  data_ui32[1] = 0xFFFFABCDUL;
  spUUT->SetData(6, data_ui32);

  auto locker(spUUT->LockData());
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 8U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 6U);
  ASSERT_EQ(writeBuffer[1], 0U);

  EXPECT_EQ(writeBuffer[2], 0x12U);
  EXPECT_EQ(writeBuffer[3], 0x34U);
  EXPECT_EQ(writeBuffer[4], 0x56U);
  EXPECT_EQ(writeBuffer[5], 0x78U);
  EXPECT_EQ(writeBuffer[6], 0xCDU);
  EXPECT_EQ(writeBuffer[7], 0xABU);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CheckLockData)
{
  CreateUUT_BOOLEAN_RWRW(255U, 0U, 255U);

  auto locker(spUUT->LockData());

  if (mutex.TryLock())
  {
    ADD_FAILURE() << "Mutex protecting the data has not been locked by ObjectARRAY_wicb::LockData()";
    mutex.Unlock();
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CheckMetaData_withoutLock_A)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Array);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::boolean);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     201U);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),          "Number of subindices");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  for (uint_fast16_t i = 1U; i <= 200U; i++)
  {
    std::string s = "Subindex " + std::to_string(i);

    ASSERT_EQ(spUUT->IsSubIndexEmpty(i),         false);
    ASSERT_EQ(spUUT->GetSubIdxDataType(i),       DataType::boolean);
    ASSERT_EQ(spUUT->GetSubIdxAttributes(i),     Object::attr_ACCESS_RW);
    ASSERT_EQ(spUUT->GetSubIdxMaxSize(i),        1U);
    ASSERT_EQ(spUUT->GetSubIdxName(i),           s);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);
  }

  for (uint_fast16_t i = 201U; i <= 255U; i++)
  {
    ASSERT_THROW((void)spUUT->IsSubIndexEmpty(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxDataType(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxAttributes(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxMaxSize(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxName(i), SubindexNotExistingError);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CheckMetaData_withLock_A)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Array);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::boolean);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     201U);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),          "Number of subindices");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  EXPECT_EQ(spUUT->GetObjectStreamSize(false), 108U);
  EXPECT_EQ(spUUT->GetObjectStreamSize(true),  116U);

  EXPECT_EQ(spUUT->GetNbOfSubIndices(),        101U);

  EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);

  for (uint_fast16_t i = 1U; i <= 200U; i++)
  {
    std::string s = "Subindex " + std::to_string(i);

    ASSERT_EQ(spUUT->IsSubIndexEmpty(i),         false);
    ASSERT_EQ(spUUT->GetSubIdxDataType(i),       DataType::boolean);
    ASSERT_EQ(spUUT->GetSubIdxAttributes(i),     Object::attr_ACCESS_RW);
    ASSERT_EQ(spUUT->GetSubIdxMaxSize(i),        1U);
    ASSERT_EQ(spUUT->GetSubIdxName(i),           s);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);

    if (i <= 100U)
    {
      ASSERT_EQ(spUUT->GetSubIdxActualSize(i),     1U);
    }
    else
    {
      ASSERT_THROW((void)spUUT->GetSubIdxActualSize(i), SubindexNotExistingError);
    }
  }

  for (uint_fast16_t i = 201U; i <= 255U; i++)
  {
    ASSERT_THROW((void)spUUT->IsSubIndexEmpty(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxDataType(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxAttributes(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxMaxSize(i), SubindexNotExistingError);
    ASSERT_THROW((void)spUUT->GetSubIdxName(i), SubindexNotExistingError);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);

    ASSERT_THROW((void)spUUT->GetSubIdxActualSize(i), SubindexNotExistingError);
  }
}


TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CheckMetaData_withoutLock_B)
{
  CreateUUT_BOOLEAN_RWRW(255U, 0U, 255U);

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Array);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::boolean);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     256U);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),          "Number of subindices");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  for (uint_fast16_t i = 1U; i <= 255U; i++)
  {
    std::string s = "Subindex " + std::to_string(i);

    ASSERT_EQ(spUUT->IsSubIndexEmpty(i),         false);
    ASSERT_EQ(spUUT->GetSubIdxDataType(i),       DataType::boolean);
    ASSERT_EQ(spUUT->GetSubIdxAttributes(i),     Object::attr_ACCESS_RW);
    ASSERT_EQ(spUUT->GetSubIdxMaxSize(i),        1U);
    ASSERT_EQ(spUUT->GetSubIdxName(i),           s);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CheckMetaData_withLock_B)
{
  CreateUUT_BOOLEAN_RWRW(255U, 0U, 255U);

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Array);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::boolean);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     256U);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),          "Number of subindices");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  EXPECT_EQ(spUUT->GetObjectStreamSize(false), 263U);
  EXPECT_EQ(spUUT->GetObjectStreamSize(true),  271U);

  EXPECT_EQ(spUUT->GetNbOfSubIndices(),        256U);

  EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);

  for (uint_fast16_t i = 1U; i <= 255U; i++)
  {
    std::string s = "Subindex " + std::to_string(i);

    ASSERT_EQ(spUUT->IsSubIndexEmpty(i),         false);
    ASSERT_EQ(spUUT->GetSubIdxDataType(i),       DataType::boolean);
    ASSERT_EQ(spUUT->GetSubIdxAttributes(i),     Object::attr_ACCESS_RW);
    ASSERT_EQ(spUUT->GetSubIdxMaxSize(i),        1U);
    ASSERT_EQ(spUUT->GetSubIdxName(i),           s);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);

    ASSERT_EQ(spUUT->GetSubIdxActualSize(i),     1U);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CheckMetaData_DataTypeMapped_withLock_B)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW(255U, 0U, 255U);

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Array);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::boolean);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     256U);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),          "Number of subindices");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  EXPECT_EQ(spUUT->GetObjectStreamSize(false), 263U);
  EXPECT_EQ(spUUT->GetObjectStreamSize(true),  271U);

  EXPECT_EQ(spUUT->GetNbOfSubIndices(),        256U);

  EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);

  for (uint_fast16_t i = 1U; i <= 255U; i++)
  {
    std::string s = "Subindex " + std::to_string(i);

    ASSERT_EQ(spUUT->IsSubIndexEmpty(i),         false);
    ASSERT_EQ(spUUT->GetSubIdxDataType(i),       DataType::boolean);
    ASSERT_EQ(spUUT->GetSubIdxAttributes(i),     Object::attr_ACCESS_RW);
    ASSERT_EQ(spUUT->GetSubIdxMaxSize(i),        1U);
    ASSERT_EQ(spUUT->GetSubIdxName(i),           s);

    ASSERT_EQ(spUUT->GetAppSpecificMetaDataSize(i), 0U);
    ASSERT_THROW((void)spUUT->GetAppSpecificMetaData(i), std::logic_error);

    ASSERT_EQ(spUUT->GetSubIdxActualSize(i),     1U);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_SI0)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BOOLEAN)
{
  CreateUUT_BOOLEAN_RWRW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 5U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bool[4] = false;
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bool[4] = true;
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x02U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_INTEGER8)
{
  CreateUUT_INTEGER8_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U,   false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 3U,   false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 255U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_i8[0]   = 75;
  data_i8[2]   = 12;
  data_i8[254] = 17;
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(3U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(255U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 75U);
  EXPECT_EQ(writeBuffer[1], 12U);
  EXPECT_EQ(writeBuffer[2], 17U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_UNSIGNED8)
{
  CreateUUT_UNSIGNED8_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 7U,   false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 87U,  false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 253U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_ui8[6]   = 175U;
  data_ui8[86]  = 23U;
  data_ui8[252] = 87U;
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(87U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(253U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 175U);
  EXPECT_EQ(writeBuffer[1], 23U);
  EXPECT_EQ(writeBuffer[2], 87U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_UNSIGNED32)
{
  CreateUUT_UNSIGNED32_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 7U,   false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 87U,  false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 253U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_ui32[6]   = 0xABCDEF00UL;
  data_ui32[86]  = 0x12345678UL;
  data_ui32[252] = 0x13243568UL;
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(87U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(253U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0],  0x00U);
  EXPECT_EQ(writeBuffer[1],  0xEFU);
  EXPECT_EQ(writeBuffer[2],  0xCDU);
  EXPECT_EQ(writeBuffer[3],  0xABU);

  EXPECT_EQ(writeBuffer[4],  0x78U);
  EXPECT_EQ(writeBuffer[5],  0x56U);
  EXPECT_EQ(writeBuffer[6],  0x34U);
  EXPECT_EQ(writeBuffer[7],  0x12U);

  EXPECT_EQ(writeBuffer[8],  0x68U);
  EXPECT_EQ(writeBuffer[9],  0x35U);
  EXPECT_EQ(writeBuffer[10], 0x24U);
  EXPECT_EQ(writeBuffer[11], 0x13U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BIT1)
{
  CreateUUT_BIT1_RW(128U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());
  data_bitX[0]  = 0x5AU;
  data_bitX[15] = 0x36U;

  for (uint_fast8_t i = 1; i <= 8U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
  }

  for (uint_fast8_t i = 121; i <= 128U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5AU);
  EXPECT_EQ(writeBuffer[1], 0x36U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BIT6_A)
{
  CreateUUT_BIT6_RW(128U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // 1      2       3       4      5      6       7       8
  // 000000.10 0000.0100 00.001000 000100.00 0010.0000 01.111111
  // 0x40      0x20      0x10      0x08      0x04      0xFE
  data_bitX[0]  = 0x40U;
  data_bitX[1]  = 0x20U;
  data_bitX[2]  = 0x10U;
  data_bitX[3]  = 0x08U;
  data_bitX[4]  = 0x04U;
  data_bitX[5]  = 0xFEU;

  for (uint_fast8_t i = 1; i <= 8U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
    writeBufferWriter.FillBits(2U, false);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x00U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], 0x02U);
  EXPECT_EQ(writeBuffer[3], 0x04U);
  EXPECT_EQ(writeBuffer[4], 0x08U);
  EXPECT_EQ(writeBuffer[5], 0x10U);
  EXPECT_EQ(writeBuffer[6], 0x20U);
  EXPECT_EQ(writeBuffer[7], 0x3FU);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BIT6_B)
{
  CreateUUT_BIT6_RW(128U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // 1      2       3       4      5      6       7       8
  // 100110.11 0101.0111 01.110010 101101.11 0101.0101 11.101010
  // 0xC9      0xEA      0x4E      0xEC      0xAA      0x57
  data_bitX[0]  = 0xD9U;
  data_bitX[1]  = 0xEAU;
  data_bitX[2]  = 0x4EU;
  data_bitX[3]  = 0xECU;
  data_bitX[4]  = 0xAAU;
  data_bitX[5]  = 0x57U;

  for (uint_fast8_t i = 1; i <= 8U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
    writeBufferWriter.FillBits(2U, false);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x19U);
  EXPECT_EQ(writeBuffer[1], 0x2BU);
  EXPECT_EQ(writeBuffer[2], 0x2EU);
  EXPECT_EQ(writeBuffer[3], 0x13U);
  EXPECT_EQ(writeBuffer[4], 0x2CU);
  EXPECT_EQ(writeBuffer[5], 0x2BU);
  EXPECT_EQ(writeBuffer[6], 0x3AU);
  EXPECT_EQ(writeBuffer[7], 0x15U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BIT8_A)
{
  CreateUUT_BIT8_RW(128U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX[0]  = 0x00U;
  data_bitX[1]  = 0x01U;
  data_bitX[2]  = 0x02U;
  data_bitX[3]  = 0x04U;
  data_bitX[4]  = 0x08U;
  data_bitX[5]  = 0x10U;
  data_bitX[6]  = 0x20U;
  data_bitX[7]  = 0x40U;
  data_bitX[8]  = 0x80U;

  for (uint_fast8_t i = 1; i <= 9U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x00U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], 0x02U);
  EXPECT_EQ(writeBuffer[3], 0x04U);
  EXPECT_EQ(writeBuffer[4], 0x08U);
  EXPECT_EQ(writeBuffer[5], 0x10U);
  EXPECT_EQ(writeBuffer[6], 0x20U);
  EXPECT_EQ(writeBuffer[7], 0x40U);
  EXPECT_EQ(writeBuffer[8], 0x80U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BIT8_B)
{
  CreateUUT_BIT8_RW(128U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX[0]  = 0x12U;
  data_bitX[1]  = 0x23U;
  data_bitX[2]  = 0xC5U;
  data_bitX[3]  = 0xDFU;
  data_bitX[4]  = 0xE8U;
  data_bitX[5]  = 0x9AU;
  data_bitX[6]  = 0x23U;
  data_bitX[7]  = 0x45U;
  data_bitX[8]  = 0xBBU;

  for (uint_fast8_t i = 1; i <= 9U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x12U);
  EXPECT_EQ(writeBuffer[1], 0x23U);
  EXPECT_EQ(writeBuffer[2], 0xC5U);
  EXPECT_EQ(writeBuffer[3], 0xDFU);
  EXPECT_EQ(writeBuffer[4], 0xE8U);
  EXPECT_EQ(writeBuffer[5], 0x9AU);
  EXPECT_EQ(writeBuffer[6], 0x23U);
  EXPECT_EQ(writeBuffer[7], 0x45U);
  EXPECT_EQ(writeBuffer[8], 0xBBU);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_BOOLEAN_NATIVE_BIT1)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW(128U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());
  data_bitX[0]  = 0x5AU;
  data_bitX[15] = 0x36U;

  for (uint_fast8_t i = 1; i <= 8U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
  }

  for (uint_fast8_t i = 121; i <= 128U; i++)
  {
    ASSERT_EQ(spUUT->Read(i, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK) << "Error at i = " << static_cast<uint32_t>(i);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5AU);
  EXPECT_EQ(writeBuffer[1], 0x36U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_SubindexNotExisting)
{
  CreateUUT_UNSIGNED8_RW(6U, 0U, 12U);

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::SubindexDoesNotExist);
  EXPECT_EQ(spUUT->Read(13U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::SubindexDoesNotExist);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_InsufficientPermission)
{
  CreateUUT_UNSIGNED8_RW(6U, 0U, 12U);

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_OP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_SAFEOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_SI0_BeforeReadCallbackDoesNotAgree)
{
  CreateUUT_UNSIGNED8_RW(6U, 0U, 12U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralIntIncompatibility));

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralIntIncompatibility);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_SI1_BeforeReadCallbackDoesNotAgree)
{
  CreateUUT_UNSIGNED8_RW(6U, 0U, 12U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralIntIncompatibility));

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralIntIncompatibility);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_SI0_BeforeReadCallbackThrows)
{
  CreateUUT_UNSIGNED8_RW(6U, 0U, 12U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_SI1_BeforeReadCallbackThrows)
{
  CreateUUT_UNSIGNED8_RW(6U, 0U, 12U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, false, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_isw_insufficientCapacity)
{
  CreateUUT_UNSIGNED32_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 7U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  writeBufferWriter.Close();
  gpcc::Stream::MemStreamWriter msw(writeBuffer, 2U, gpcc::Stream::IStreamWriter::Endian::Little);

  auto locker(spUUT->LockData());
  data_ui32[6] = 0xABCDEF00UL;

  EXPECT_THROW(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, msw), gpcc::Stream::FullError);

  msw.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_isw_full)
{
  CreateUUT_UNSIGNED32_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 7U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  writeBufferWriter.Close();
  gpcc::Stream::MemStreamWriter msw(writeBuffer, 0U, gpcc::Stream::IStreamWriter::Endian::Little);

  auto locker(spUUT->LockData());
  data_ui32[6] = 0xABCDEF00UL;

  EXPECT_THROW(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, msw), gpcc::Stream::FullError);

  msw.Close();
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Read_isw_closed)
{
  CreateUUT_UNSIGNED32_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 7U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  writeBufferWriter.Close();

  auto locker(spUUT->LockData());
  data_ui32[6] = 0xABCDEF00UL;

  EXPECT_THROW(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), gpcc::Stream::ClosedError);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_OK)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0U, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
    EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 50;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv, 50U);

  // read-back
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 50U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_min)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0U, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
    EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv, 0U);

  // read-back
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_max)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0U, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
    EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 200;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv, 200U);

  // read-back
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 200U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_InsufficientPermission)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_OP, sr), SDOAbortCode::AttemptToWriteRdOnlyObject);
  ASSERT_EQ(sr.RemainingBytes(), 1U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_StreamEmpty)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  gpcc::Stream::MemStreamReader sr(nullptr, 0, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooSmall);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_StreamClosed)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  auto sr = readBufferReader.SubStream(2U);
  sr.Close();
  EXPECT_THROW(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), gpcc::Stream::ClosedError);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_StreamNotEmpty)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooLong);

  // readback
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_BeforeWriteCbDoesNotAgree)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(uint8_t)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::GeneralError)));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::GeneralError);

  EXPECT_EQ(pv, 12U);

  // readback
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SI0_BeforeWriteCbThrows)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(uint8_t)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Throw(std::runtime_error("Test"))));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_THROW(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), std::runtime_error);

  EXPECT_EQ(pv, 12U);

  // readback
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_DeathTestsF, Write_SI0_AfterWriteCallbackThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto test = [&]()
  {
    CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

    // prepare mock
    {
      InSequence seq;
      EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, false, 0U, _)).WillOnce(Return(SDOAbortCode::OK));
      EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 1U, false)).WillOnce(Throw(std::runtime_error("Test")));
    }

    // stimulus

    auto locker(spUUT->LockData());

    readBuffer[0] = 87U;

    auto sr = readBufferReader.SubStream(1U);

    // leathal call:
    (void)spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr);
  };

  EXPECT_DEATH(test(), ".*After-write-callback threw.*");
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_NotSI0_OrderOfCallbacks)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  bool pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(bool)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, false, 0U, _)).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 1U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 1U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_TRUE(pv);
  EXPECT_TRUE(data_bool[0]);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_BOOLEAN)
{
  CreateUUT_BOOLEAN_RWRW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  bool pv[16];
  bool* pPV = pv;

  auto recorder1 = [&pv, &pPV](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pPV, pData, sizeof(bool)); pPV++; };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), _, false, 0U, _)).Times(4).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), _, false)).Times(4);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x0AU;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(3U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(5U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(7U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_FALSE(pv[0]);
  EXPECT_TRUE(pv[1]);
  EXPECT_FALSE(pv[2]);
  EXPECT_TRUE(pv[3]);

  EXPECT_FALSE(data_bool[0]);
  EXPECT_FALSE(data_bool[1]);
  EXPECT_TRUE(data_bool[2]);
  EXPECT_FALSE(data_bool[3]);
  EXPECT_FALSE(data_bool[4]);
  EXPECT_FALSE(data_bool[5]);
  EXPECT_TRUE(data_bool[6]);
  EXPECT_FALSE(data_bool[7]);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_INTEGER8)
{
  CreateUUT_INTEGER8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  int8_t pv[16];
  int8_t* pPV = pv;

  auto recorder1 = [&pv, &pPV](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pPV, pData, sizeof(int8_t)); pPV++; };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), _, false, 0U, _)).Times(4).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), _, false)).Times(4);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;
  readBuffer[1] = 66U;
  readBuffer[2] = 255U;
  readBuffer[3] = 254U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(3U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(5U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(7U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 12);
  EXPECT_EQ(pv[1], 66);
  EXPECT_EQ(pv[2], -1);
  EXPECT_EQ(pv[3], -2);

  EXPECT_EQ(data_i8[0], 12);
  EXPECT_EQ(data_i8[1], 0);
  EXPECT_EQ(data_i8[2], 66);
  EXPECT_EQ(data_i8[3], 0);
  EXPECT_EQ(data_i8[4], -1);
  EXPECT_EQ(data_i8[5], 0);
  EXPECT_EQ(data_i8[6], -2);
  EXPECT_EQ(data_i8[7], 0);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_UNSIGNED8)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[16];
  uint8_t* pPV = pv;

  auto recorder1 = [&pv, &pPV](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pPV, pData, sizeof(uint8_t)); pPV++; };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), _, false, 0U, _)).Times(4).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), _, false)).Times(4);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;
  readBuffer[1] = 66U;
  readBuffer[2] = 255U;
  readBuffer[3] = 254U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(3U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(5U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(7U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 12U);
  EXPECT_EQ(pv[1], 66U);
  EXPECT_EQ(pv[2], 255U);
  EXPECT_EQ(pv[3], 254U);

  EXPECT_EQ(data_ui8[0], 12);
  EXPECT_EQ(data_ui8[1], 0);
  EXPECT_EQ(data_ui8[2], 66);
  EXPECT_EQ(data_ui8[3], 0);
  EXPECT_EQ(data_ui8[4], 255U);
  EXPECT_EQ(data_ui8[5], 0);
  EXPECT_EQ(data_ui8[6], 254U);
  EXPECT_EQ(data_ui8[7], 0U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_UNSIGNED32)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint32_t pv[16];
  uint32_t* pPV = pv;

  auto recorder1 = [&pv, &pPV](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pPV, pData, sizeof(uint32_t)); pPV++; };

  // prepare mock
  {
    InSequence seq;

    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 2U, false, 0U, _)).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 2U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x12U;
  readBuffer[1] = 0xD2U;
  readBuffer[2] = 0xA1U;
  readBuffer[3] = 0x7BU;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(2U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0x7BA1D212UL);

  EXPECT_EQ(data_ui32[0], 0U);
  EXPECT_EQ(data_ui32[1], 0x7BA1D212UL);
  EXPECT_EQ(data_ui32[3], 0U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_BOOLEAN_NATIVE_BIT1)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[16];
  uint8_t* pPV = pv;

  auto recorder1 = [&pv, &pPV](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pPV, pData, sizeof(uint8_t)); pPV++; };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), _, false, 0U, _)).Times(4).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), _, false)).Times(4);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x0AU;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(3U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(5U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(7U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0x00U);
  EXPECT_EQ(pv[1], 0x01U);
  EXPECT_EQ(pv[2], 0x00U);
  EXPECT_EQ(pv[3], 0x01U);

  EXPECT_EQ(data_bitX[0], 0x44U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_SubindexNotExisiting)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(101U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::SubindexDoesNotExist);
  ASSERT_EQ(sr.RemainingBytes(), 1U);

  EXPECT_EQ(spUUT->Write(201U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::SubindexDoesNotExist);
  ASSERT_EQ(sr.RemainingBytes(), 1U);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_InsufficientPermission)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_SAFEOP, sr), SDOAbortCode::AttemptToWriteRdOnlyObject);
  ASSERT_EQ(sr.RemainingBytes(), 1U);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_StreamEmpty)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  gpcc::Stream::MemStreamReader sr(nullptr, 0, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooSmall);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui32[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_StreamNotEnoughData)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooSmall);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui32[i], 0U) << "Invalid value for i = " << i;
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_StreamClosed)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  auto sr = readBufferReader.SubStream(2U);
  sr.Close();
  EXPECT_THROW(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), gpcc::Stream::ClosedError);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui32[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_StreamNotEmpty)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooLong);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_BeforeWriteCbDoesNotAgree)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(uint8_t)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, false, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::GeneralError)));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::GeneralError);

  EXPECT_EQ(pv, 12U);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, Write_BeforeWriteCbThrows)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(uint8_t)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, false, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Throw(std::runtime_error("Test"))));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_THROW(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), std::runtime_error);

  EXPECT_EQ(pv, 12U);

  for (uint_fast8_t i = 0U; i <= 254U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_DeathTestsF, Write_AfterWriteCallbackThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto test = [&]()
  {
    CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

    // prepare mock
    {
      InSequence seq;
      EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, false, 0, _)).WillOnce(Return(SDOAbortCode::OK));
      EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 1U, false)).WillOnce(Throw(std::runtime_error("Test")));
    }

    // stimulus

    auto locker(spUUT->LockData());

    readBuffer[0] = 87U;

    auto sr = readBufferReader.SubStream(1U);

    // leathal call:
    (void)spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr);
  };

  EXPECT_DEATH(test(), ".*After-write-callback threw.*");
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BOOLEAN_A_SI0_16bit)
{
  // variant A: 255 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BOOLEAN_RWRW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bool();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 33U); // 7 bit bits not yet written

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    bool bitInWriteBuf = ((writeBuffer[2U + (i / 8U)] & (1U << (i % 8U))) != 0U);
    ASSERT_EQ(bitInWriteBuf, data_bool[i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BOOLEAN_B_SI0_16bit)
{
  // variant B: 64 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BOOLEAN_RWRW(64U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bool();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 10U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 64U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 64U; i++)
  {
    bool bitInWriteBuf = ((writeBuffer[2U + (i / 8U)] & (1U << (i % 8U))) != 0U);
    ASSERT_EQ(bitInWriteBuf, data_bool[i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BOOLEAN_SI0_8bit)
{
  // variant B: 64 SIs, max. 255SIs + SI0 8 bit

  CreateUUT_BOOLEAN_RWRW(64U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bool();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 9U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 64U);

  for (uint_fast16_t i = 0; i < 64; i++)
  {
    bool bitInWriteBuf = ((writeBuffer[1U + (i / 8U)] & (1U << (i % 8U))) != 0U);
    ASSERT_EQ(bitInWriteBuf, data_bool[i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BOOLEAN_ExclSI0)
{
  // variant B: 64 SIs, max. 255SIs + without SI0

  CreateUUT_BOOLEAN_RWRW(64U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bool();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 8U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 64U; i++)
  {
    bool bitInWriteBuf = ((writeBuffer[0U + (i / 8U)] & (1U << (i % 8U))) != 0U);
    ASSERT_EQ(bitInWriteBuf, data_bool[i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_INTEGER8_A_SI0_16bit)
{
  // variant A: 255 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_INTEGER8_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_i8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 257U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    ASSERT_EQ(data_i8[i], static_cast<int8_t>(writeBuffer[2U + i]));
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_INTEGER8_B_SI0_16bit)
{
  // variant B: 200 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_INTEGER8_RW(200U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_i8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 202U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 200U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 200U; i++)
  {
    ASSERT_EQ(data_i8[i], static_cast<int8_t>(writeBuffer[2U + i]));
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_INTEGER8_C_SI0_16bit)
{
  // variant C: 200 SIs, max. 200SIs + SI0 16 bit

  CreateUUT_INTEGER8_RW(200U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_i8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 202U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 200U);
  ASSERT_EQ(writeBuffer[1], 0U);

  // check read data
  for (uint_fast16_t i = 0U; i < 200U; i++)
  {
    ASSERT_EQ(data_i8[i], static_cast<int8_t>(writeBuffer[2U + i]));
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_INTEGER8_D_SI0_16bit)
{
  // variant D: 100 SIs, max. 200SIs + SI0 16 bit

  CreateUUT_INTEGER8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_i8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 102U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_i8[i], static_cast<int8_t>(writeBuffer[2U + i]));
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_INTEGER8_SI0_8bit)
{
  // variant A: 255 SIs, max. 255SIs + SI0 8 bit

  CreateUUT_INTEGER8_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_i8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 256U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    ASSERT_EQ(data_i8[i], static_cast<int8_t>(writeBuffer[1U + i]));
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_INTEGER8_ExclSI0)
{
  // variant A: 255 SIs, max. 255SIs + without SI0

  CreateUUT_INTEGER8_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_i8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 255U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    ASSERT_EQ(data_i8[i], static_cast<int8_t>(writeBuffer[0U + i]));
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED8_SI0_16bit)
{
  // 100 SIs, max. 200SIs + SI0 16 bit

  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 102U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], writeBuffer[2U + i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED8_SI0_8bit)
{
  // 100 SIs, max. 200SIs + SI0 8 bit

  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 101U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], writeBuffer[1U + i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED8_ExclSI0)
{
  // 100 SIs, max. 200SIs + without SI0

  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 100U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], writeBuffer[0U + i]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED32_SI0_16bit)
{
  // 100 SIs, max. 200SIs + SI0 16 bit

  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui32();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 402U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ((data_ui32[i] >> 0U ) & 0xFFU, writeBuffer[2U + (i * 4U) + 0U]);
    ASSERT_EQ((data_ui32[i] >> 8U ) & 0xFFU, writeBuffer[2U + (i * 4U) + 1U]);
    ASSERT_EQ((data_ui32[i] >> 16U) & 0xFFU, writeBuffer[2U + (i * 4U) + 2U]);
    ASSERT_EQ((data_ui32[i] >> 24U) & 0xFFU, writeBuffer[2U + (i * 4U) + 3U]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED32_SI0_8bit)
{
  // 100 SIs, max. 200SIs + SI0 8 bit

  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui32();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 401U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ((data_ui32[i] >> 0U ) & 0xFFU, writeBuffer[1U + (i * 4U) + 0U]);
    ASSERT_EQ((data_ui32[i] >> 8U ) & 0xFFU, writeBuffer[1U + (i * 4U) + 1U]);
    ASSERT_EQ((data_ui32[i] >> 16U) & 0xFFU, writeBuffer[1U + (i * 4U) + 2U]);
    ASSERT_EQ((data_ui32[i] >> 24U) & 0xFFU, writeBuffer[1U + (i * 4U) + 3U]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED32_ExclSI0)
{
  // 100 SIs, max. 200SIs + without SI0

  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui32();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 400U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ((data_ui32[i] >> 0U ) & 0xFFU, writeBuffer[0U + (i * 4U) + 0U]);
    ASSERT_EQ((data_ui32[i] >> 8U ) & 0xFFU, writeBuffer[0U + (i * 4U) + 1U]);
    ASSERT_EQ((data_ui32[i] >> 16U) & 0xFFU, writeBuffer[0U + (i * 4U) + 2U]);
    ASSERT_EQ((data_ui32[i] >> 24U) & 0xFFU, writeBuffer[0U + (i * 4U) + 3U]);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_A_SI0_16bit)
{
  // variant A: 255 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BIT1_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 33U); // 7 bits of last byte are not yet written

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[2U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_B_SI0_16bit)
{
  // variant B: 64 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BIT1_RW(64U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 10U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 64U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 64U; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[2U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_C_SI0_16bit)
{
  // variant C: 64 SIs, max. 64SIs + SI0 16 bit

  CreateUUT_BIT1_RW(64U, 0U, 64U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 10U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 64U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 64U; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[2U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_D_SI0_16bit)
{
  // variant D: 56 SIs, max. 64SIs + SI0 16bit

  CreateUUT_BIT1_RW(56U, 0U, 64U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 9U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 56U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 56; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[2U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_D_SI0_8bit)
{
  // variant D: 56 SIs, max. 64SIs + SI0 8 bit

  CreateUUT_BIT1_RW(56U, 0U, 64U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 8U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 56U);

  for (uint_fast16_t i = 0U; i < 56U; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[1U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_D_ExclSI0)
{
  // variant D: 56 SIs, max. 64SIs + without SI0

  CreateUUT_BIT1_RW(56U, 0U, 64U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 7U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 56U; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[0U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT6_A_SI0_16bit)
{
  // variant A: 255 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BIT6_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 193U); // 2 bits of last byte are not yet written

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    uint_fast16_t byteOffset = (i * 6U) / 8U;
    uint_fast8_t bitOffset  = (i * 6U) % 8U;

    ASSERT_TRUE(byteOffset <= sizeof(data_bitX) - 2U) << "Check unit test code";

    uint8_t original = ((data_bitX[byteOffset] | (static_cast<uint16_t>(data_bitX[byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    uint8_t read     = ((writeBuffer[2U + byteOffset] | (static_cast<uint16_t>(writeBuffer[2U + byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT6_B_SI0_16bit)
{
  // variant B: 64 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BIT6_RW(64U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 50U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 64U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 64U; i++)
  {
    uint_fast16_t byteOffset = (i * 6U) / 8U;
    uint_fast8_t bitOffset  = (i * 6U) % 8U;

    ASSERT_TRUE(byteOffset <= sizeof(data_bitX) - 2U) << "Check unit test code";

    uint8_t original = ((data_bitX[byteOffset] | (static_cast<uint16_t>(data_bitX[byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    uint8_t read     = ((writeBuffer[2U + byteOffset] | (static_cast<uint16_t>(writeBuffer[2U + byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT6_C_SI0_8bit)
{
  // variant C: 32 SIs, max. 100SIs + SI0 8 bit

  CreateUUT_BIT6_RW(32U, 0U, 100U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 25U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 32U);

  for (uint_fast16_t i = 0U; i < 32U; i++)
  {
    uint_fast16_t byteOffset = (i * 6U) / 8U;
    uint_fast8_t bitOffset  = (i * 6U) % 8U;

    ASSERT_TRUE(byteOffset <= sizeof(data_bitX) - 2U) << "Check unit test code";

    uint8_t original = ((data_bitX[byteOffset] | (static_cast<uint16_t>(data_bitX[byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    uint8_t read     = ((writeBuffer[1U + byteOffset] | (static_cast<uint16_t>(writeBuffer[1U + byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT6_C_ExclSI0)
{
  // variant C: 32 SIs, max. 100SIs + withoug SI0

  CreateUUT_BIT6_RW(32U, 0U, 100U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 24U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 32U; i++)
  {
    uint_fast16_t byteOffset = (i * 6U) / 8U;
    uint_fast8_t bitOffset  = (i * 6U) % 8U;

    ASSERT_TRUE(byteOffset <= sizeof(data_bitX) - 2U) << "Check unit test code";

    uint8_t original = ((data_bitX[byteOffset] | (static_cast<uint16_t>(data_bitX[byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    uint8_t read     = ((writeBuffer[0U + byteOffset] | (static_cast<uint16_t>(writeBuffer[0U + byteOffset + 1U]) << 8U)) >> bitOffset) & 0x3FU;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT8_A_SI0_16bit)
{
  // variant A: 255 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BIT8_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 257U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    ASSERT_EQ(data_bitX[i], writeBuffer[2U + i]) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT8_B_SI0_16bit)
{
  // variant B: 100 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BIT8_RW(100U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 102U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_bitX[i], writeBuffer[2U + i]) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT8_B_SI0_8bit)
{
  // variant B: 100 SIs, max. 255SIs + SI0 8 bit

  CreateUUT_BIT8_RW(100U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 101U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_bitX[i], writeBuffer[1U + i]) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT8_B_ExclSI0)
{
  // variant A: 100 SIs, max. 255SIs + without SI0

  CreateUUT_BIT8_RW(100U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 100U);

  writeBufferWriter.Close();

  // check read data
  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_bitX[i], writeBuffer[0U + i]) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_UNSIGNED8_dataPureWO)
{
  CreateUUT_UNSIGNED8_RWWO(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 102U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 100U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(writeBuffer[2U + i], 0) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BIT1_dataPureWO)
{
  CreateUUT_BIT1_RWWO(16, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_ui8();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 4U);

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 16U);
  ASSERT_EQ(writeBuffer[1], 0U);

  EXPECT_EQ(writeBuffer[2U], 0);
  EXPECT_EQ(writeBuffer[3U], 0);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BOOLEAN_NATIVE_BIT1)
{
  // 255 SIs, max. 255SIs + SI0 16 bit

  CreateUUT_BOOLEAN_NATIVE_BIT1_RW(255U, 0U, 255U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  Randomize_data_bitX();

  // stimulus
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  // check number of bytes written to stream
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 33U); // 7 bits of last byte are not yet written

  writeBufferWriter.Close();

  // check read data
  ASSERT_EQ(writeBuffer[0], 255U);
  ASSERT_EQ(writeBuffer[1], 0U);

  for (uint_fast16_t i = 0U; i < 255U; i++)
  {
    uint_fast8_t byteOffset = i / 8U;
    uint_fast8_t bitOffset  = i % 8U;

    uint8_t original = (data_bitX[byteOffset] >> bitOffset) & 0x1U;
    uint8_t read     = (writeBuffer[2U + byteOffset] >> bitOffset) & 0x1U;
    ASSERT_EQ(original, read) << "Error at i = " << static_cast<uint32_t>(i);
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_InsufficientPermission)
{
  // 100 SIs, max. 200SIs + SI0 16 bit

  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_OP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);
  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_SAFEOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BeforeReadCallbackDoesNotAgree)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralIntIncompatibility));

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralIntIncompatibility);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_BeforeReadCallbackThrows)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Read_isw_insufficientCapacity)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(5).WillRepeatedly(Return(SDOAbortCode::OK));

  writeBufferWriter.Close();

  for (uint_fast8_t i = 0; i <= 4; i++)
  {
    gpcc::Stream::MemStreamWriter msw(writeBuffer, i, gpcc::Stream::IStreamWriter::Endian::Little);

    auto locker(spUUT->LockData());

    EXPECT_THROW(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, msw), gpcc::Stream::FullError);

    msw.Close();
  }
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0_16bit_modified)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[10];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 10U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(12);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 10U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0_8bit_modified)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[10];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 10U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;

  auto sr = readBufferReader.SubStream(11);
  EXPECT_EQ(spUUT->CompleteWrite(true, false, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 1U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], readBuffer[i + 1U]) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 10U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_ExclSI0)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[100];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, true, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 1U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();

  auto sr = readBufferReader.SubStream(100);
  EXPECT_EQ(spUUT->CompleteWrite(false, false, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], readBuffer[i]) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0_16bit_not_modified)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[100];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 100U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 100U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(102);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0_noPerm_not_modified)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 100U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(102);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_OP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::AttemptToWriteRdOnlyObject);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_Data_noPerm)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 100U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(102);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_SAFEOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::AttemptToWriteRdOnlyObject);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0RO_16bit_not_modified)
{
  CreateUUT_UNSIGNED8_RORW(100U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[100];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 100U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 100U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(102);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0RO_16bit_modified)
{
  CreateUUT_UNSIGNED8_RORW(100U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 99U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(101);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::UnsupportedAccessToObject);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0_MinViolation)
{
  CreateUUT_UNSIGNED8_RW(100U, 50U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 49U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(51);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::ValueTooLow);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_SI0_MaxViolation)
{
  CreateUUT_UNSIGNED8_RW(100U, 50U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 201U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(203);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::ValueTooHigh);

  // check data
  for (uint_fast8_t i = 0U; i < 100U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_dataPureRO)
{
  CreateUUT_UNSIGNED8_RWRO(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[90];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 90U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 90U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(92);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 90U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 90U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BIT1_dataPureRO)
{
  CreateUUT_BIT1_RWRO(16U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[3];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 24U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 24U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(5);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  for (uint_fast8_t i = 0U; i < 3U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 24U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED32_SI0_16bit_modified)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint32_t pv[2];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 2U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 2U;
  readBuffer[1] = 0U;
  readBuffer[2] = 0x12U;
  readBuffer[3] = 0x34U;
  readBuffer[4] = 0x56U;
  readBuffer[5] = 0x78U;
  readBuffer[6] = 0x9AU;
  readBuffer[7] = 0xBCU;
  readBuffer[8] = 0xDEU;
  readBuffer[9] = 0xF1U;

  auto sr = readBufferReader.SubStream(10);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(((pv[0] >>  0U) & 0xFFU), readBuffer[2]);
  EXPECT_EQ(((pv[0] >>  8U) & 0xFFU), readBuffer[3]);
  EXPECT_EQ(((pv[0] >> 16U) & 0xFFU), readBuffer[4]);
  EXPECT_EQ(((pv[0] >> 24U) & 0xFFU), readBuffer[5]);

  EXPECT_EQ(((pv[1] >>  0U) & 0xFFU), readBuffer[6]);
  EXPECT_EQ(((pv[1] >>  8U) & 0xFFU), readBuffer[7]);
  EXPECT_EQ(((pv[1] >> 16U) & 0xFFU), readBuffer[8]);
  EXPECT_EQ(((pv[1] >> 24U) & 0xFFU), readBuffer[9]);

  EXPECT_EQ(((data_ui32[0] >>  0U) & 0xFFU), readBuffer[2]);
  EXPECT_EQ(((data_ui32[0] >>  8U) & 0xFFU), readBuffer[3]);
  EXPECT_EQ(((data_ui32[0] >> 16U) & 0xFFU), readBuffer[4]);
  EXPECT_EQ(((data_ui32[0] >> 24U) & 0xFFU), readBuffer[5]);

  EXPECT_EQ(((data_ui32[1] >>  0U) & 0xFFU), readBuffer[6]);
  EXPECT_EQ(((data_ui32[1] >>  8U) & 0xFFU), readBuffer[7]);
  EXPECT_EQ(((data_ui32[1] >> 16U) & 0xFFU), readBuffer[8]);
  EXPECT_EQ(((data_ui32[1] >> 24U) & 0xFFU), readBuffer[9]);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 2U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED32_SI0_8bit_modified)
{
  CreateUUT_UNSIGNED32_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint32_t pv[2];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 2U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 2U;
  readBuffer[1] = 0x12U;
  readBuffer[2] = 0x34U;
  readBuffer[3] = 0x56U;
  readBuffer[4] = 0x78U;
  readBuffer[5] = 0x9AU;
  readBuffer[6] = 0xBCU;
  readBuffer[7] = 0xDEU;
  readBuffer[8] = 0xF1U;

  auto sr = readBufferReader.SubStream(9);
  EXPECT_EQ(spUUT->CompleteWrite(true, false, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(((pv[0] >>  0U) & 0xFFU), readBuffer[1]);
  EXPECT_EQ(((pv[0] >>  8U) & 0xFFU), readBuffer[2]);
  EXPECT_EQ(((pv[0] >> 16U) & 0xFFU), readBuffer[3]);
  EXPECT_EQ(((pv[0] >> 24U) & 0xFFU), readBuffer[4]);

  EXPECT_EQ(((pv[1] >>  0U) & 0xFFU), readBuffer[5]);
  EXPECT_EQ(((pv[1] >>  8U) & 0xFFU), readBuffer[6]);
  EXPECT_EQ(((pv[1] >> 16U) & 0xFFU), readBuffer[7]);
  EXPECT_EQ(((pv[1] >> 24U) & 0xFFU), readBuffer[8]);

  EXPECT_EQ(((data_ui32[0] >>  0U) & 0xFFU), readBuffer[1]);
  EXPECT_EQ(((data_ui32[0] >>  8U) & 0xFFU), readBuffer[2]);
  EXPECT_EQ(((data_ui32[0] >> 16U) & 0xFFU), readBuffer[3]);
  EXPECT_EQ(((data_ui32[0] >> 24U) & 0xFFU), readBuffer[4]);

  EXPECT_EQ(((data_ui32[1] >>  0U) & 0xFFU), readBuffer[5]);
  EXPECT_EQ(((data_ui32[1] >>  8U) & 0xFFU), readBuffer[6]);
  EXPECT_EQ(((data_ui32[1] >> 16U) & 0xFFU), readBuffer[7]);
  EXPECT_EQ(((data_ui32[1] >> 24U) & 0xFFU), readBuffer[8]);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 2U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED32_ExclSI0)
{
  CreateUUT_UNSIGNED32_RW(2U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint32_t pv[2];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, true, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 1U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 0x12U;
  readBuffer[1] = 0x34U;
  readBuffer[2] = 0x56U;
  readBuffer[3] = 0x78U;
  readBuffer[4] = 0x9AU;
  readBuffer[5] = 0xBCU;
  readBuffer[6] = 0xDEU;
  readBuffer[7] = 0xF1U;

  auto sr = readBufferReader.SubStream(8);
  EXPECT_EQ(spUUT->CompleteWrite(false, false, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(((pv[0] >>  0U) & 0xFFU), readBuffer[0]);
  EXPECT_EQ(((pv[0] >>  8U) & 0xFFU), readBuffer[1]);
  EXPECT_EQ(((pv[0] >> 16U) & 0xFFU), readBuffer[2]);
  EXPECT_EQ(((pv[0] >> 24U) & 0xFFU), readBuffer[3]);

  EXPECT_EQ(((pv[1] >>  0U) & 0xFFU), readBuffer[4]);
  EXPECT_EQ(((pv[1] >>  8U) & 0xFFU), readBuffer[5]);
  EXPECT_EQ(((pv[1] >> 16U) & 0xFFU), readBuffer[6]);
  EXPECT_EQ(((pv[1] >> 24U) & 0xFFU), readBuffer[7]);

  EXPECT_EQ(((data_ui32[0] >>  0U) & 0xFFU), readBuffer[0]);
  EXPECT_EQ(((data_ui32[0] >>  8U) & 0xFFU), readBuffer[1]);
  EXPECT_EQ(((data_ui32[0] >> 16U) & 0xFFU), readBuffer[2]);
  EXPECT_EQ(((data_ui32[0] >> 24U) & 0xFFU), readBuffer[3]);

  EXPECT_EQ(((data_ui32[1] >>  0U) & 0xFFU), readBuffer[4]);
  EXPECT_EQ(((data_ui32[1] >>  8U) & 0xFFU), readBuffer[5]);
  EXPECT_EQ(((data_ui32[1] >> 16U) & 0xFFU), readBuffer[6]);
  EXPECT_EQ(((data_ui32[1] >> 24U) & 0xFFU), readBuffer[7]);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 2U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BIT1_SI0_16bit_modified)
{
  CreateUUT_BIT1_RW(12U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[2];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 12U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;
  readBuffer[1] = 0U;
  readBuffer[2] = 0x69U;
  readBuffer[3] = 0xFFU;

  auto sr = readBufferReader.SubStream(4);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::four), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(data_bitX[0], 0x69U);
  EXPECT_EQ(data_bitX[1], 0x0FU);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 12U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BIT6_SI0_16bit_modified)
{
  CreateUUT_BIT6_RW(3U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[3];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 3U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 3U;
  readBuffer[1] = 0U;
  readBuffer[2] = 0x69U;
  readBuffer[3] = 0x55U;
  readBuffer[4] = 0xFFU;

  auto sr = readBufferReader.SubStream(5);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::six), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(data_bitX[0], 0x69U);
  EXPECT_EQ(data_bitX[1], 0x55U);
  EXPECT_EQ(data_bitX[2], 0x03U);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 3U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BIT8_SI0_16bit_modified)
{
  CreateUUT_BIT8_RW(3U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[3];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 3U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 3U;
  readBuffer[1] = 0U;
  readBuffer[2] = 0x69U;
  readBuffer[3] = 0x55U;
  readBuffer[4] = 0xFFU;

  auto sr = readBufferReader.SubStream(5);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(data_bitX[0], 0x69U);
  EXPECT_EQ(data_bitX[1], 0x55U);
  EXPECT_EQ(data_bitX[2], 0xFFU);
  EXPECT_EQ(data_bitX[3], 0x00U);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 3U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BOOLEAN_NATIVE_BIT1_SI0_16bit_modified)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW(0U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[2];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 12U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 12U;
  readBuffer[1] = 0U;
  readBuffer[2] = 0x69U;
  readBuffer[3] = 0xFFU;

  auto sr = readBufferReader.SubStream(4);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::four), SDOAbortCode::OK);

  // check data
  EXPECT_EQ(data_bitX[0], 0x69U);
  EXPECT_EQ(data_bitX[1], 0x0FU);

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 12U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_isr_notEnoughData)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(11);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::DataTypeMismatchTooSmall);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_isr_closed)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(11);
  sr.Close();
  EXPECT_THROW(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), gpcc::Stream::ClosedError);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_UNSIGNED8_ernobMismatch)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(12);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::one), SDOAbortCode::DataTypeMismatchTooLong);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BeforeWriteCallbackDoesNotAgree)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[10];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 10U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::GeneralError)));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(12);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::GeneralError);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_TestsF, CA_Write_BeforeWriteCallbackThrows)
{
  CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[10];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 10U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Throw(std::runtime_error("Test"))));
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  // stimulus

  auto locker(spUUT->LockData());

  Randomize_readbuffer();
  readBuffer[0] = 10U;
  readBuffer[1] = 0U;

  auto sr = readBufferReader.SubStream(12);
  EXPECT_THROW(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero), std::runtime_error);

  // check data
  for (uint_fast8_t i = 0U; i < 10U; i++)
  {
    ASSERT_EQ(pv[i], readBuffer[i + 2U]) << "Error at i = " << static_cast<uint32_t>(i);
    ASSERT_EQ(data_ui8[i], 0U) << "Error at i = " << static_cast<uint32_t>(i);
  }

  // read and check SI0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  writeBufferWriter.Close();
  EXPECT_EQ(writeBuffer[0], 100U);
}

TEST_F(gpcc_cood_ObjectARRAY_wicb_DeathTestsF, CA_Write_AfterWriteCallbackThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto test = [&]()
  {
    CreateUUT_UNSIGNED8_RW(100U, 0U, 200U);

    // prepare mock
    {
      InSequence seq;
      EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 10U, _)).WillOnce(Return(SDOAbortCode::OK));
      EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).WillOnce(Throw(std::runtime_error("Test")));
    }

    // stimulus

    auto locker(spUUT->LockData());

    Randomize_readbuffer();
    readBuffer[0] = 10U;
    readBuffer[1] = 0U;

    auto sr = readBufferReader.SubStream(12U);

    // leathal call:
    (void)spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, sr, gpcc::Stream::IStreamReader::RemainingNbOfBits::zero);
  };

  EXPECT_DEATH(test(), ".*After-write-callback threw.*");
}

} // namespace gpcc_tests
} // namespace cood
