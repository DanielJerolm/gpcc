/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include "gpcc/src/cood/ObjectVAR_wicb.hpp"
#include "gpcc/src/cood/exceptions.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include "IObjectNotifiableMock.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <cstring>

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;
using namespace gpcc::Stream;

using namespace testing;

/// Test fixture for gpcc::cood::ObjectVAR_wicb related tests.
class gpcc_cood_ObjectVAR_wicb_TestsF: public Test
{
  public:
    gpcc_cood_ObjectVAR_wicb_TestsF(void);

  protected:
    virtual ~gpcc_cood_ObjectVAR_wicb_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

    void CreateUUT_BOOLEAN_RW(void);
    void CreateUUT_INTEGER8_RW(void);
    void CreateUUT_INTEGER16_RW(void);
    void CreateUUT_INTEGER32_RW(void);
    void CreateUUT_UNSIGNED8_RW(void);
    void CreateUUT_UNSIGNED8_RO_Preop(void);
    void CreateUUT_UNSIGNED16_RW(void);
    void CreateUUT_UNSIGNED32_RW(void);
    void CreateUUT_REAL32_RW(void);
    void CreateUUT_VISIBLE_STRING_RW(void);
    void CreateUUT_OCTET_STRING_RW(void);
    void CreateUUT_UNICODE_STRING_RW(void);
    void CreateUUT_REAL64_RW(void);
    void CreateUUT_INTEGER64_RW(void);
    void CreateUUT_UNSIGNED64_RW(void);
    void CreateUUT_BIT1_RW(void);
    void CreateUUT_BIT2_RW(void);
    void CreateUUT_BIT3_RW(void);
    void CreateUUT_BIT4_RW(void);
    void CreateUUT_BIT5_RW(void);
    void CreateUUT_BIT6_RW(void);
    void CreateUUT_BIT7_RW(void);
    void CreateUUT_BIT8_RW(void);

    void CreateUUT_BOOLEAN_NATIVE_BIT1_RW(void);

    // Mutex protecting the data
    gpcc::osal::Mutex mutex;

    // The data
    bool data_bool;
    int8_t data_i8;
    int16_t data_i16;
    int32_t data_i32;
    uint8_t data_ui8;
    uint16_t data_ui16;
    uint32_t data_ui32;
    float data_f;
    char data_visible_string[8];
    uint8_t data_octet_string[4];
    uint16_t data_unicode_string[8];
    double data_d;
    int64_t data_i64;
    uint64_t data_ui64;
    uint8_t data_bitX;

    // Mock for reception of callbacks
    StrictMock<IObjectNotifiableMock> cbm;

    // Buffers for use with MemStreamReader and MemStreamWriter
    uint8_t readBuffer[64];
    uint8_t writeBuffer[64];

    // Stream reader/writer for the buffers above
    MemStreamReader readBufferReader;
    MemStreamWriter writeBufferWriter;

    // ...and finally the UUT
    std::unique_ptr<ObjectVAR_wicb> spUUT;
};

gpcc_cood_ObjectVAR_wicb_TestsF::gpcc_cood_ObjectVAR_wicb_TestsF(void)
: Test()
, mutex()
, data_bool(false)
, data_i8(0)
, data_i16(0)
, data_i32(0)
, data_ui8(0)
, data_ui16(0)
, data_ui32(0)
, data_f(0)
, data_visible_string()
, data_octet_string()
, data_unicode_string()
, data_d(0)
, data_i64(0)
, data_ui64(0)
, data_bitX(0)
, cbm()
, readBuffer()
, writeBuffer()
, readBufferReader(&readBuffer, sizeof(readBuffer), MemStreamReader::Endian::Little)
, writeBufferWriter(&writeBuffer, sizeof(writeBuffer), MemStreamWriter::Endian::Little)
, spUUT()
{
}

gpcc_cood_ObjectVAR_wicb_TestsF::~gpcc_cood_ObjectVAR_wicb_TestsF(void)
{
}

void gpcc_cood_ObjectVAR_wicb_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectVAR_wicb_TestsF::TearDown(void)
{
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BOOLEAN_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::boolean,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bool,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_INTEGER8_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::integer8,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_i8,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_INTEGER16_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::integer16,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_i16,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_INTEGER32_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::integer32,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_i32,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_UNSIGNED8_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned8,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_ui8,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_UNSIGNED8_RO_Preop(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned8,
                            1U,
                            Object::attr_ACCESS_RD_PREOP,
                            &data_ui8,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_UNSIGNED16_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned16,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_ui16,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_UNSIGNED32_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned32,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_ui32,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_REAL32_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::real32,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_f,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_VISIBLE_STRING_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::visible_string,
                            sizeof(data_visible_string),
                            Object::attr_ACCESS_RW,
                            &data_visible_string,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_OCTET_STRING_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::octet_string,
                            sizeof(data_octet_string),
                            Object::attr_ACCESS_RW,
                            &data_octet_string,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_UNICODE_STRING_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unicode_string,
                            sizeof(data_unicode_string) / sizeof(uint16_t),
                            Object::attr_ACCESS_RW,
                            &data_unicode_string,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_REAL64_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::real64,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_d,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_INTEGER64_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::integer64,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_i64,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_UNSIGNED64_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned64,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_ui64,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT1_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit1,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT2_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit2,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT3_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit3,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT4_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit4,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT5_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit5,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT6_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit6,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT7_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit7,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BIT8_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::bit8,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

void gpcc_cood_ObjectVAR_wicb_TestsF::CreateUUT_BOOLEAN_NATIVE_BIT1_RW(void)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::boolean_native_bit1,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_bitX,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

typedef gpcc_cood_ObjectVAR_wicb_TestsF gpcc_cood_ObjectVAR_wicb_DeathTestsF;

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, InstantiateAndDestroy)
{
  CreateUUT_UNSIGNED8_RW();
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Constructor_InvalidArgs)
{
  // unsupported data type
  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::integer24,
                                         1U,
                                         Object::attr_ACCESS_RW,
                                         &data_i32,
                                         &mutex,
                                         nullptr,
                                         nullptr,
                                         nullptr)), DataTypeNotSupportedError);

  // unsupported number of elements
  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::integer32,
                                         0U,
                                         Object::attr_ACCESS_RW,
                                         &data_i32,
                                         &mutex,
                                         nullptr,
                                         nullptr,
                                         nullptr)), std::invalid_argument);

  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::integer32,
                                         2U,
                                         Object::attr_ACCESS_RW,
                                         &data_i32,
                                         &mutex,
                                         nullptr,
                                         nullptr,
                                         nullptr)), std::invalid_argument);

  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::visible_string,
                                         0U,
                                         Object::attr_ACCESS_RW,
                                         &data_visible_string,
                                         &mutex,
                                         nullptr,
                                         nullptr,
                                         nullptr)), std::invalid_argument);

  // no R/W-permission specified
  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::integer32,
                                         1U,
                                         Object::attr_BACKUP,
                                         &data_i32,
                                         &mutex,
                                         nullptr,
                                         nullptr,
                                         nullptr)), std::invalid_argument);

  // no mutex, but write access possible
  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::integer32,
                                         1U,
                                         Object::attr_ACCESS_RW,
                                         &data_i32,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr)), std::logic_error);

  // pointer to data is nullptr
  EXPECT_THROW(spUUT.reset(new ObjectVAR_wicb("ObjName",
                                         DataType::integer32,
                                         1U,
                                         Object::attr_ACCESS_RW,
                                         nullptr,
                                         &mutex,
                                         nullptr,
                                         nullptr,
                                         nullptr)), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CheckLockData)
{
  CreateUUT_UNSIGNED8_RW();

  auto locker(spUUT->LockData());

  if (mutex.TryLock())
  {
    ADD_FAILURE() << "Mutex protecting the data has not been locked by ObjectVAR_wicb::LockData()";
    mutex.Unlock();
  }
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CheckMetaData_withoutLock)
{
  CreateUUT_UNSIGNED8_RW();

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Variable);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::unsigned8);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     1U);
  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),           "ObjName");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CheckMetaData_withLock)
{
  CreateUUT_UNSIGNED8_RW();

  auto locker(spUUT->LockData());

  // methods which do not require the lock:
  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Variable);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::unsigned8);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     1U);
  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),           "ObjName");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  // methods which REQUIRE the lock:
  EXPECT_EQ(spUUT->GetObjectStreamSize(false), 8U);
  EXPECT_EQ(spUUT->GetObjectStreamSize(true),  8U);

  EXPECT_EQ(spUUT->GetNbOfSubIndices(),        1U);
  EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CheckMetaData_VISIBLE_STRING_withLock)
{
  CreateUUT_VISIBLE_STRING_RW();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // methods which do not require the lock:
  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Variable);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::visible_string);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     1U);
  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::visible_string);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        sizeof(data_visible_string) * 8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),           "ObjName");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  // methods which REQUIRE the lock:
  EXPECT_EQ(spUUT->GetObjectStreamSize(false), (sizeof(data_visible_string) * 8U));
  EXPECT_EQ(spUUT->GetObjectStreamSize(true),  (sizeof(data_visible_string) * 8U));

  EXPECT_EQ(spUUT->GetNbOfSubIndices(),        1U);
  EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CheckMetaData_DataTypeMapped_withLock)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW();

  auto locker(spUUT->LockData());

  // methods which do not require the lock:
  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Variable);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::boolean);
  EXPECT_EQ(spUUT->GetObjectName(),            "ObjName");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     1U);
  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::boolean);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        1U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),           "ObjName");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  // methods which REQUIRE the lock:
  EXPECT_EQ(spUUT->GetObjectStreamSize(false), 1U);
  EXPECT_EQ(spUUT->GetObjectStreamSize(true),  1U);

  EXPECT_EQ(spUUT->GetNbOfSubIndices(),        1U);
  EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     1U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CheckMetaData_InvalidSubindex)
{
  CreateUUT_UNSIGNED8_RW();

  auto locker(spUUT->LockData());

  // methods which do not require the lock:
  EXPECT_THROW((void)spUUT->IsSubIndexEmpty(1U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxDataType(1U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxAttributes(1U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxMaxSize(1U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxName(1U), SubindexNotExistingError);

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(1U), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(1U), std::logic_error);

  // methods which REQUIRE the lock:
  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(1U), SubindexNotExistingError);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, GetSubIdxActualSize_BeforeReadCbReportsOutOfMemory)
{
  CreateUUT_VISIBLE_STRING_RW();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::OutOfMemory));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(0), std::bad_alloc);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, GetSubIdxActualSize_BeforeReadCbReportsError)
{
  CreateUUT_VISIBLE_STRING_RW();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralIntIncompatibility));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(0), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, GetSubIdxActualSize_BeforeReadCbThrows)
{
  CreateUUT_VISIBLE_STRING_RW();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, true)).Times(1).WillRepeatedly(Throw(std::runtime_error("Intentionally thrown exception")));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(0), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BOOLEAN)
{
  CreateUUT_BOOLEAN_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bool = false;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bool = true;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x02U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_INTEGER8)
{
  CreateUUT_INTEGER8_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_i8 = 0x5B;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_INTEGER16)
{
  CreateUUT_INTEGER16_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_i16 = 0x5BE2;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0xE2U);
  EXPECT_EQ(writeBuffer[1], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_INTEGER32)
{
  CreateUUT_INTEGER32_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_i32 = 0x5BF61299L;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x99U);
  EXPECT_EQ(writeBuffer[1], 0x12U);
  EXPECT_EQ(writeBuffer[2], 0xF6U);
  EXPECT_EQ(writeBuffer[3], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_UNSIGNED8)
{
  CreateUUT_UNSIGNED8_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_ui8 = 0x5BU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_UNSIGNED16)
{
  CreateUUT_UNSIGNED16_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_ui16 = 0x5BE2U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0xE2U);
  EXPECT_EQ(writeBuffer[1], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_UNSIGNED32)
{
  CreateUUT_UNSIGNED32_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_ui32 = 0x5BF61299UL;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x99U);
  EXPECT_EQ(writeBuffer[1], 0x12U);
  EXPECT_EQ(writeBuffer[2], 0xF6U);
  EXPECT_EQ(writeBuffer[3], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_REAL32)
{
  CreateUUT_REAL32_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_f = 15.78;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Write_float(data_f);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], writeBuffer[4]);
  EXPECT_EQ(writeBuffer[1], writeBuffer[5]);
  EXPECT_EQ(writeBuffer[2], writeBuffer[6]);
  EXPECT_EQ(writeBuffer[3], writeBuffer[7]);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_VISIBLE_STRING_zero)
{
  CreateUUT_VISIBLE_STRING_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_visible_string[0] = 0;
  data_visible_string[1] = 0;
  data_visible_string[2] = 0;
  data_visible_string[3] = 0;
  data_visible_string[4] = 0;
  data_visible_string[5] = 0;
  data_visible_string[6] = 0;
  data_visible_string[7] = 0;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  ASSERT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 1U);
  writeBufferWriter.Close();

  EXPECT_EQ(static_cast<char>(writeBuffer[0]), 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_VISIBLE_STRING_half)
{
  CreateUUT_VISIBLE_STRING_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_visible_string[0] = 'T';
  data_visible_string[1] = 'e';
  data_visible_string[2] = 's';
  data_visible_string[3] = 't';
  data_visible_string[4] = 0;
  data_visible_string[5] = 0;
  data_visible_string[6] = 0;
  data_visible_string[7] = 0;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  ASSERT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 5U);
  writeBufferWriter.Close();

  EXPECT_EQ(static_cast<char>(writeBuffer[0]), 'T');
  EXPECT_EQ(static_cast<char>(writeBuffer[1]), 'e');
  EXPECT_EQ(static_cast<char>(writeBuffer[2]), 's');
  EXPECT_EQ(static_cast<char>(writeBuffer[3]), 't');
  EXPECT_EQ(static_cast<char>(writeBuffer[4]), 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_VISIBLE_STRING_full)
{
  CreateUUT_VISIBLE_STRING_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_visible_string[0] = 'T';
  data_visible_string[1] = 'e';
  data_visible_string[2] = 's';
  data_visible_string[3] = 't';
  data_visible_string[4] = 'f';
  data_visible_string[5] = 'u';
  data_visible_string[6] = 'l';
  data_visible_string[7] = 'l';

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  ASSERT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 8U);
  writeBufferWriter.Close();

  EXPECT_EQ(static_cast<char>(writeBuffer[0]), 'T');
  EXPECT_EQ(static_cast<char>(writeBuffer[1]), 'e');
  EXPECT_EQ(static_cast<char>(writeBuffer[2]), 's');
  EXPECT_EQ(static_cast<char>(writeBuffer[3]), 't');
  EXPECT_EQ(static_cast<char>(writeBuffer[4]), 'f');
  EXPECT_EQ(static_cast<char>(writeBuffer[5]), 'u');
  EXPECT_EQ(static_cast<char>(writeBuffer[6]), 'l');
  EXPECT_EQ(static_cast<char>(writeBuffer[7]), 'l');
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_OCTET_STRING)
{
  CreateUUT_OCTET_STRING_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_octet_string[0] = 0xABU;
  data_octet_string[1] = 0xCDU;
  data_octet_string[2] = 0xEFU;
  data_octet_string[3] = 0x12U;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  ASSERT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 4U);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0xABU);
  EXPECT_EQ(writeBuffer[1], 0xCDU);
  EXPECT_EQ(writeBuffer[2], 0xEFU);
  EXPECT_EQ(writeBuffer[3], 0x12U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_UNICODE_STRING)
{
  CreateUUT_UNICODE_STRING_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_unicode_string[0] = 0x1658U;
  data_unicode_string[1] = 0x8B3AU;
  data_unicode_string[2] = 0x1523U;
  data_unicode_string[3] = 0x9882U;
  data_unicode_string[4] = 0xCD62U;
  data_unicode_string[5] = 0x8E22U;
  data_unicode_string[6] = 0x1009U;
  data_unicode_string[7] = 0xD7FFU;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  ASSERT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer) - 16U);
  writeBufferWriter.Close();

  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[ 0]) | (static_cast<uint16_t>(writeBuffer[ 1]) << 8U)), 0x1658U);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[ 2]) | (static_cast<uint16_t>(writeBuffer[ 3]) << 8U)), 0x8B3AU);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[ 4]) | (static_cast<uint16_t>(writeBuffer[ 5]) << 8U)), 0x1523U);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[ 6]) | (static_cast<uint16_t>(writeBuffer[ 7]) << 8U)), 0x9882U);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[ 8]) | (static_cast<uint16_t>(writeBuffer[ 9]) << 8U)), 0xCD62U);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[10]) | (static_cast<uint16_t>(writeBuffer[11]) << 8U)), 0x8E22U);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[12]) | (static_cast<uint16_t>(writeBuffer[13]) << 8U)), 0x1009U);
  EXPECT_EQ(static_cast<uint16_t>(static_cast<uint16_t>(writeBuffer[14]) | (static_cast<uint16_t>(writeBuffer[15]) << 8U)), 0xD7FFU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_REAL64)
{
  CreateUUT_REAL64_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_d = 15.78;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Write_double(data_d);
  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], writeBuffer[8]);
  EXPECT_EQ(writeBuffer[1], writeBuffer[9]);
  EXPECT_EQ(writeBuffer[2], writeBuffer[10]);
  EXPECT_EQ(writeBuffer[3], writeBuffer[11]);
  EXPECT_EQ(writeBuffer[4], writeBuffer[12]);
  EXPECT_EQ(writeBuffer[5], writeBuffer[13]);
  EXPECT_EQ(writeBuffer[6], writeBuffer[14]);
  EXPECT_EQ(writeBuffer[7], writeBuffer[15]);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_INTEGER64)
{
  CreateUUT_INTEGER64_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_i64 = 0x5BF61299FF21345BLL;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5BU);
  EXPECT_EQ(writeBuffer[1], 0x34U);
  EXPECT_EQ(writeBuffer[2], 0x21U);
  EXPECT_EQ(writeBuffer[3], 0xFFU);
  EXPECT_EQ(writeBuffer[4], 0x99U);
  EXPECT_EQ(writeBuffer[5], 0x12U);
  EXPECT_EQ(writeBuffer[6], 0xF6U);
  EXPECT_EQ(writeBuffer[7], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_UNSIGNED64)
{
  CreateUUT_UNSIGNED64_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_ui64 = 0x5BF61299FF21345BULL;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5BU);
  EXPECT_EQ(writeBuffer[1], 0x34U);
  EXPECT_EQ(writeBuffer[2], 0x21U);
  EXPECT_EQ(writeBuffer[3], 0xFFU);
  EXPECT_EQ(writeBuffer[4], 0x99U);
  EXPECT_EQ(writeBuffer[5], 0x12U);
  EXPECT_EQ(writeBuffer[6], 0xF6U);
  EXPECT_EQ(writeBuffer[7], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT1)
{
  CreateUUT_BIT1_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xFEU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x01U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x02U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT2)
{
  CreateUUT_BIT2_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xFCU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x03U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x0CU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT3)
{
  CreateUUT_BIT3_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xFAU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x05U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x2AU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT4)
{
  CreateUUT_BIT4_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xF5U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x0AU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0xA5U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT5)
{
  CreateUUT_BIT5_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xE5U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x1AU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x45U);
  EXPECT_EQ(writeBuffer[1], 0x03U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT6)
{
  CreateUUT_BIT6_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xC0U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x3FU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0xC0U);
  EXPECT_EQ(writeBuffer[1], 0x0FU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT7)
{
  CreateUUT_BIT7_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0x80U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0x7FU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x80U);
  EXPECT_EQ(writeBuffer[1], 0x3FU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BIT8)
{
  CreateUUT_BIT8_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0x00U;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0xFFU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x00U);
  EXPECT_EQ(writeBuffer[1], 0xFFU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BOOLEAN_NATIVE_BIT1)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data_bitX = 0xFEU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  data_bitX = 0xFFU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x02U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_InvalidSubindex)
{
  CreateUUT_UNSIGNED8_RW();

  auto locker(spUUT->LockData());

  data_ui8 = 0xABU;
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::SubindexDoesNotExist);

  // check: stream writer has not been modified
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_InsufficientPermission)
{
  CreateUUT_UNSIGNED8_RO_Preop();

  auto locker(spUUT->LockData());

  data_ui8 = 0x5BU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_SAFEOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);

  // check: stream writer has not been modified
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_WithoutCallback)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned8,
                            1U,
                            Object::attr_ACCESS_RD_PREOP,
                            &data_ui8,
                            &mutex,
                            nullptr,
                            nullptr,
                            nullptr));

  auto locker(spUUT->LockData());

  data_ui8 = 0x5BU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BeforeReadCallbackRejects)
{
  CreateUUT_UNSIGNED8_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralError));

  auto locker(spUUT->LockData());

  data_ui8 = 0x5BU;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralError);

  // check: stream writer has not been modified
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_StreamWriterFullyUsed)
{
  CreateUUT_UNSIGNED32_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // replace the stream writer to "writeBuffer" with our own
  writeBufferWriter.Close();
  MemStreamWriter msw(writeBuffer, 4U, MemStreamWriter::Endian::Little);

  data_ui32 = 0x5BF61299UL;
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, msw), SDOAbortCode::OK);

  EXPECT_EQ(msw.RemainingCapacity(), 0U);
  EXPECT_EQ(msw.GetState(), IStreamWriter::States::full);
  msw.Close();

  EXPECT_EQ(writeBuffer[0], 0x99U);
  EXPECT_EQ(writeBuffer[1], 0x12U);
  EXPECT_EQ(writeBuffer[2], 0xF6U);
  EXPECT_EQ(writeBuffer[3], 0x5BU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_StreamWriterHasNotEnoughSpace)
{
  CreateUUT_UNSIGNED32_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // replace the stream writer to "writeBuffer" with our own
  writeBufferWriter.Close();
  MemStreamWriter msw(writeBuffer, 2U, MemStreamWriter::Endian::Little);

  data_ui32 = 0x5BF61299UL;
  ASSERT_THROW(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, msw), FullError);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Read_BeforeReadCallbackThrows)
{
  CreateUUT_UNSIGNED32_RW();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  data_ui32 = 0x5BF61299UL;
  ASSERT_THROW(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);

  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));
  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BOOLEAN)
{
  CreateUUT_BOOLEAN_RW();

  // define variables for preview values and define lambdas to catch them
  bool pv1 = false;
  bool pv2 = false;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<bool const*>(pData); };

  auto recorder2 = [&pv2](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv2 = *static_cast<bool const*>(pData); };

  // prepare mock
  {
    InSequence seq;

    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder2), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x01U;
  readBuffer[1] = 0x00U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_TRUE(pv1);
  EXPECT_TRUE(data_bool);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_FALSE(pv2);
  EXPECT_FALSE(data_bool);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_INTEGER8)
{
  CreateUUT_INTEGER8_RW();

  // define variable for preview value and define a lambda to catch it
  int8_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<int8_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 87U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 87);
  EXPECT_EQ(data_i8, 87);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_INTEGER16)
{
  CreateUUT_INTEGER16_RW();

  // define variable for preview value and define a lambda to catch it
  int16_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<int16_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 8983);
  EXPECT_EQ(data_i16, 8983);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_INTEGER32)
{
  CreateUUT_INTEGER32_RW();

  // define variable for preview value and define a lambda to catch it
  int32_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<int32_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 1158882071L);
  EXPECT_EQ(data_i32, 1158882071L);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_UNSIGNED8)
{
  CreateUUT_UNSIGNED8_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 87U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 87U);
  EXPECT_EQ(data_ui8, 87U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_UNSIGNED16)
{
  CreateUUT_UNSIGNED16_RW();

  // define variable for preview value and define a lambda to catch it
  uint16_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint16_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 8983U);
  EXPECT_EQ(data_ui16, 8983U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_UNSIGNED32)
{
  CreateUUT_UNSIGNED32_RW();

  // define variable for preview value and define a lambda to catch it
  uint32_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint32_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 1158882071UL);
  EXPECT_EQ(data_ui32, 1158882071UL);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_REAL32)
{
  CreateUUT_REAL32_RW();

  // define variable for preview value and define a lambda to catch it
  float pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<float const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  MemStreamWriter msw(readBuffer, sizeof(readBuffer), IStreamWriter::Endian::Little);
  msw.Write_float(23.5);
  msw.Close();

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv1, 23.5);
  EXPECT_EQ(data_f, 23.5);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_VISIBLE_STRING_empty_A)
{
  // this is variant 'A': Zero bytes are passed to Write(...)

  CreateUUT_VISIBLE_STRING_RW();

  // Fill target with 0xFF. We want to see that it is filled with zeros by Write(...).
  memset(data_visible_string, 0xFF, sizeof(data_visible_string));

  // define variable for preview value and define a lambda to catch it
  char pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  gpcc::Stream::MemStreamReader sr(nullptr, 0, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0);
  EXPECT_EQ(pv[1], 0);
  EXPECT_EQ(pv[2], 0);
  EXPECT_EQ(pv[3], 0);
  EXPECT_EQ(pv[4], 0);
  EXPECT_EQ(pv[5], 0);
  EXPECT_EQ(pv[6], 0);
  EXPECT_EQ(pv[7], 0);

  EXPECT_EQ(data_visible_string[0], 0);
  EXPECT_EQ(data_visible_string[1], 0);
  EXPECT_EQ(data_visible_string[2], 0);
  EXPECT_EQ(data_visible_string[3], 0);
  EXPECT_EQ(data_visible_string[4], 0);
  EXPECT_EQ(data_visible_string[5], 0);
  EXPECT_EQ(data_visible_string[6], 0);
  EXPECT_EQ(data_visible_string[7], 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_VISIBLE_STRING_empty_B)
{
  // this is variant 'B': A single NUL is passed to Write(...)

  CreateUUT_VISIBLE_STRING_RW();

  // Fill target with 0xFF. We want to see that it is filled with zeros by Write(...).
  memset(data_visible_string, 0xFF, sizeof(data_visible_string));

  // define variable for preview value and define a lambda to catch it
  char pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0);
  EXPECT_EQ(pv[1], 0);
  EXPECT_EQ(pv[2], 0);
  EXPECT_EQ(pv[3], 0);
  EXPECT_EQ(pv[4], 0);
  EXPECT_EQ(pv[5], 0);
  EXPECT_EQ(pv[6], 0);
  EXPECT_EQ(pv[7], 0);

  EXPECT_EQ(data_visible_string[0], 0);
  EXPECT_EQ(data_visible_string[1], 0);
  EXPECT_EQ(data_visible_string[2], 0);
  EXPECT_EQ(data_visible_string[3], 0);
  EXPECT_EQ(data_visible_string[4], 0);
  EXPECT_EQ(data_visible_string[5], 0);
  EXPECT_EQ(data_visible_string[6], 0);
  EXPECT_EQ(data_visible_string[7], 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_VISIBLE_STRING_empty_C)
{
  // this is variant 'C': A full bunch of NULs is passed to Write(...)

  CreateUUT_VISIBLE_STRING_RW();

  // Fill target with 0xFF. We want to see that it is filled with zeros by Write(...).
  memset(data_visible_string, 0xFF, sizeof(data_visible_string));

  // define variable for preview value and define a lambda to catch it
  char pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0U;
  readBuffer[1] = 0U;
  readBuffer[2] = 0U;
  readBuffer[3] = 0U;
  readBuffer[4] = 0U;
  readBuffer[5] = 0U;
  readBuffer[6] = 0U;
  readBuffer[7] = 0U;

  auto sr = readBufferReader.SubStream(8U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0);
  EXPECT_EQ(pv[1], 0);
  EXPECT_EQ(pv[2], 0);
  EXPECT_EQ(pv[3], 0);
  EXPECT_EQ(pv[4], 0);
  EXPECT_EQ(pv[5], 0);
  EXPECT_EQ(pv[6], 0);
  EXPECT_EQ(pv[7], 0);

  EXPECT_EQ(data_visible_string[0], 0);
  EXPECT_EQ(data_visible_string[1], 0);
  EXPECT_EQ(data_visible_string[2], 0);
  EXPECT_EQ(data_visible_string[3], 0);
  EXPECT_EQ(data_visible_string[4], 0);
  EXPECT_EQ(data_visible_string[5], 0);
  EXPECT_EQ(data_visible_string[6], 0);
  EXPECT_EQ(data_visible_string[7], 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_VISIBLE_STRING_half_A)
{
  // this is variant 'A': A few chars and a NUL are passed to write. The length of the data passed to write
  //                      is less than the maximum

  CreateUUT_VISIBLE_STRING_RW();

  // Fill target with 0xFF. We want to see that the unsed rest is filled with zeros by Write(...).
  memset(data_visible_string, 0xFF, sizeof(data_visible_string));

  // define variable for preview value and define a lambda to catch it
  char pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = static_cast<uint8_t>('T');
  readBuffer[1] = static_cast<uint8_t>('e');
  readBuffer[2] = static_cast<uint8_t>('s');
  readBuffer[3] = static_cast<uint8_t>('t');
  readBuffer[4] = 0U;

  auto sr = readBufferReader.SubStream(5U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 'T');
  EXPECT_EQ(pv[1], 'e');
  EXPECT_EQ(pv[2], 's');
  EXPECT_EQ(pv[3], 't');
  EXPECT_EQ(pv[4], 0);
  EXPECT_EQ(pv[5], 0);
  EXPECT_EQ(pv[6], 0);
  EXPECT_EQ(pv[7], 0);

  EXPECT_EQ(data_visible_string[0], 'T');
  EXPECT_EQ(data_visible_string[1], 'e');
  EXPECT_EQ(data_visible_string[2], 's');
  EXPECT_EQ(data_visible_string[3], 't');
  EXPECT_EQ(data_visible_string[4], 0);
  EXPECT_EQ(data_visible_string[5], 0);
  EXPECT_EQ(data_visible_string[6], 0);
  EXPECT_EQ(data_visible_string[7], 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_VISIBLE_STRING_half_B)
{
  // this is variant 'B': A few chars and multiple NUL are passed to write. The length of the data passed to write
  //                      matches the maximum size of the object

  CreateUUT_VISIBLE_STRING_RW();

  // Fill target with 0xFF. We want to see that the unsed rest is filled with zeros by Write(...).
  memset(data_visible_string, 0xFF, sizeof(data_visible_string));

  // define variable for preview value and define a lambda to catch it
  char pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = static_cast<uint8_t>('T');
  readBuffer[1] = static_cast<uint8_t>('e');
  readBuffer[2] = static_cast<uint8_t>('s');
  readBuffer[3] = static_cast<uint8_t>('t');
  readBuffer[4] = 0U;
  readBuffer[5] = 0U;
  readBuffer[6] = 0U;
  readBuffer[7] = 0U;

  auto sr = readBufferReader.SubStream(8U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 'T');
  EXPECT_EQ(pv[1], 'e');
  EXPECT_EQ(pv[2], 's');
  EXPECT_EQ(pv[3], 't');
  EXPECT_EQ(pv[4], 0);
  EXPECT_EQ(pv[5], 0);
  EXPECT_EQ(pv[6], 0);
  EXPECT_EQ(pv[7], 0);

  EXPECT_EQ(data_visible_string[0], 'T');
  EXPECT_EQ(data_visible_string[1], 'e');
  EXPECT_EQ(data_visible_string[2], 's');
  EXPECT_EQ(data_visible_string[3], 't');
  EXPECT_EQ(data_visible_string[4], 0);
  EXPECT_EQ(data_visible_string[5], 0);
  EXPECT_EQ(data_visible_string[6], 0);
  EXPECT_EQ(data_visible_string[7], 0);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_VISIBLE_STRING_full)
{
  CreateUUT_VISIBLE_STRING_RW();

  // define variable for preview value and define a lambda to catch it
  char pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = static_cast<uint8_t>('T');
  readBuffer[1] = static_cast<uint8_t>('e');
  readBuffer[2] = static_cast<uint8_t>('s');
  readBuffer[3] = static_cast<uint8_t>('t');
  readBuffer[4] = static_cast<uint8_t>('1');
  readBuffer[5] = static_cast<uint8_t>('2');
  readBuffer[6] = static_cast<uint8_t>('3');
  readBuffer[7] = static_cast<uint8_t>('4');

  auto sr = readBufferReader.SubStream(8U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 'T');
  EXPECT_EQ(pv[1], 'e');
  EXPECT_EQ(pv[2], 's');
  EXPECT_EQ(pv[3], 't');
  EXPECT_EQ(pv[4], '1');
  EXPECT_EQ(pv[5], '2');
  EXPECT_EQ(pv[6], '3');
  EXPECT_EQ(pv[7], '4');

  EXPECT_EQ(data_visible_string[0], 'T');
  EXPECT_EQ(data_visible_string[1], 'e');
  EXPECT_EQ(data_visible_string[2], 's');
  EXPECT_EQ(data_visible_string[3], 't');
  EXPECT_EQ(data_visible_string[4], '1');
  EXPECT_EQ(data_visible_string[5], '2');
  EXPECT_EQ(data_visible_string[6], '3');
  EXPECT_EQ(data_visible_string[7], '4');
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_OCTET_STRING)
{
  CreateUUT_OCTET_STRING_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv[4];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x12U;
  readBuffer[1] = 0x34U;
  readBuffer[2] = 0x56U;
  readBuffer[3] = 0x78U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0x12U);
  EXPECT_EQ(pv[1], 0x34U);
  EXPECT_EQ(pv[2], 0x56U);
  EXPECT_EQ(pv[3], 0x78U);

  EXPECT_EQ(data_octet_string[0], 0x12U);
  EXPECT_EQ(data_octet_string[1], 0x34U);
  EXPECT_EQ(data_octet_string[2], 0x56U);
  EXPECT_EQ(data_octet_string[3], 0x78U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_UNICODE_STRING)
{
  CreateUUT_UNICODE_STRING_RW();

  // define variable for preview value and define a lambda to catch it
  uint16_t pv[8];

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(pv, pData, sizeof(pv)); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[ 0] = 0xA1U;   readBuffer[ 1] = 0x34U;
  readBuffer[ 2] = 0xB1U;   readBuffer[ 3] = 0x35U;
  readBuffer[ 4] = 0xC1U;   readBuffer[ 5] = 0x36U;
  readBuffer[ 6] = 0xD1U;   readBuffer[ 7] = 0x37U;
  readBuffer[ 8] = 0xE1U;   readBuffer[ 9] = 0x38U;
  readBuffer[10] = 0xF1U;   readBuffer[11] = 0x39U;
  readBuffer[12] = 0xA2U;   readBuffer[13] = 0x40U;
  readBuffer[14] = 0xB2U;   readBuffer[15] = 0x41U;

  auto sr = readBufferReader.SubStream(16U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);

  EXPECT_EQ(pv[0], 0x34A1U);
  EXPECT_EQ(pv[1], 0x35B1U);
  EXPECT_EQ(pv[2], 0x36C1U);
  EXPECT_EQ(pv[3], 0x37D1U);
  EXPECT_EQ(pv[4], 0x38E1U);
  EXPECT_EQ(pv[5], 0x39F1U);
  EXPECT_EQ(pv[6], 0x40A2U);
  EXPECT_EQ(pv[7], 0x41B2U);

  EXPECT_EQ(data_unicode_string[0], 0x34A1U);
  EXPECT_EQ(data_unicode_string[1], 0x35B1U);
  EXPECT_EQ(data_unicode_string[2], 0x36C1U);
  EXPECT_EQ(data_unicode_string[3], 0x37D1U);
  EXPECT_EQ(data_unicode_string[4], 0x38E1U);
  EXPECT_EQ(data_unicode_string[5], 0x39F1U);
  EXPECT_EQ(data_unicode_string[6], 0x40A2U);
  EXPECT_EQ(data_unicode_string[7], 0x41B2U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_INTEGER64)
{
  CreateUUT_INTEGER64_RW();

  // define variable for preview value and define a lambda to catch it
  int64_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<int64_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;
  readBuffer[4] = 0xA5U;
  readBuffer[5] = 0xD3U;
  readBuffer[6] = 0xF5U;
  readBuffer[7] = 0x13U;

  auto sr = readBufferReader.SubStream(8U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 0x13F5D3A545132317LL);
  EXPECT_EQ(data_i64, 0x13F5D3A545132317LL);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_UNSIGNED64)
{
  CreateUUT_UNSIGNED64_RW();

  // define variable for preview value and define a lambda to catch it
  uint64_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint64_t const*>(pData); };

  // prepare mock
  {
    InSequence seq;
    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false));
  }

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;
  readBuffer[4] = 0xA5U;
  readBuffer[5] = 0xD3U;
  readBuffer[6] = 0xF5U;
  readBuffer[7] = 0x13U;

  auto sr = readBufferReader.SubStream(8U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1, 0x13F5D3A545132317ULL);
  EXPECT_EQ(data_ui64, 0x13F5D3A545132317ULL);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT1)
{
  CreateUUT_BIT1_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(2).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(2);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x01U;
  readBuffer[1] = 0x00U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x01U);
  EXPECT_EQ(data_bitX, 0x01U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x00U);
  EXPECT_EQ(data_bitX, 0x00U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT2)
{
  CreateUUT_BIT2_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(3).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(3);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x01U;
  readBuffer[1] = 0x02U;
  readBuffer[2] = 0x03U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x01U);
  EXPECT_EQ(data_bitX, 0x01U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x02U);
  EXPECT_EQ(data_bitX, 0x02U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x03U);
  EXPECT_EQ(data_bitX, 0x03U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT3)
{
  CreateUUT_BIT3_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(5).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(5);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x01U;
  readBuffer[1] = 0x02U;
  readBuffer[2] = 0x04U;
  readBuffer[3] = 0x07U;
  readBuffer[4] = 0x00U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x01U);
  EXPECT_EQ(data_bitX, 0x01U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x02U);
  EXPECT_EQ(data_bitX, 0x02U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x04U);
  EXPECT_EQ(data_bitX, 0x04U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x07U);
  EXPECT_EQ(data_bitX, 0x07U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x00U);
  EXPECT_EQ(data_bitX, 0x00U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT4)
{
  CreateUUT_BIT4_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(4).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(4);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x01U;
  readBuffer[1] = 0x02U;
  readBuffer[2] = 0x04U;
  readBuffer[3] = 0x08U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x01U);
  EXPECT_EQ(data_bitX, 0x01U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x02U);
  EXPECT_EQ(data_bitX, 0x02U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x04U);
  EXPECT_EQ(data_bitX, 0x04U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x08U);
  EXPECT_EQ(data_bitX, 0x08U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT5)
{
  CreateUUT_BIT5_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(2).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(2);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x13U;
  readBuffer[1] = 0x14U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x13U);
  EXPECT_EQ(data_bitX, 0x13U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x14U);
  EXPECT_EQ(data_bitX, 0x14U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT6)
{
  CreateUUT_BIT6_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(2).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(2);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x3FU;
  readBuffer[1] = 0x3CU;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x3FU);
  EXPECT_EQ(data_bitX, 0x3FU);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x3CU);
  EXPECT_EQ(data_bitX, 0x3CU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT7)
{
  CreateUUT_BIT7_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(2).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(2);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x7FU;
  readBuffer[1] = 0x7EU;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x7FU);
  EXPECT_EQ(data_bitX, 0x7FU);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x7EU);
  EXPECT_EQ(data_bitX, 0x7EU);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BIT8)
{
  CreateUUT_BIT8_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(2).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(2);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0xF0U;
  readBuffer[1] = 0xE3U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0xF0U);
  EXPECT_EQ(data_bitX, 0xF0U);

  sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0xE3U);
  EXPECT_EQ(data_bitX, 0xE3U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BOOLEAN_NATIVE_BIT1)
{
  CreateUUT_BOOLEAN_NATIVE_BIT1_RW();

  // define variable for preview value and define a lambda to catch it
  uint8_t pv1 = 0U;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint8_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).Times(2).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).Times(2);

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0xFEU;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x00U);
  EXPECT_EQ(data_bitX, 0x00U);

  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(pv1,       0x01U);
  EXPECT_EQ(data_bitX, 0x01U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_SubindexNotExisting)
{
  CreateUUT_UNSIGNED8_RW();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 87U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::SubindexDoesNotExist);

  EXPECT_EQ(data_ui8, 0U);
  EXPECT_EQ(sr.RemainingBytes(), 1U) << "Data has been read from the StreamReader. This was not expected";
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_InsufficientPermission)
{
  CreateUUT_UNSIGNED8_RO_Preop();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 87U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::AttemptToWriteRdOnlyObject);

  EXPECT_EQ(data_ui8, 0U);
  EXPECT_EQ(sr.RemainingBytes(), 1U) << "Data has been read from the StreamReader. This was not expected";
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_StreamReaderEmpty)
{
  CreateUUT_UNSIGNED32_RW();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = gpcc::Stream::MemStreamReader(nullptr, 0U, gpcc::Stream::IStreamReader::Endian::Little);
  auto const retVal = spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr);
  ASSERT_EQ(SDOAbortCode::DataTypeMismatchTooSmall, retVal);

  EXPECT_EQ(data_ui32, 0U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_NotEnoughData_SmallObject)
{
  CreateUUT_UNSIGNED32_RW();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = readBufferReader.SubStream(2U);
  auto const retVal = spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr);
  ASSERT_EQ(SDOAbortCode::DataTypeMismatchTooSmall, retVal);

  EXPECT_EQ(data_ui32, 0U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_NotEnoughData_LargeObject)
{
  CreateUUT_UNICODE_STRING_RW();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[ 0] = 'T';   readBuffer[ 1] = 0U;
  readBuffer[ 2] = 'e';   readBuffer[ 3] = 0U;
  readBuffer[ 4] = 's';   readBuffer[ 5] = 0U;
  readBuffer[ 6] = 't';   readBuffer[ 7] = 0U;
  readBuffer[ 8] = '1';   readBuffer[ 9] = 0U;
  readBuffer[10] = '2';   readBuffer[11] = 0U;
  readBuffer[12] = '3';   readBuffer[13] = 0U;
  readBuffer[14] = '4';   readBuffer[15] = 0U;

  auto sr = readBufferReader.SubStream(8U);
  auto const retVal = spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr);
  ASSERT_EQ(SDOAbortCode::DataTypeMismatchTooSmall, retVal);

  EXPECT_EQ(data_unicode_string[0], 0U);
  EXPECT_EQ(data_unicode_string[1], 0U);
  EXPECT_EQ(data_unicode_string[2], 0U);
  EXPECT_EQ(data_unicode_string[3], 0U);
  EXPECT_EQ(data_unicode_string[4], 0U);
  EXPECT_EQ(data_unicode_string[5], 0U);
  EXPECT_EQ(data_unicode_string[6], 0U);
  EXPECT_EQ(data_unicode_string[7], 0U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_TooManyData)
{
  CreateUUT_UNSIGNED8_RW();

  // prepare mock
  // - no calls expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 87U;

  auto sr = readBufferReader.SubStream(2U);
  auto const retVal = spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr);
  ASSERT_EQ(SDOAbortCode::DataTypeMismatchTooLong, retVal);

  EXPECT_EQ(data_ui8, 0U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BeforeWriteCallbackRejects)
{
  CreateUUT_UNSIGNED32_RW();

  // define variable for preview value and define a lambda to catch it
  uint32_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint32_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::GeneralError)));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::GeneralError);
  EXPECT_EQ(pv1, 0x45132317UL);
  EXPECT_EQ(data_ui32, 0U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_BeforeWriteCallbackThrows)
{
  CreateUUT_UNSIGNED32_RW();

  // define variable for preview value and define a lambda to catch it
  uint32_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint32_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Throw(std::runtime_error("Test"))));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_THROW((void)spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), std::runtime_error);
  EXPECT_EQ(pv1, 0x45132317UL);
  EXPECT_EQ(data_ui32, 0U);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, Write_NoCallbacks)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned32,
                            1U,
                            Object::attr_ACCESS_RW,
                            &data_ui32,
                            &mutex,
                            nullptr,
                            nullptr,
                            nullptr));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x17U;
  readBuffer[1] = 0x23U;
  readBuffer[2] = 0x13U;
  readBuffer[3] = 0x45U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::OK);
  EXPECT_EQ(data_ui32, 0x45132317UL);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_DeathTestsF, Write_AfterWriteCallbackThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto test = [&]()
  {
    CreateUUT_UNSIGNED8_RW();

    // prepare mock
    {
      InSequence seq;
      EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, false, 0, _)).WillOnce(Return(SDOAbortCode::OK));
      EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, false)).WillOnce(Throw(std::runtime_error("Test")));
    }

    // stimulus

    auto locker(spUUT->LockData());

    readBuffer[0] = 87U;

    auto sr = readBufferReader.SubStream(1U);

    // leathal call:
    (void)spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr);
  };

  EXPECT_DEATH(test(), ".*After-write-callback threw.*");
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CompleteRead)
{
  CreateUUT_UNSIGNED8_RW();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::UnsupportedAccessToObject);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, CompleteWrite)
{
  CreateUUT_UNSIGNED8_RW();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, readBufferReader, IStreamReader::RemainingNbOfBits::any), SDOAbortCode::UnsupportedAccessToObject);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, SetData_NoMutex)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned8,
                            1U,
                            Object::attr_ACCESS_RD,
                            &data_ui8,
                            nullptr,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));


  // prepare mock
  // - no call expected -

  // stimulus

  EXPECT_THROW(spUUT->SetData(&data_ui16), std::logic_error);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, SetData_nullptr)
{
  spUUT.reset(new ObjectVAR_wicb("ObjName",
                            DataType::unsigned8,
                            1U,
                            Object::attr_ACCESS_RD,
                            &data_ui8,
                            &mutex,
                            std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                            std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                            std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));


  // prepare mock
  // - no call expected -

  // stimulus

  EXPECT_THROW(spUUT->SetData(nullptr), std::invalid_argument);
}

TEST_F(gpcc_cood_ObjectVAR_wicb_TestsF, SetData_OK)
{
  CreateUUT_UNSIGNED8_RW();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(2).WillRepeatedly(Return(SDOAbortCode::OK));

  data_ui8 = 0x5BU;
  data_i8 = 0x12U;

  {
    auto locker(spUUT->LockData());
    EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  }

  spUUT->SetData(&data_i8);

  {
    auto locker(spUUT->LockData());
    EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  }

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x5BU);
  EXPECT_EQ(writeBuffer[1], 0x12U);
}

} // namespace gpcc_tests
} // namespace cood
