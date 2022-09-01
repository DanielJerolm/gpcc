/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include <gpcc/cood/ObjectRECORD_wicb.hpp>
#include <gpcc/cood/exceptions.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/raii/scope_guard.hpp>
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

/// Test fixture for gpcc::cood::ObjectRECORD_wicb related tests.
class gpcc_cood_ObjectRECORD_wicb_TestsF: public Test
{
  public:
    gpcc_cood_ObjectRECORD_wicb_TestsF(void);

  protected:
    // RECORD object description: All RW, no gaps
    static ObjectRECORD_wicb::SubIdxDescr const siDescr_A[11];

    // RECORD object description: All RW, one gap
    static ObjectRECORD_wicb::SubIdxDescr const siDescr_B[12];

    // RECORD object description: One WR, one RD, rest RW
    static ObjectRECORD_wicb::SubIdxDescr const siDescr_C[11];

    // RECORD object description: All RD
    static ObjectRECORD_wicb::SubIdxDescr const siDescr_D[11];

    // RECORD object description: All RW, one empty SI
    static ObjectRECORD_wicb::SubIdxDescr const siDescr_E[12];

    // RECORD object description: All RW, no gaps, uses data type boolean_native_bit1
    static ObjectRECORD_wicb::SubIdxDescr const siDescr_F[11];

    // Mutex protecting the data
    gpcc::osal::Mutex mutex;

    // The data
    struct Data
    {
      bool data_bool;
      int8_t data_i8;
      uint8_t data_ui8;
      uint32_t data_ui32a;
      uint8_t data_bitX[4];
      char data_visiblestring[8];
      uint32_t data_ui32b;
      uint8_t data_octectstring[4];

      bool operator==(Data const & rhv) const;
    } data;

    // Mock for reception of callbacks
    StrictMock<IObjectNotifiableMock> cbm;

    // Buffers for use with MemStreamReader and MemStreamWriter (255 x 4 + 2)
    uint8_t readBuffer[1022];
    uint8_t writeBuffer[1022];

    // Stream reader/writer for the buffers above
    gpcc::Stream::MemStreamReader readBufferReader;
    gpcc::Stream::MemStreamWriter writeBufferWriter;

    // ...and finally the UUT
    std::unique_ptr<ObjectRECORD_wicb> spUUT;

    virtual ~gpcc_cood_ObjectRECORD_wicb_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

    void CreateUUT_A(void);
    void CreateUUT_B(void);
    void CreateUUT_C(void);
    void CreateUUT_D(bool const withMutex);
    void CreateUUT_E(void);
    void CreateUUT_F(void);
};

ObjectRECORD_wicb::SubIdxDescr const gpcc_cood_ObjectRECORD_wicb_TestsF::siDescr_A[11] =
{
    // name,        type,                     attributes,                   nElements, byteOffset,                                                        bitOffset
    { "Data Bool",  DataType::boolean,        Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bool),          0},
    { "Data i8",    DataType::integer8,       Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,      Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui8),           0},
    { "Data ui32a", DataType::unsigned32,     Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32a),         0},
    { "Bit 0",      DataType::bit1,           Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          0},
    { "Bit 7..8",   DataType::bit2,           Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          7},
    { "Bit 1",      DataType::bit1,           Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          1},
    { "Bit 28..31", DataType::bit4,           Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX) + 3,      4},
    { "Text",       DataType::visible_string, Object::attr_ACCESS_RW,       8,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,     Object::attr_ACCESS_WR_PREOP |
                                              Object::attr_ACCESS_RD,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,   Object::attr_ACCESS_RW,       4,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_octectstring),  0}
};

ObjectRECORD_wicb::SubIdxDescr const gpcc_cood_ObjectRECORD_wicb_TestsF::siDescr_B[12] =
{
    // name,        type,                     attributes,               nElements, byteOffset,                                                        bitOffset
    { "Data Bool",  DataType::boolean,        Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bool),          0},
    { "Data i8",    DataType::integer8,       Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,      Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui8),           0},
    { "Align",      DataType::null,           Object::attr_ACCESS_RW,   8,         0,                                                                 0},
    { "Data ui32a", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32a),         0},
    { "Bit 0",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          0},
    { "Bit 7..8",   DataType::bit2,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          7},
    { "Bit 1",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          1},
    { "Bit 28..31", DataType::bit4,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX) + 3,      4},
    { "Text",       DataType::visible_string, Object::attr_ACCESS_RW,   8,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,   Object::attr_ACCESS_RW,   4,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_octectstring),  0}
};

ObjectRECORD_wicb::SubIdxDescr const gpcc_cood_ObjectRECORD_wicb_TestsF::siDescr_C[11] =
{
    // name,        type,                     attributes,               nElements, byteOffset,                                                        bitOffset
    { "Data Bool",  DataType::boolean,        Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bool),          0},
    { "Data i8",    DataType::integer8,       Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,      Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui8),           0},
    { "Data ui32a", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32a),         0},
    { "Bit 0",      DataType::bit1,           Object::attr_ACCESS_WR,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          0},
    { "Bit 7..8",   DataType::bit2,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          7},
    { "Bit 1",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          1},
    { "Bit 28..31", DataType::bit4,           Object::attr_ACCESS_RD,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX) + 3,      4},
    { "Text",       DataType::visible_string, Object::attr_ACCESS_RW,   8,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,     Object::attr_ACCESS_RD,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,   Object::attr_ACCESS_RW,   4,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_octectstring),  0}
};

ObjectRECORD_wicb::SubIdxDescr const gpcc_cood_ObjectRECORD_wicb_TestsF::siDescr_D[11] =
{
    // name,        type,                     attributes,               nElements, byteOffset,                                                        bitOffset
    { "Data Bool",  DataType::boolean,        Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bool),          0},
    { "Data i8",    DataType::integer8,       Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,      Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui8),           0},
    { "Data ui32a", DataType::unsigned32,     Object::attr_ACCESS_RD_PREOP, 1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32a),         0},
    { "Bit 0",      DataType::bit1,           Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          0},
    { "Bit 7..8",   DataType::bit2,           Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          7},
    { "Bit 1",      DataType::bit1,           Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          1},
    { "Bit 28..31", DataType::bit4,           Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX) + 3,      4},
    { "Text",       DataType::visible_string, Object::attr_ACCESS_RD,       8,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,     Object::attr_ACCESS_RD,       1,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,   Object::attr_ACCESS_RD,       4,     offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_octectstring),  0}
};

ObjectRECORD_wicb::SubIdxDescr const gpcc_cood_ObjectRECORD_wicb_TestsF::siDescr_E[12] =
{
    // name,        type,                     attributes,               nElements, byteOffset,                                                        bitOffset
    { "Data Bool",  DataType::boolean,        Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bool),          0},
    { "Data i8",    DataType::integer8,       Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,      Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui8),           0},
    { nullptr,      DataType::null,           0,                        0,         0,                                                                 0},
    { "Data ui32a", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32a),         0},
    { "Bit 0",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          0},
    { "Bit 7..8",   DataType::bit2,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          7},
    { "Bit 1",      DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          1},
    { "Bit 28..31", DataType::bit4,           Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX) + 3,      4},
    { "Text",       DataType::visible_string, Object::attr_ACCESS_RW,   8,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,   Object::attr_ACCESS_RW,   4,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_octectstring),  0}
};

ObjectRECORD_wicb::SubIdxDescr const gpcc_cood_ObjectRECORD_wicb_TestsF::siDescr_F[11] =
{
    // name,        type,                          attributes,                   nElements, byteOffset,                                                        bitOffset
    { "Data Bool",  DataType::boolean,             Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bool),          0},
    { "Data i8",    DataType::integer8,            Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_i8),            0},
    { "Data ui8",   DataType::unsigned8,           Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui8),           0},
    { "Data ui32a", DataType::unsigned32,          Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32a),         0},
    { "Bool_Bit0",  DataType::boolean_native_bit1, Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          0},
    { "Bool_Bit1",  DataType::boolean_native_bit1, Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          1},
    { "Bool_Bit2",  DataType::boolean_native_bit1, Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          2},
    { "Bool_Bit3",  DataType::boolean_native_bit1, Object::attr_ACCESS_RW,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_bitX),          3},
    { "Text",       DataType::visible_string,      Object::attr_ACCESS_RW,       8,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_visiblestring), 0},
    { "Data ui32b", DataType::unsigned32,          Object::attr_ACCESS_WR_PREOP |
                                                   Object::attr_ACCESS_RD,       1,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_ui32b),         0},
    { "Octet str",  DataType::octet_string,        Object::attr_ACCESS_RW,       4,         offsetof(gpcc_cood_ObjectRECORD_wicb_TestsF::Data, data_octectstring),  0}
};

bool gpcc_cood_ObjectRECORD_wicb_TestsF::Data::operator==(Data const & rhv) const
{
  return (memcmp(this, &rhv, sizeof(Data)) == 0);
}

gpcc_cood_ObjectRECORD_wicb_TestsF::gpcc_cood_ObjectRECORD_wicb_TestsF(void)
: Test()
, mutex()
, data()
, cbm()
, readBuffer()
, writeBuffer()
, readBufferReader(&readBuffer, sizeof(readBuffer), gpcc::Stream::MemStreamReader::Endian::Little)
, writeBufferWriter(&writeBuffer, sizeof(writeBuffer), gpcc::Stream::MemStreamWriter::Endian::Little)
, spUUT()
{
  strcpy(data.data_visiblestring, "Test!");
}

gpcc_cood_ObjectRECORD_wicb_TestsF::~gpcc_cood_ObjectRECORD_wicb_TestsF(void)
{
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::SetUp(void)
{
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::TearDown(void)
{
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::CreateUUT_A(void)
{
  spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                         11U,
                                         &data,
                                         sizeof(Data),
                                         &mutex,
                                         siDescr_A,
                                         std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                         std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                         std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::CreateUUT_B(void)
{
  spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (B)",
                                         12U,
                                         &data,
                                         sizeof(Data),
                                         &mutex,
                                         siDescr_B,
                                         std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                         std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                         std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::CreateUUT_C(void)
{
  spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (C)",
                                         11U,
                                         &data,
                                         sizeof(Data),
                                         &mutex,
                                         siDescr_C,
                                         std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                         std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                         std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::CreateUUT_D(bool const withMutex)
{
  spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (D)",
                                         11U,
                                         &data,
                                         sizeof(Data),
                                         withMutex ? (&mutex) : nullptr,
                                         siDescr_D,
                                         std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                         std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                         std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::CreateUUT_E(void)
{
  spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (E)",
                                         12U,
                                         &data,
                                         sizeof(Data),
                                         &mutex,
                                         siDescr_E,
                                         std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                         std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                         std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void gpcc_cood_ObjectRECORD_wicb_TestsF::CreateUUT_F(void)
{
  spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (F)",
                                         11U,
                                         &data,
                                         sizeof(Data),
                                         &mutex,
                                         siDescr_F,
                                         std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                         std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                         std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

typedef gpcc_cood_ObjectRECORD_wicb_TestsF gpcc_cood_ObjectRECORD_wicb_DeathTestsF;

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CreateAndDestroy_OK)
{
  // This tests different VALID variations of RECORD objects

  EXPECT_NO_THROW(CreateUUT_A());
  EXPECT_NO_THROW(CreateUUT_B());
  EXPECT_NO_THROW(CreateUUT_C());
  EXPECT_NO_THROW(CreateUUT_D(false));
  EXPECT_NO_THROW(CreateUUT_D(true));
  EXPECT_NO_THROW(CreateUUT_E());
  EXPECT_NO_THROW(CreateUUT_F());
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Constructor_NOK)
{
  // This tests INVALID variations of RECORD objects and invalid arguments passed to UUT's constructor

  // Local array of subindex descriptors.
  // This is filled with a copy of a valid array (i.e. siDescr_A) and single fields are modified.
  // Using this requires that the UUT is released before siDescr is manipulated.
  ObjectRECORD_wicb::SubIdxDescr siDescr[11];

  // _pStruct nullptr
  // --------------------------------------------------------------------------
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           nullptr,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr_A,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);


  // struct's native size too large
  // --------------------------------------------------------------------------
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           65537UL,
                                           &mutex,
                                           siDescr_A,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);

  // empty subindex with invalid description (not all fields zero / nullptr)
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_E, sizeof(siDescr));
  siDescr[3].name = "";
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (E)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_E, sizeof(siDescr));
  siDescr[3].attributes = Object::attr_ACCESS_WR;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (E)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_E, sizeof(siDescr));
  siDescr[3].byteOffset = 1U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (E)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_E, sizeof(siDescr));
  siDescr[3].bitOffset = 1U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (E)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // gap subindex with invalid description
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_B, sizeof(siDescr));
  siDescr[3].name = nullptr;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (B)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_B, sizeof(siDescr));
  siDescr[3].attributes = 0;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (B)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_B, sizeof(siDescr));
  siDescr[3].byteOffset = 1U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (B)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_B, sizeof(siDescr));
  siDescr[3].bitOffset = 1U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (B)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // adjacent gap subindices
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_B, sizeof(siDescr));
  siDescr[4].name       = "Align2";
  siDescr[4].type       = DataType::null;
  siDescr[4].attributes = Object::attr_ACCESS_RW;
  siDescr[4].nElements  = 8U;
  siDescr[4].byteOffset = 0U;
  siDescr[4].bitOffset  = 0U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (B)",
                                           12U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex without name
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[1].name = nullptr;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with unsupported data type
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[1].type = DataType::pdo_mapping;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , DataTypeNotSupportedError);
  spUUT.reset();

  // normal subindex without at least one R/W permission set
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[7].attributes = 0;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with array data type and invalid nElements
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[8].nElements = 0;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with non-array data type and invalid nElements
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[7].nElements = 0;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[7].nElements = 2;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with bit-based data type and invalid bit offset
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[4].bitOffset = 8U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with non-bit-based data type and invalid bit offset
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[1].bitOffset = 1U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with bit-based data refers to bits outside the native struct referenced by pStruct
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[5].byteOffset = sizeof(Data) - 1U; // SI5 is BIT2
  siDescr[5].bitOffset  = 7U;
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // normal subindex with non-bit-based data refers to bits outside the native struct referenced by pStruct
  // --------------------------------------------------------------------------
  memcpy(siDescr, siDescr_A, sizeof(siDescr));
  siDescr[8].byteOffset = sizeof(Data) - 7U; // SI8 is VISIBLE_STRING with 8 bytes length
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           siDescr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::invalid_argument);
  spUUT.reset();

  // pMutex nullptr, though write access allowed
  // --------------------------------------------------------------------------
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           nullptr,
                                           siDescr_A,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::logic_error);

  // pSIDescr nullptr
  // --------------------------------------------------------------------------
  EXPECT_THROW(
    spUUT = std::make_unique<ObjectRECORD_wicb>("Testobject (A)",
                                           11U,
                                           &data,
                                           sizeof(Data),
                                           &mutex,
                                           nullptr,
                                           std::bind(&IObjectNotifiableMock::OnBeforeRead, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                                           std::bind(&IObjectNotifiableMock::OnBeforeWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5),
                                           std::bind(&IObjectNotifiableMock::OnAfterWrite, &cbm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    , std::logic_error);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CheckLockData)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  if (mutex.TryLock())
  {
    ADD_FAILURE() << "Mutex protecting the data has not been locked by ObjectRECORD_wicb::LockData()";
    mutex.Unlock();
  }
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CheckMetaData_withoutLock)
{
  CreateUUT_C();

  EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Record);
  EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::domain);
  EXPECT_EQ(spUUT->GetObjectName(),            "Testobject (C)");

  EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     12U);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RD);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(0),           "Number of subindices");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(1),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(1),       DataType::boolean);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(1),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(1),        1U);
  EXPECT_EQ(spUUT->GetSubIdxName(1),           "Data Bool");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(1), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(1), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(2),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(2),       DataType::integer8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(2),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(2),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(2),           "Data i8");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(2), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(2), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(3),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(3),       DataType::unsigned8);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(3),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(3),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(3),           "Data ui8");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(3), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(3), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(4),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(4),       DataType::unsigned32);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(4),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(4),        32U);
  EXPECT_EQ(spUUT->GetSubIdxName(4),           "Data ui32a");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(5),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(5),       DataType::bit1);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(5),     Object::attr_ACCESS_WR);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(5),        1U);
  EXPECT_EQ(spUUT->GetSubIdxName(5),           "Bit 0");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(5), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(5), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(6),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(6),       DataType::bit2);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(6),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(6),        2U);
  EXPECT_EQ(spUUT->GetSubIdxName(6),           "Bit 7..8");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(6), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(6), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(7),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(7),       DataType::bit1);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(7),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(7),        1U);
  EXPECT_EQ(spUUT->GetSubIdxName(7),           "Bit 1");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(7), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(7), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(8),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(8),       DataType::bit4);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(8),     Object::attr_ACCESS_RD);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(8),        4U);
  EXPECT_EQ(spUUT->GetSubIdxName(8),           "Bit 28..31");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(8), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(8), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(9),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(9),       DataType::visible_string);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(9),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(9),        8U * 8U);
  EXPECT_EQ(spUUT->GetSubIdxName(9),           "Text");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(9), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(9), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(10),        false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(10),      DataType::unsigned32);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(10),    Object::attr_ACCESS_RD);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(10),       32U);
  EXPECT_EQ(spUUT->GetSubIdxName(10),          "Data ui32b");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(10), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(10), std::logic_error);

  EXPECT_EQ(spUUT->IsSubIndexEmpty(11),        false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(11),      DataType::octet_string);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(11),    Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(11),       32U);
  EXPECT_EQ(spUUT->GetSubIdxName(11),          "Octet str");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(11), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(11), std::logic_error);

  CreateUUT_B(); // SI 4 is a gap

  EXPECT_EQ(spUUT->IsSubIndexEmpty(4),         false);
  EXPECT_EQ(spUUT->GetSubIdxDataType(4),       DataType::null);
  EXPECT_EQ(spUUT->GetSubIdxAttributes(4),     Object::attr_ACCESS_RW);
  EXPECT_EQ(spUUT->GetSubIdxMaxSize(4),        8U);
  EXPECT_EQ(spUUT->GetSubIdxName(4),           "Align");

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);

  CreateUUT_E(); // SI 4 is empty

  EXPECT_EQ(spUUT->IsSubIndexEmpty(4), true);
  EXPECT_THROW((void)spUUT->GetSubIdxDataType(4), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxAttributes(4), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxMaxSize(4), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxName(4), SubindexNotExistingError);

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CheckMetaData_withLock)
{
  EXPECT_CALL(cbm, OnBeforeRead(_, _, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  CreateUUT_C();

  {
    auto mutexLocker(spUUT->LockData());

    EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Record);
    EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::domain);
    EXPECT_EQ(spUUT->GetObjectName(),            "Testobject (C)");

    EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     12U);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RD);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(0),           "Number of subindices");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(1),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(1),       DataType::boolean);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(1),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(1),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(1),           "Data Bool");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(1), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(1), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(2),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(2),       DataType::integer8);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(2),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(2),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(2),           "Data i8");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(2), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(2), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(3),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(3),       DataType::unsigned8);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(3),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(3),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(3),           "Data ui8");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(3), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(3), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(4),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(4),       DataType::unsigned32);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(4),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(4),        32U);
    EXPECT_EQ(spUUT->GetSubIdxName(4),           "Data ui32a");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(5),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(5),       DataType::bit1);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(5),     Object::attr_ACCESS_WR);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(5),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(5),           "Bit 0");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(5), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(5), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(6),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(6),       DataType::bit2);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(6),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(6),        2U);
    EXPECT_EQ(spUUT->GetSubIdxName(6),           "Bit 7..8");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(6), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(6), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(7),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(7),       DataType::bit1);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(7),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(7),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(7),           "Bit 1");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(7), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(7), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(8),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(8),       DataType::bit4);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(8),     Object::attr_ACCESS_RD);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(8),        4U);
    EXPECT_EQ(spUUT->GetSubIdxName(8),           "Bit 28..31");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(8), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(8), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(9),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(9),       DataType::visible_string);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(9),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(9),        8U * 8U);
    EXPECT_EQ(spUUT->GetSubIdxName(9),           "Text");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(9), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(9), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(10),        false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(10),      DataType::unsigned32);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(10),    Object::attr_ACCESS_RD);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(10),       32U);
    EXPECT_EQ(spUUT->GetSubIdxName(10),          "Data ui32b");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(10), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(10), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(11),        false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(11),      DataType::octet_string);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(11),    Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(11),       32U);
    EXPECT_EQ(spUUT->GetSubIdxName(11),          "Octet str");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(11), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(11), std::logic_error);

    EXPECT_EQ(spUUT->GetObjectStreamSize(false), 25U * 8U);
    EXPECT_EQ(spUUT->GetObjectStreamSize(true),  26U * 8U);
    EXPECT_EQ(spUUT->GetNbOfSubIndices(),        12U);

    EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(1),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(2),     8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(3),     8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(4),     32U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(5),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(6),     2U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(7),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(8),     4U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(9),     6U * 8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(10),    32U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(11),    32U);
  }

  CreateUUT_B(); // SI 4 is a gap

  {
    auto mutexLocker(spUUT->LockData());

    EXPECT_EQ(spUUT->IsSubIndexEmpty(4),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(4),       DataType::null);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(4),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(4),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(4),           "Align");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);
  }

  CreateUUT_E(); // SI 4 is empty

  {
    auto mutexLocker(spUUT->LockData());

    EXPECT_EQ(spUUT->IsSubIndexEmpty(4), true);
    EXPECT_THROW((void)spUUT->GetSubIdxDataType(4), SubindexNotExistingError);
    EXPECT_THROW((void)spUUT->GetSubIdxAttributes(4), SubindexNotExistingError);
    EXPECT_THROW((void)spUUT->GetSubIdxMaxSize(4), SubindexNotExistingError);
    EXPECT_THROW((void)spUUT->GetSubIdxName(4), SubindexNotExistingError);

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);
  }
}


TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CheckMetaData_withLock_withDataTypeMapping)
{
  EXPECT_CALL(cbm, OnBeforeRead(_, _, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  CreateUUT_F();

  {
    auto mutexLocker(spUUT->LockData());

    EXPECT_EQ(spUUT->GetObjectCode(),            Object::ObjectCode::Record);
    EXPECT_EQ(spUUT->GetObjectDataType(),        DataType::domain);
    EXPECT_EQ(spUUT->GetObjectName(),            "Testobject (F)");

    EXPECT_EQ(spUUT->GetMaxNbOfSubindices(),     12U);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(0),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(0),       DataType::unsigned8);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(0),     Object::attr_ACCESS_RD);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(0),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(0),           "Number of subindices");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(0), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(0), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(1),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(1),       DataType::boolean);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(1),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(1),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(1),           "Data Bool");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(1), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(1), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(2),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(2),       DataType::integer8);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(2),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(2),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(2),           "Data i8");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(2), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(2), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(3),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(3),       DataType::unsigned8);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(3),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(3),        8U);
    EXPECT_EQ(spUUT->GetSubIdxName(3),           "Data ui8");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(3), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(3), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(4),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(4),       DataType::unsigned32);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(4),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(4),        32U);
    EXPECT_EQ(spUUT->GetSubIdxName(4),           "Data ui32a");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(4), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(4), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(5),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(5),       DataType::boolean);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(5),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(5),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(5),           "Bool_Bit0");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(5), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(5), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(6),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(6),       DataType::boolean);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(6),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(6),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(6),           "Bool_Bit1");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(6), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(6), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(7),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(7),       DataType::boolean);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(7),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(7),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(7),           "Bool_Bit2");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(7), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(7), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(8),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(8),       DataType::boolean);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(8),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(8),        1U);
    EXPECT_EQ(spUUT->GetSubIdxName(8),           "Bool_Bit3");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(8), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(8), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(9),         false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(9),       DataType::visible_string);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(9),     Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(9),        8U * 8U);
    EXPECT_EQ(spUUT->GetSubIdxName(9),           "Text");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(9), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(9), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(10),        false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(10),      DataType::unsigned32);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(10),    Object::attr_ACCESS_WR_PREOP | Object::attr_ACCESS_RD);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(10),       32U);
    EXPECT_EQ(spUUT->GetSubIdxName(10),          "Data ui32b");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(10), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(10), std::logic_error);

    EXPECT_EQ(spUUT->IsSubIndexEmpty(11),        false);
    EXPECT_EQ(spUUT->GetSubIdxDataType(11),      DataType::octet_string);
    EXPECT_EQ(spUUT->GetSubIdxAttributes(11),    Object::attr_ACCESS_RW);
    EXPECT_EQ(spUUT->GetSubIdxMaxSize(11),       32U);
    EXPECT_EQ(spUUT->GetSubIdxName(11),          "Octet str");

    EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(11), 0U);
    EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(11), std::logic_error);


    EXPECT_EQ(spUUT->GetObjectStreamSize(false), 25U * 8U);
    EXPECT_EQ(spUUT->GetObjectStreamSize(true),  26U * 8U);
    EXPECT_EQ(spUUT->GetNbOfSubIndices(),        12U);

    EXPECT_EQ(spUUT->GetSubIdxActualSize(0),     8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(1),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(2),     8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(3),     8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(4),     32U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(5),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(6),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(7),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(8),     1U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(9),     6U * 8U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(10),    32U);
    EXPECT_EQ(spUUT->GetSubIdxActualSize(11),    32U);
  }
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CheckMetaData_InvalidSubindex)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  // methods which do not require the lock:
  EXPECT_THROW((void)spUUT->IsSubIndexEmpty(12U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxDataType(12U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxAttributes(12U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxMaxSize(12U), SubindexNotExistingError);
  EXPECT_THROW((void)spUUT->GetSubIdxName(12U), SubindexNotExistingError);

  EXPECT_EQ(spUUT->GetAppSpecificMetaDataSize(12U), 0U);
  EXPECT_THROW((void)spUUT->GetAppSpecificMetaData(12U), std::logic_error);

  // methods which REQUIRE the lock:
  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(12U), SubindexNotExistingError);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, GetSubIdxActualSize_BeforeReadCbReportsOutOfMemory)
{
  CreateUUT_C();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 9U, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::OutOfMemory));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(9), std::bad_alloc);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, GetSubIdxActualSize_BeforeReadCbReportsError)
{
  CreateUUT_C();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 9U, false, true)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralIntIncompatibility));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(9), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, GetSubIdxActualSize_BeforeReadCbThrows)
{
  CreateUUT_C();

  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 9U, false, true)).Times(1).WillRepeatedly(Throw(std::runtime_error("Intentionally thrown exception")));

  auto locker(spUUT->LockData());

  EXPECT_THROW((void)spUUT->GetSubIdxActualSize(9), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_AllSIs_A)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).Times(12).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(2U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(3U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(8U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(10U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(11U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 22U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[3], 239U);
  EXPECT_EQ(writeBuffer[4], 0xEFU);
  EXPECT_EQ(writeBuffer[5], 0xBEU);
  EXPECT_EQ(writeBuffer[6], 0xADU);
  EXPECT_EQ(writeBuffer[7], 0xDEU);
  EXPECT_EQ(writeBuffer[8], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[9], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[13], 0U);
  EXPECT_EQ(writeBuffer[14], 0x78U);
  EXPECT_EQ(writeBuffer[15], 0x56U);
  EXPECT_EQ(writeBuffer[16], 0x34U);
  EXPECT_EQ(writeBuffer[17], 0x12U);
  EXPECT_EQ(writeBuffer[18], 0xF5U);
  EXPECT_EQ(writeBuffer[19], 0xDEU);
  EXPECT_EQ(writeBuffer[20], 0xB2U);
  EXPECT_EQ(writeBuffer[21], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_AllSIs_B)
{
  // difference to A: SI4 is for alignment

  CreateUUT_B();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).Times(12).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(2U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(3U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(8U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(10U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(11U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(12U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 23U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 12U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[3], 239U);
  EXPECT_EQ(writeBuffer[4], 0x00U); // align
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0x78U);
  EXPECT_EQ(writeBuffer[16], 0x56U);
  EXPECT_EQ(writeBuffer[17], 0x34U);
  EXPECT_EQ(writeBuffer[18], 0x12U);
  EXPECT_EQ(writeBuffer[19], 0xF5U);
  EXPECT_EQ(writeBuffer[20], 0xDEU);
  EXPECT_EQ(writeBuffer[21], 0xB2U);
  EXPECT_EQ(writeBuffer[22], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_AllSIs_C)
{
  // difference to A: SI5 is write-only

  CreateUUT_C();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).Times(11).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(2U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(3U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);
  EXPECT_EQ(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(8U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(10U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(11U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 22U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[3], 239U);
  EXPECT_EQ(writeBuffer[4], 0xEFU);
  EXPECT_EQ(writeBuffer[5], 0xBEU);
  EXPECT_EQ(writeBuffer[6], 0xADU);
  EXPECT_EQ(writeBuffer[7], 0xDEU);
  EXPECT_EQ(writeBuffer[8], 0x5FU); // Bits 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[9], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[13], 0U);
  EXPECT_EQ(writeBuffer[14], 0x78U);
  EXPECT_EQ(writeBuffer[15], 0x56U);
  EXPECT_EQ(writeBuffer[16], 0x34U);
  EXPECT_EQ(writeBuffer[17], 0x12U);
  EXPECT_EQ(writeBuffer[18], 0xF5U);
  EXPECT_EQ(writeBuffer[19], 0xDEU);
  EXPECT_EQ(writeBuffer[20], 0xB2U);
  EXPECT_EQ(writeBuffer[21], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_AllSIs_E)
{
  // difference to A: SI4 is not existig

  CreateUUT_E();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).Times(12).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(2U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(3U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::SubindexDoesNotExist);
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(8U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(10U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(11U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(12U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 22U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 12U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[3], 239U);
  EXPECT_EQ(writeBuffer[4], 0xEFU);
  EXPECT_EQ(writeBuffer[5], 0xBEU);
  EXPECT_EQ(writeBuffer[6], 0xADU);
  EXPECT_EQ(writeBuffer[7], 0xDEU);
  EXPECT_EQ(writeBuffer[8], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[9],  static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[13], 0U);
  EXPECT_EQ(writeBuffer[14], 0x78U);
  EXPECT_EQ(writeBuffer[15], 0x56U);
  EXPECT_EQ(writeBuffer[16], 0x34U);
  EXPECT_EQ(writeBuffer[17], 0x12U);
  EXPECT_EQ(writeBuffer[18], 0xF5U);
  EXPECT_EQ(writeBuffer[19], 0xDEU);
  EXPECT_EQ(writeBuffer[20], 0xB2U);
  EXPECT_EQ(writeBuffer[21], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_AllSIs_F)
{
  // difference to A: Data type boolean_native_bit1 is present

  CreateUUT_F();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), _, false, false)).Times(12).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x0AU;    // bit 1, 3
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(2U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(3U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(5U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(7U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(8U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(10U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Read(11U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 22U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[3], 239U);
  EXPECT_EQ(writeBuffer[4], 0xEFU);
  EXPECT_EQ(writeBuffer[5], 0xBEU);
  EXPECT_EQ(writeBuffer[6], 0xADU);
  EXPECT_EQ(writeBuffer[7], 0xDEU);
  EXPECT_EQ(writeBuffer[8], 0x0AU);
  EXPECT_EQ(writeBuffer[9],  static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[13], 0U);
  EXPECT_EQ(writeBuffer[14], 0x78U);
  EXPECT_EQ(writeBuffer[15], 0x56U);
  EXPECT_EQ(writeBuffer[16], 0x34U);
  EXPECT_EQ(writeBuffer[17], 0x12U);
  EXPECT_EQ(writeBuffer[18], 0xF5U);
  EXPECT_EQ(writeBuffer[19], 0xDEU);
  EXPECT_EQ(writeBuffer[20], 0xB2U);
  EXPECT_EQ(writeBuffer[21], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_VisibleString_empty)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 9U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_visiblestring[0] = 0x00U;

  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 1U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x00U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_VisibleString_half)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 9U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00U;

  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 5U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[1], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[4], 0x00U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_VisibleString_full)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 9U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 'f';
  data.data_visiblestring[5] = 'u';
  data.data_visiblestring[6] = 'l';
  data.data_visiblestring[7] = 'l';

  EXPECT_EQ(spUUT->Read(9U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 8U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[1], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[4], static_cast<uint8_t>('f'));
  EXPECT_EQ(writeBuffer[5], static_cast<uint8_t>('u'));
  EXPECT_EQ(writeBuffer[6], static_cast<uint8_t>('l'));
  EXPECT_EQ(writeBuffer[7], static_cast<uint8_t>('l'));
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_SI_not_existing)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(12U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::SubindexDoesNotExist);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 0U) << "Unexpected write to ISW";
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_Subindex_empty)
{
  CreateUUT_E();

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::SubindexDoesNotExist);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 0U) << "Unexpected write to ISW";
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_Subindex_gap)
{
  CreateUUT_B();

  auto locker(spUUT->LockData());

  // fill data with 0xFF
  memset(&data, 0xFFU, sizeof(data));

  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 1U);
  writeBufferWriter.Close();

  // zeros must have been read from the gap, though the whole native struct is full of 0xFF
  EXPECT_EQ(writeBuffer[0], 0x00U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_InsufficientPermission)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  // SI 0
  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_WR_PREOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);

  // other SI
  EXPECT_EQ(spUUT->Read(1U, Object::attr_ACCESS_WR_PREOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 0U) << "Unexpected write to ISW";
  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_BeforeReadCallbackRejects_SI0)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralError));

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralError);

  // check: stream writer has not been modified
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_BeforeReadCallbackRejects_OtherSI)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 6U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralError));

  auto locker(spUUT->LockData());

  EXPECT_EQ(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralError);

  // check: stream writer has not been modified
  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_BeforeReadCallbackThrows_SI0)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  ASSERT_THROW(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);

  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));
  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_BeforeReadCallbackThrows_OtherSI)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 6U, false, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  ASSERT_THROW(spUUT->Read(6U, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);

  EXPECT_EQ(writeBufferWriter.RemainingCapacity(), sizeof(writeBuffer));
  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_StreamWriterFullyUsed_SI0)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // replace the stream writer to "writeBuffer" with our own
  writeBufferWriter.Close();
  MemStreamWriter msw(writeBuffer, 1U, MemStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, msw), SDOAbortCode::OK);

  EXPECT_EQ(msw.RemainingCapacity(), 0U);
  EXPECT_EQ(msw.GetState(), IStreamWriter::States::full);
  msw.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_StreamWriterFullyUsed_OtherSI)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 4U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_ui32a = 0x173541BCU;
  // replace the stream writer to "writeBuffer" with our own
  writeBufferWriter.Close();
  MemStreamWriter msw(writeBuffer, 4U, MemStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, msw), SDOAbortCode::OK);

  EXPECT_EQ(msw.RemainingCapacity(), 0U);
  EXPECT_EQ(msw.GetState(), IStreamWriter::States::full);
  msw.Close();

  EXPECT_EQ(writeBuffer[0], 0xBCU);
  EXPECT_EQ(writeBuffer[1], 0x41U);
  EXPECT_EQ(writeBuffer[2], 0x35U);
  EXPECT_EQ(writeBuffer[3], 0x17U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_StreamWriterHasNotEnoughSpace_SI0)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // replace the stream writer to "writeBuffer" with our own
  writeBufferWriter.Close();
  MemStreamWriter msw(writeBuffer, 0U, MemStreamWriter::Endian::Little);

  ASSERT_THROW(spUUT->Read(0U, Object::attr_ACCESS_RD_PREOP, msw), FullError);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Read_StreamWriterHasNotEnoughSpace_OtherSI)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 4U, false, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  // replace the stream writer to "writeBuffer" with our own
  writeBufferWriter.Close();
  MemStreamWriter msw(writeBuffer, 3U, MemStreamWriter::Endian::Little);

  ASSERT_THROW(spUUT->Read(4U, Object::attr_ACCESS_RD_PREOP, msw), FullError);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_AllSIs_A)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), _, false, 0U, _)).Times(11).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), _, false)).Times(11);
  auto locker(spUUT->LockData());

  readBuffer[0]  = 0x01U;
  readBuffer[1]  = static_cast<uint8_t>(-25);
  readBuffer[2]  = 239U;
  readBuffer[3]  = 0xEFU;
  readBuffer[4]  = 0xBEU;
  readBuffer[5]  = 0xADU;
  readBuffer[6]  = 0xDEU;
  readBuffer[7]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[8]  = static_cast<uint8_t>('T');
  readBuffer[9] = static_cast<uint8_t>('e');
  readBuffer[10] = static_cast<uint8_t>('s');
  readBuffer[11] = static_cast<uint8_t>('t');
  readBuffer[12] = 0U;
  readBuffer[13] = 0x78U;
  readBuffer[14] = 0x56U;
  readBuffer[15] = 0x34U;
  readBuffer[16] = 0x12U;
  readBuffer[17] = 0xF5U;
  readBuffer[18] = 0xDEU;
  readBuffer[19] = 0xB2U;
  readBuffer[20] = 0x87U;

  auto ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(2U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(3U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(5U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(6U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(7U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(8U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(5U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(10U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(11U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0xB0U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_AllSIs_F)
{
  CreateUUT_F();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), _, false, 0U, _)).Times(11).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), _, false)).Times(11);
  auto locker(spUUT->LockData());

  readBuffer[0]  = 0x01U;
  readBuffer[1]  = static_cast<uint8_t>(-25);
  readBuffer[2]  = 239U;
  readBuffer[3]  = 0xEFU;
  readBuffer[4]  = 0xBEU;
  readBuffer[5]  = 0xADU;
  readBuffer[6]  = 0xDEU;
  readBuffer[7]  = 0x0AU;
  readBuffer[8]  = static_cast<uint8_t>('T');
  readBuffer[9] = static_cast<uint8_t>('e');
  readBuffer[10] = static_cast<uint8_t>('s');
  readBuffer[11] = static_cast<uint8_t>('t');
  readBuffer[12] = 0U;
  readBuffer[13] = 0x78U;
  readBuffer[14] = 0x56U;
  readBuffer[15] = 0x34U;
  readBuffer[16] = 0x12U;
  readBuffer[17] = 0xF5U;
  readBuffer[18] = 0xDEU;
  readBuffer[19] = 0xB2U;
  readBuffer[20] = 0x87U;

  auto ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(1U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(2U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(3U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(5U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(6U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(7U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);
  EXPECT_EQ(spUUT->Write(8U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(5U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(10U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(11U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x0AU);
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_VISIBLESTRING_empty_A)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 9, false, 0U, _)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 9, false)).Times(1);
  auto locker(spUUT->LockData());

  readBuffer[0] = 0x00;

  auto ssr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);


  EXPECT_EQ(data.data_visiblestring[0], 0x00);
  EXPECT_EQ(data.data_visiblestring[1], 0x00);
  EXPECT_EQ(data.data_visiblestring[2], 0x00);
  EXPECT_EQ(data.data_visiblestring[3], 0x00);
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_VISIBLESTRING_empty_B)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 9, false, 0U, _)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 9, false)).Times(1);
  auto locker(spUUT->LockData());

  readBuffer[0] = 0x00;

  auto ssr = readBufferReader.SubStream(0U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);

  EXPECT_EQ(data.data_visiblestring[0], 0x00);
  EXPECT_EQ(data.data_visiblestring[1], 0x00);
  EXPECT_EQ(data.data_visiblestring[2], 0x00);
  EXPECT_EQ(data.data_visiblestring[3], 0x00);
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_VISIBLESTRING_half_A)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 9, false, 0U, _)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 9, false)).Times(1);
  auto locker(spUUT->LockData());

  readBuffer[0] = static_cast<uint8_t>('A');
  readBuffer[1] = static_cast<uint8_t>('B');
  readBuffer[2] = static_cast<uint8_t>('C');
  readBuffer[3] = static_cast<uint8_t>('D');
  readBuffer[4] =  0x00U;

  auto ssr = readBufferReader.SubStream(5U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);


  EXPECT_EQ(data.data_visiblestring[0], 'A');
  EXPECT_EQ(data.data_visiblestring[1], 'B');
  EXPECT_EQ(data.data_visiblestring[2], 'C');
  EXPECT_EQ(data.data_visiblestring[3], 'D');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_VISIBLESTRING_half_B)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 9, false, 0U, _)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 9, false)).Times(1);
  auto locker(spUUT->LockData());

  readBuffer[0] = static_cast<uint8_t>('A');
  readBuffer[1] = static_cast<uint8_t>('B');
  readBuffer[2] = static_cast<uint8_t>('C');
  readBuffer[3] = static_cast<uint8_t>('D');
  readBuffer[4] =  0x00U;

  auto ssr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);


  EXPECT_EQ(data.data_visiblestring[0], 'A');
  EXPECT_EQ(data.data_visiblestring[1], 'B');
  EXPECT_EQ(data.data_visiblestring[2], 'C');
  EXPECT_EQ(data.data_visiblestring[3], 'D');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_VISIBLESTRING_full)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 9, false, 0U, _)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 9, false)).Times(1);
  auto locker(spUUT->LockData());

  readBuffer[0] = static_cast<uint8_t>('A');
  readBuffer[1] = static_cast<uint8_t>('B');
  readBuffer[2] = static_cast<uint8_t>('C');
  readBuffer[3] = static_cast<uint8_t>('D');
  readBuffer[4] = static_cast<uint8_t>('E');
  readBuffer[5] = static_cast<uint8_t>('F');
  readBuffer[6] = static_cast<uint8_t>('G');
  readBuffer[7] = static_cast<uint8_t>('H');

  auto ssr = readBufferReader.SubStream(8U);
  EXPECT_EQ(spUUT->Write(9U, Object::attr_ACCESS_WR, ssr), SDOAbortCode::OK);


  EXPECT_EQ(data.data_visiblestring[0], 'A');
  EXPECT_EQ(data.data_visiblestring[1], 'B');
  EXPECT_EQ(data.data_visiblestring[2], 'C');
  EXPECT_EQ(data.data_visiblestring[3], 'D');
  EXPECT_EQ(data.data_visiblestring[4], 'E');
  EXPECT_EQ(data.data_visiblestring[5], 'F');
  EXPECT_EQ(data.data_visiblestring[6], 'G');
  EXPECT_EQ(data.data_visiblestring[7], 'H');
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_SubindexNotExisting)
{
  CreateUUT_A();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 87U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(12U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::SubindexDoesNotExist);

  EXPECT_EQ(sr.RemainingBytes(), 1U) << "Data has been read from the StreamReader. This was not expected";
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_SI0)
{
  CreateUUT_A();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 11U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(0U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::AttemptToWriteRdOnlyObject);

  EXPECT_EQ(sr.RemainingBytes(), 1U) << "Data has been read from the StreamReader. This was not expected";
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_SI_empty)
{
  CreateUUT_E();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 11U;

  auto sr = readBufferReader.SubStream(1U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::SubindexDoesNotExist);

  EXPECT_EQ(sr.RemainingBytes(), 1U) << "Data has been read from the StreamReader. This was not expected";
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_InsufficientPermission)
{
  CreateUUT_C();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x0AU;
  readBuffer[1] = 0x0BU;
  readBuffer[2] = 0x0CU;
  readBuffer[3] = 0x0DU;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(10U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::AttemptToWriteRdOnlyObject);

  EXPECT_EQ(sr.RemainingBytes(), 4U) << "Data has been read from the StreamReader. This was not expected";
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_Gap_StreamReaderEmpty)
{
  CreateUUT_B(); // SI4 is a gap

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  auto sr = gpcc::Stream::MemStreamReader(nullptr, 0U, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooSmall);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_Gap_TooMuchData)
{
  CreateUUT_B(); // SI4 is a gap

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooLong);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_Gap_StreamReaderThrows)
{
  CreateUUT_B(); // SI4 is a gap

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  auto sr = readBufferReader.SubStream(2U);
  ASSERT_THROW((void)sr.Read_uint32(), std::exception);
  ASSERT_EQ(sr.GetState(), IStreamReader::States::error);
  EXPECT_THROW(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), std::exception);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_StreamReaderEmpty)
{
  CreateUUT_A();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  auto sr = gpcc::Stream::MemStreamReader(nullptr, 0U, gpcc::Stream::IStreamReader::Endian::Little);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooSmall);

  EXPECT_EQ(data.data_ui32a, 0U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_NotEnoughData)
{
  CreateUUT_A();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x12U;
  readBuffer[1] = 0x34U;
  readBuffer[2] = 0x56U;
  readBuffer[3] = 0x78U;

  auto sr = readBufferReader.SubStream(2U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooSmall);

  EXPECT_EQ(data.data_ui32a, 0U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_TooMuchData)
{
  CreateUUT_A();

  // prepare mock
  // - no call expected -

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x12U;
  readBuffer[1] = 0x34U;
  readBuffer[2] = 0x56U;
  readBuffer[3] = 0x78U;
  readBuffer[4] = 0xABU;

  auto sr = readBufferReader.SubStream(5U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::DataTypeMismatchTooLong);

  EXPECT_EQ(data.data_ui32a, 0U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_BeforeWriteCallbackRejects)
{
  CreateUUT_A();

  // define variable for preview value and define a lambda to catch it
  uint32_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint32_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 4U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Return(SDOAbortCode::GeneralError)));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x12U;
  readBuffer[1] = 0x34U;
  readBuffer[2] = 0x56U;
  readBuffer[3] = 0x78U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_EQ(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), SDOAbortCode::GeneralError);
  EXPECT_EQ(pv1, 0x78563412UL);
  EXPECT_EQ(data.data_ui32a, 0U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, Write_BeforeWriteCallbackThrows)
{
  CreateUUT_A();

  // define variable for preview value and define a lambda to catch it
  uint32_t pv1 = 0;

  auto recorder1 = [&pv1](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; pv1 = *static_cast<uint32_t const*>(pData); };

  // prepare mock
  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 4U, false, 0, _)).WillOnce(DoAll(Invoke(recorder1), Throw(std::runtime_error("Test"))));

  // stimulus

  auto locker(spUUT->LockData());

  readBuffer[0] = 0x12U;
  readBuffer[1] = 0x34U;
  readBuffer[2] = 0x56U;
  readBuffer[3] = 0x78U;

  auto sr = readBufferReader.SubStream(4U);
  EXPECT_THROW(spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr), std::runtime_error);
  EXPECT_EQ(pv1, 0x78563412UL);
  EXPECT_EQ(data.data_ui32a, 0U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_DeathTestsF, Write_AfterWriteCallbackThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto test = [&]()
  {
    CreateUUT_A();

    // prepare mock
    {
      InSequence seq;
      EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 4U, false, 0, _)).WillOnce(Return(SDOAbortCode::OK));
      EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 4U, false)).WillOnce(Throw(std::runtime_error("Test")));
    }

    // stimulus

    auto locker(spUUT->LockData());

    readBuffer[0] = 0x12U;
    readBuffer[1] = 0x34U;
    readBuffer[2] = 0x56U;
    readBuffer[3] = 0x78U;

    auto sr = readBufferReader.SubStream(4U);

    // leathal call:
    (void)spUUT->Write(4U, Object::attr_ACCESS_WR_PREOP, sr);
  };

  EXPECT_DEATH(test(), ".*After-write-callback threw.*");
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_A_withSI0_16bit)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_A_withSI0_8bit)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 25U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0x01U);
  EXPECT_EQ(writeBuffer[2], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[3], 239U);
  EXPECT_EQ(writeBuffer[4], 0xEFU);
  EXPECT_EQ(writeBuffer[5], 0xBEU);
  EXPECT_EQ(writeBuffer[6], 0xADU);
  EXPECT_EQ(writeBuffer[7], 0xDEU);
  EXPECT_EQ(writeBuffer[8], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[9], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[13], 0U);
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0x78U);
  EXPECT_EQ(writeBuffer[18], 0x56U);
  EXPECT_EQ(writeBuffer[19], 0x34U);
  EXPECT_EQ(writeBuffer[20], 0x12U);
  EXPECT_EQ(writeBuffer[21], 0xF5U);
  EXPECT_EQ(writeBuffer[22], 0xDEU);
  EXPECT_EQ(writeBuffer[23], 0xB2U);
  EXPECT_EQ(writeBuffer[24], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_A_withoutSI0)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 1U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(false, false, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 24U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 0x01U);
  EXPECT_EQ(writeBuffer[1], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[2], 239U);
  EXPECT_EQ(writeBuffer[3], 0xEFU);
  EXPECT_EQ(writeBuffer[4], 0xBEU);
  EXPECT_EQ(writeBuffer[5], 0xADU);
  EXPECT_EQ(writeBuffer[6], 0xDEU);
  EXPECT_EQ(writeBuffer[7], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[8], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[9], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[12], 0U);
  EXPECT_EQ(writeBuffer[13], 0U);
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0x78U);
  EXPECT_EQ(writeBuffer[17], 0x56U);
  EXPECT_EQ(writeBuffer[18], 0x34U);
  EXPECT_EQ(writeBuffer[19], 0x12U);
  EXPECT_EQ(writeBuffer[20], 0xF5U);
  EXPECT_EQ(writeBuffer[21], 0xDEU);
  EXPECT_EQ(writeBuffer[22], 0xB2U);
  EXPECT_EQ(writeBuffer[23], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_B_withSI0_16bit)
{
  CreateUUT_B();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 27U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 12U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0U); // Align
  EXPECT_EQ(writeBuffer[6], 0xEFU);
  EXPECT_EQ(writeBuffer[7], 0xBEU);
  EXPECT_EQ(writeBuffer[8], 0xADU);
  EXPECT_EQ(writeBuffer[9], 0xDEU);
  EXPECT_EQ(writeBuffer[10], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[14], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0U);
  EXPECT_EQ(writeBuffer[19], 0x78U);
  EXPECT_EQ(writeBuffer[20], 0x56U);
  EXPECT_EQ(writeBuffer[21], 0x34U);
  EXPECT_EQ(writeBuffer[22], 0x12U);
  EXPECT_EQ(writeBuffer[23], 0xF5U);
  EXPECT_EQ(writeBuffer[24], 0xDEU);
  EXPECT_EQ(writeBuffer[25], 0xB2U);
  EXPECT_EQ(writeBuffer[26], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_C_withSI0_16bit)
{
  CreateUUT_C();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x83U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_D_withSI0_16bit)
{
  CreateUUT_D(false);
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_E_withSI0_16bit)
{
  CreateUUT_E();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 12U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_F_withSI0_16bit)
{
  CreateUUT_F();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x0AU;
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0x0AU);
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_InsufficientPermission)
{
  CreateUUT_D(false);

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_SAFEOP, writeBufferWriter), SDOAbortCode::AttemptToReadWrOnlyObject);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 0U);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_BeforeReadCallbackDoesNotAgree)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::GeneralError));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::GeneralError);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 0U);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_BeforeReadCallbackThrows)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Throw(std::runtime_error("Test")));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_THROW(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), std::runtime_error);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 0U);

  writeBufferWriter.Close();
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_NotEnoughSpaceInStreamWriter)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  writeBufferWriter.Close();

  MemStreamWriter msw(writeBuffer, 12U, IStreamWriter::Endian::Little);
  EXPECT_THROW(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, msw), gpcc::Stream::FullError);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteRead_StreamWriterInErrorState)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  writeBufferWriter.Close();

  MemStreamWriter msw(writeBuffer, 2U, IStreamWriter::Endian::Little);
  ASSERT_THROW(msw.Write_uint32(0), gpcc::Stream::FullError);
  ASSERT_EQ(msw.GetState(), IStreamWriter::States::error);

  EXPECT_THROW(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, msw), gpcc::Stream::ErrorStateError);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_A_SI0_16bit)
{
  CreateUUT_A();

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0xB0U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_A_SI0_8bit)
{
  CreateUUT_A();

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0x01U;
  readBuffer[2]  = static_cast<uint8_t>(-25);
  readBuffer[3]  = 239U;
  readBuffer[4]  = 0xEFU;
  readBuffer[5]  = 0xBEU;
  readBuffer[6]  = 0xADU;
  readBuffer[7]  = 0xDEU;
  readBuffer[8]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[9] = static_cast<uint8_t>('T');
  readBuffer[10] = static_cast<uint8_t>('e');
  readBuffer[11] = static_cast<uint8_t>('s');
  readBuffer[12] = static_cast<uint8_t>('t');
  readBuffer[13] = 0U;
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0x78U;
  readBuffer[18] = 0x56U;
  readBuffer[19] = 0x34U;
  readBuffer[20] = 0x12U;
  readBuffer[21] = 0xF5U;
  readBuffer[22] = 0xDEU;
  readBuffer[23] = 0xB2U;
  readBuffer[24] = 0x87U;

  auto ssr = readBufferReader.SubStream(25U);
  EXPECT_EQ(spUUT->CompleteWrite(true, false, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0xB0U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_A_without_SI0)
{
  CreateUUT_A();

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 1U, true, 0U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 1U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 0x01U;
  readBuffer[1]  = static_cast<uint8_t>(-25);
  readBuffer[2]  = 239U;
  readBuffer[3]  = 0xEFU;
  readBuffer[4]  = 0xBEU;
  readBuffer[5]  = 0xADU;
  readBuffer[6]  = 0xDEU;
  readBuffer[7]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[8] = static_cast<uint8_t>('T');
  readBuffer[9] = static_cast<uint8_t>('e');
  readBuffer[10] = static_cast<uint8_t>('s');
  readBuffer[11] = static_cast<uint8_t>('t');
  readBuffer[12] = 0U;
  readBuffer[13] = 0U;
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0x78U;
  readBuffer[17] = 0x56U;
  readBuffer[18] = 0x34U;
  readBuffer[19] = 0x12U;
  readBuffer[20] = 0xF5U;
  readBuffer[21] = 0xDEU;
  readBuffer[22] = 0xB2U;
  readBuffer[23] = 0x87U;

  auto ssr = readBufferReader.SubStream(24U);
  EXPECT_EQ(spUUT->CompleteWrite(false, false, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0xB0U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_B_SI0_16bit_gap)
{
  CreateUUT_B(); // SI4 is 8 bit gap

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 12U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 12U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0U;
  readBuffer[6]  = 0xEFU;
  readBuffer[7]  = 0xBEU;
  readBuffer[8]  = 0xADU;
  readBuffer[9]  = 0xDEU;
  readBuffer[10]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[11] = static_cast<uint8_t>('T');
  readBuffer[12] = static_cast<uint8_t>('e');
  readBuffer[13] = static_cast<uint8_t>('s');
  readBuffer[14] = static_cast<uint8_t>('t');
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0U;
  readBuffer[19] = 0x78U;
  readBuffer[20] = 0x56U;
  readBuffer[21] = 0x34U;
  readBuffer[22] = 0x12U;
  readBuffer[23] = 0xF5U;
  readBuffer[24] = 0xDEU;
  readBuffer[25] = 0xB2U;
  readBuffer[26] = 0x87U;

  auto ssr = readBufferReader.SubStream(27U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0xB0U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_C_SI0_16bit_WO_RO)
{
  CreateUUT_C(); // SI5 is w/o, SI8 is r/o (bit-based) SI10 is r/o (byte-based)

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0xFFU;
  readBuffer[19] = 0xFFU;
  readBuffer[20] = 0xFFU;
  readBuffer[21] = 0xFFU;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0x00U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0U);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_E_SI0_16bit_emptySI)
{
  CreateUUT_E(); // SI4 is empty

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 12U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 12U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x82U);    // bit 0, 1, 7
  EXPECT_EQ(data.data_bitX[1],          0x01U);    // bit 0
  EXPECT_EQ(data.data_bitX[2],          0x00U);
  EXPECT_EQ(data.data_bitX[3],          0xB0U);    // bit 4..7
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678U);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_F_SI0_16bit_emptySI)
{
  CreateUUT_F();

  // define variable for preview value and define a lambda to catch it
  Data pv;

  auto recorder1 = [&pv](cood::Object const * pObj, uint8_t si, bool ca, uint8_t si0, void const * pData)
  { (void)pObj; (void)si; (void)ca; (void)si0; memcpy(&pv, pData, sizeof(pv)); };

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillRepeatedly(DoAll(Invoke(recorder1), Return(SDOAbortCode::OK)));
  EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1);

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0x05U;
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::OK);

  EXPECT_EQ(data.data_bool,             true);
  EXPECT_EQ(data.data_i8,               -25);
  EXPECT_EQ(data.data_ui8,              239U);
  EXPECT_EQ(data.data_ui32a,            0xDEADBEEFUL);
  EXPECT_EQ(data.data_bitX[0],          0x05U);
  EXPECT_EQ(data.data_visiblestring[0], 'T');
  EXPECT_EQ(data.data_visiblestring[1], 'e');
  EXPECT_EQ(data.data_visiblestring[2], 's');
  EXPECT_EQ(data.data_visiblestring[3], 't');
  EXPECT_EQ(data.data_visiblestring[4], 0x00);
  EXPECT_EQ(data.data_visiblestring[5], 0x00);
  EXPECT_EQ(data.data_visiblestring[6], 0x00);
  EXPECT_EQ(data.data_visiblestring[7], 0x00);
  EXPECT_EQ(data.data_ui32b,            0x12345678UL);
  EXPECT_EQ(data.data_octectstring[0],  0xF5U);
  EXPECT_EQ(data.data_octectstring[1],  0xDEU);
  EXPECT_EQ(data.data_octectstring[2],  0xB2U);
  EXPECT_EQ(data.data_octectstring[3],  0x87U);

  EXPECT_TRUE(data == pv);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_InsufficientPermission)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_SAFEOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::AttemptToWriteRdOnlyObject);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_WriteInvValueToSI0)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  readBuffer[0]  = 12U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::UnsupportedAccessToObject);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_NotEnoughData)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(25U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::DataTypeMismatchTooSmall);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_ERNOB_not_met)
{
  CreateUUT_A();

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::moreThanSeven), SDOAbortCode::DataTypeMismatchTooLong);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_OnBeforeWriteCB_Rejects)
{
  CreateUUT_A();

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillOnce(Return(SDOAbortCode::GeneralError));

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_EQ(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), SDOAbortCode::GeneralError);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, CompleteWrite_OnBeforeWriteCB_Throws)
{
  CreateUUT_A();

  EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillOnce(Throw(std::runtime_error("")));

  auto locker(spUUT->LockData());

  readBuffer[0]  = 11U;
  readBuffer[1]  = 0U;
  readBuffer[2]  = 0x01U;
  readBuffer[3]  = static_cast<uint8_t>(-25);
  readBuffer[4]  = 239U;
  readBuffer[5]  = 0xEFU;
  readBuffer[6]  = 0xBEU;
  readBuffer[7]  = 0xADU;
  readBuffer[8]  = 0xDEU;
  readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
  readBuffer[10] = static_cast<uint8_t>('T');
  readBuffer[11] = static_cast<uint8_t>('e');
  readBuffer[12] = static_cast<uint8_t>('s');
  readBuffer[13] = static_cast<uint8_t>('t');
  readBuffer[14] = 0U;
  readBuffer[15] = 0U;
  readBuffer[16] = 0U;
  readBuffer[17] = 0U;
  readBuffer[18] = 0x78U;
  readBuffer[19] = 0x56U;
  readBuffer[20] = 0x34U;
  readBuffer[21] = 0x12U;
  readBuffer[22] = 0xF5U;
  readBuffer[23] = 0xDEU;
  readBuffer[24] = 0xB2U;
  readBuffer[25] = 0x87U;

  auto ssr = readBufferReader.SubStream(26U);
  EXPECT_THROW(spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_DeathTestsF, CompleteWrite_AfterWriteCallbackThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto test = [&]()
  {
    CreateUUT_A();

    EXPECT_CALL(cbm, OnBeforeWrite(spUUT.get(), 0U, true, 11U, _)).Times(1).WillOnce(Return(SDOAbortCode::OK));
    EXPECT_CALL(cbm, OnAfterWrite(spUUT.get(), 0U, true)).Times(1).WillOnce(Throw(std::runtime_error("Test")));

    auto locker(spUUT->LockData());

    readBuffer[0]  = 11U;
    readBuffer[1]  = 0U;
    readBuffer[2]  = 0x01U;
    readBuffer[3]  = static_cast<uint8_t>(-25);
    readBuffer[4]  = 239U;
    readBuffer[5]  = 0xEFU;
    readBuffer[6]  = 0xBEU;
    readBuffer[7]  = 0xADU;
    readBuffer[8]  = 0xDEU;
    readBuffer[9]  = 0xBEU; // Bits 0, 7..8, 1, 28..31
    readBuffer[10] = static_cast<uint8_t>('T');
    readBuffer[11] = static_cast<uint8_t>('e');
    readBuffer[12] = static_cast<uint8_t>('s');
    readBuffer[13] = static_cast<uint8_t>('t');
    readBuffer[14] = 0U;
    readBuffer[15] = 0U;
    readBuffer[16] = 0U;
    readBuffer[17] = 0U;
    readBuffer[18] = 0x78U;
    readBuffer[19] = 0x56U;
    readBuffer[20] = 0x34U;
    readBuffer[21] = 0x12U;
    readBuffer[22] = 0xF5U;
    readBuffer[23] = 0xDEU;
    readBuffer[24] = 0xB2U;
    readBuffer[25] = 0x87U;

    auto ssr = readBufferReader.SubStream(26U);

    // leathal call:
    (void)spUUT->CompleteWrite(true, true, Object::attr_ACCESS_WR_PREOP, ssr, IStreamReader::RemainingNbOfBits::zero);
  };

  EXPECT_DEATH(test(), ".*After-write-callback threw.*");
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, SetData_NoMutex)
{
  CreateUUT_D(false);

  Data d2;
  ON_SCOPE_EXIT() { spUUT.reset(); };

  EXPECT_THROW(spUUT->SetData(&d2), std::logic_error);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, SetData_DataDoesNotChange)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  spUUT->SetData(&data);

  auto locker(spUUT->LockData());

  data.data_bool = true;
  data.data_i8 = -25;
  data.data_ui8 = 239U;
  data.data_ui32a = 0xDEADBEEFUL;
  data.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data.data_bitX[1] = 0x01U;    // bit 0
  data.data_bitX[2] = 0x00U;
  data.data_bitX[3] = 0xB0U;    // bit 4..7
  data.data_visiblestring[0] = 'T';
  data.data_visiblestring[1] = 'e';
  data.data_visiblestring[2] = 's';
  data.data_visiblestring[3] = 't';
  data.data_visiblestring[4] = 0x00;
  data.data_visiblestring[5] = 0x00;
  data.data_visiblestring[6] = 0x00;
  data.data_visiblestring[7] = 0x00;
  data.data_ui32b = 0x12345678UL;
  data.data_octectstring[0] = 0xF5U;
  data.data_octectstring[1] = 0xDEU;
  data.data_octectstring[2] = 0xB2U;
  data.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

TEST_F(gpcc_cood_ObjectRECORD_wicb_TestsF, SetData_DataDoesChange)
{
  CreateUUT_A();
  EXPECT_CALL(cbm, OnBeforeRead(spUUT.get(), 0U, true, false)).Times(1).WillRepeatedly(Return(SDOAbortCode::OK));

  Data data2;
  spUUT->SetData(&data2);
  ON_SCOPE_EXIT() { spUUT.reset(); };

  auto locker(spUUT->LockData());

  data2.data_bool = true;
  data2.data_i8 = -25;
  data2.data_ui8 = 239U;
  data2.data_ui32a = 0xDEADBEEFUL;
  data2.data_bitX[0] = 0x82U;    // bit 0, 1, 7
  data2.data_bitX[1] = 0x01U;    // bit 0
  data2.data_bitX[2] = 0x00U;
  data2.data_bitX[3] = 0xB0U;    // bit 4..7
  data2.data_visiblestring[0] = 'T';
  data2.data_visiblestring[1] = 'e';
  data2.data_visiblestring[2] = 's';
  data2.data_visiblestring[3] = 't';
  data2.data_visiblestring[4] = 0x00;
  data2.data_visiblestring[5] = 0x00;
  data2.data_visiblestring[6] = 0x00;
  data2.data_visiblestring[7] = 0x00;
  data2.data_ui32b = 0x12345678UL;
  data2.data_octectstring[0] = 0xF5U;
  data2.data_octectstring[1] = 0xDEU;
  data2.data_octectstring[2] = 0xB2U;
  data2.data_octectstring[3] = 0x87U;

  EXPECT_EQ(spUUT->CompleteRead(true, true, Object::attr_ACCESS_RD_PREOP, writeBufferWriter), SDOAbortCode::OK);

  EXPECT_EQ(sizeof(writeBuffer) - writeBufferWriter.RemainingCapacity(), 26U);

  writeBufferWriter.Close();

  EXPECT_EQ(writeBuffer[0], 11U);
  EXPECT_EQ(writeBuffer[1], 0U);
  EXPECT_EQ(writeBuffer[2], 0x01U);
  EXPECT_EQ(writeBuffer[3], static_cast<uint8_t>(-25));
  EXPECT_EQ(writeBuffer[4], 239U);
  EXPECT_EQ(writeBuffer[5], 0xEFU);
  EXPECT_EQ(writeBuffer[6], 0xBEU);
  EXPECT_EQ(writeBuffer[7], 0xADU);
  EXPECT_EQ(writeBuffer[8], 0xDEU);
  EXPECT_EQ(writeBuffer[9], 0xBEU); // Bits 0, 7..8, 1, 28..31
  EXPECT_EQ(writeBuffer[10], static_cast<uint8_t>('T'));
  EXPECT_EQ(writeBuffer[11], static_cast<uint8_t>('e'));
  EXPECT_EQ(writeBuffer[12], static_cast<uint8_t>('s'));
  EXPECT_EQ(writeBuffer[13], static_cast<uint8_t>('t'));
  EXPECT_EQ(writeBuffer[14], 0U);
  EXPECT_EQ(writeBuffer[15], 0U);
  EXPECT_EQ(writeBuffer[16], 0U);
  EXPECT_EQ(writeBuffer[17], 0U);
  EXPECT_EQ(writeBuffer[18], 0x78U);
  EXPECT_EQ(writeBuffer[19], 0x56U);
  EXPECT_EQ(writeBuffer[20], 0x34U);
  EXPECT_EQ(writeBuffer[21], 0x12U);
  EXPECT_EQ(writeBuffer[22], 0xF5U);
  EXPECT_EQ(writeBuffer[23], 0xDEU);
  EXPECT_EQ(writeBuffer[24], 0xB2U);
  EXPECT_EQ(writeBuffer[25], 0x87U);
}

} // namespace gpcc_tests
} // namespace cood
