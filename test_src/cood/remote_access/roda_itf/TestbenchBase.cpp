/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef SKIP_TFC_BASED_TESTS

#include "TestbenchBase.hpp"
#include "gpcc/src/cood/ObjectARRAY.hpp"
#include "gpcc/src/cood/ObjectVAR.hpp"
#include "gpcc/src/cood/ObjectVAR_wicb.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include <gpcc/string/tools.hpp>
#include "gpcc/test_src/cood/ObjectVAR_wicb_withASM.hpp"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace gpcc_tests {
namespace cood       {

uint32_t const TestbenchBase::beforeReadCallbackDuration_ms;
uint32_t const TestbenchBase::beforeWriteCallbackDuration_ms;

gpcc::cood::ObjectRECORD::SubIdxDescr const TestbenchBase::Descr0x3000[11] =
{
    // name,        type,                                 attributes,                               nElements, byteOffset,                                              bitOffset
    { "Data Bool",  gpcc::cood::DataType::boolean,        gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_bool),          0},
    { "Data i8",    gpcc::cood::DataType::integer8,       gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_i8),            0},
    { "Data ui8",   gpcc::cood::DataType::unsigned8,      gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_ui8),           0},
    { "Data ui32a", gpcc::cood::DataType::unsigned32,     gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_ui32a),         0},
    { "Bit 0",      gpcc::cood::DataType::bit1,           gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_bitX),          0},
    { "Bit 7..8",   gpcc::cood::DataType::bit2,           gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_bitX),          7},
    { "Bit 1",      gpcc::cood::DataType::bit1,           gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_bitX),          1},
    { "Bit 28..31", gpcc::cood::DataType::bit4,           gpcc::cood::Object::attr_ACCESS_RW,       1,         offsetof(TestbenchBase::Data0x3000, data_bitX) + 3,      4},
    { "Text",       gpcc::cood::DataType::visible_string, gpcc::cood::Object::attr_ACCESS_RW,       8,         offsetof(TestbenchBase::Data0x3000, data_visiblestring), 0},
    { "Data ui32b", gpcc::cood::DataType::unsigned32,     gpcc::cood::Object::attr_ACCESS_RD,       1,         offsetof(TestbenchBase::Data0x3000, data_ui32b),         0},
    { "Octet str",  gpcc::cood::DataType::octet_string,   gpcc::cood::Object::attr_ACCESS_RW,       4,         offsetof(TestbenchBase::Data0x3000, data_octectstring),  0}
};

/**
 * \brief Enumerates the indices of objects in the object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param attrFilter
 * Filter for attributes. Only objects that have at least one subindex with at least one attribute bit matching this
 * mask are enumerated.\n
 * Zero is not allowed.
 *
 * \return
 * Indices of enumerated objects.
 */
std::vector<uint16_t> TestbenchBase::EnumerateObjects(gpcc::cood::Object::attr_t const attrFilter)
{
  if (attrFilter == 0U)
    throw std::invalid_argument("TestbenchBase::EnumerateObjects: 'attrFilter' invalid");

  std::vector<uint16_t> indices;

  auto obj = od.GetFirstObject();
  while (obj)
  {
    auto const nbOfSI = obj->GetMaxNbOfSubindices();

    for (uint_fast16_t i = 0; i < nbOfSI; ++i)
    {
      if ((obj->GetSubIdxAttributes(i) & attrFilter) != 0U)
      {
        indices.push_back(obj->GetIndex());
        break;
      }
    }

    ++obj;
  }

  return indices;
}

/**
 * \brief Sets the value of the ARRAY object 0x2000, SI0.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param si0
 * New value for subindex 0.
 */
void TestbenchBase::Set0x2000_SI0(uint8_t const si0)
{
  pObj0x2000->SetData(si0, data0x2000);
}

/**
 * \brief Retrieves the number of subindices of ARRAY object 0x2000.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref dataMutex must be locked by caller.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Current number of subindices of object 0x2000, incl. SI0.
 */
uint16_t TestbenchBase::GetNbOfSI0x2000(void)
{
  return pObj0x2000->GetNbOfSubIndices();
}

/**
 * \brief Creates dublicates of object 0x1000 starting at 0x8000.
 *
 * This is intended to be invoked by unit test cases that require a large amount of objects in the object dictionary.
 *
 * \pre   There are no objects registered yet at [0x8000; 0x8000 + count-1].
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Some objects may have been created. They will not be removed again.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Some objects may have been created. They will not be removed again.
 *
 * - - -
 *
 * \param count
 * Number of copies.\n
 * Allowed range: 0..32768
 */
void TestbenchBase::CreateDublicatesOf0x1000(uint16_t const count)
{
  if ((count == 0U) || (count > 32768U))
    throw std::invalid_argument("TestbenchBase::CreateDublicatesOf0x1000: 'count' invalid");

  for (uint_fast16_t i = 0; i < count; ++i)
  {
    PublishVariableObjectU32(0x8000U + i, "Dublicate of 0x1000", &data0x1000);
  }
}

/**
 * \brief Prints all recorded log messages to stdout and discards all recorded log messages.
 *
 * \post  The recorder is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - incomplete/undefined output may have been written to stdout.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - not all records may be printed to stdout.
 * - the last printed record may be incomplete.
 */
void TestbenchBase::PrintLogMessagesToStdout(void)
{
  logFacility.Flush();
  logRecorder.PrintToStdout(true);
}

/**
 * \brief Constructor. Creates the common part of the testbench and starts common components.
 *
 * The UUT is provided and started by the sub-class. The constructor of the sub-class will be executed after this.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TestbenchBase::TestbenchBase(void)
: logFacility("LogFacility", 1000U)
, logRecorder()
, tcLogger("Testcase")
, tbLogger("Testbench")
, rodanLogger("Listener")
, dataMutex()
, data0x1000(0U)
, data0x1001(0U)
, data0x1002(0U)
, data0x1004(0U)
, data0x1005(0xDEADBEEFUL)
, data0x1010()
, data0x2000()
, data0x3000()
, od()
, pObj0x2000(nullptr)
{
  for (uint_fast8_t i = 0U; i < sizeof(data0x1003); i++)
    data0x1003[i] = i;

  for (uint_fast16_t i = 0; i < sizeof(data0x2000); i++)
    data0x2000[i] = i;

  logFacility.Register(logRecorder);
  ON_SCOPE_EXIT(unregLogRecorder) { logFacility.Unregister(logRecorder); };

  tcLogger.SetLogLevel(gpcc::log::LogLevel::DebugOrAbove);
  logFacility.Register(tcLogger);
  ON_SCOPE_EXIT(unregTCLogger) { logFacility.Unregister(tcLogger); };

  tbLogger.SetLogLevel(gpcc::log::LogLevel::DebugOrAbove);
  logFacility.Register(tbLogger);
  ON_SCOPE_EXIT(unregTBLogger) { logFacility.Unregister(tbLogger); };

  rodanLogger.SetLogLevel(gpcc::log::LogLevel::DebugOrAbove);
  logFacility.Register(rodanLogger);
  ON_SCOPE_EXIT(unregRodanLogger) { logFacility.Unregister(rodanLogger); };

  logFacility.Start(gpcc::osal::Thread::SchedPolicy::Other, 0U, gpcc::osal::Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(stopLogFacility) { logFacility.Stop(); };

  PublishVariableObjectU32withASM(0x1000U, "Testobject 1", &data0x1000, { 0xDEU, 0xADU, 0xBEU, 0xEFU });
  PublishVariableObjectU32(0x1001U, "Testobject 2", &data0x1001);
  PublishVariableObjectU32(0x1002U, "Testobject 3", &data0x1002);
  PublishVariableObjectOctetString(0x1003U, "Testobject 4", data0x1003, sizeof(data0x1003));
  PublishVariableObjectU32(0x1004U, "Testobject 5", &data0x1004);
  PublishVariableObjectU32ro(0x1005U, "Testobject 6", &data0x1005);

  {
    std::unique_ptr<gpcc::cood::Object> spObj;
    spObj = std::make_unique<gpcc::cood::ObjectVAR>("Testobject 7",
                                                    gpcc::cood::DataType::visible_string,
                                                    sizeof(data0x1010) - 1U,
                                                    gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR,
                                                    data0x1010,
                                                    &dataMutex,
                                                    nullptr);
    od.Add(spObj, 0x1010U);
  }

  {
    std::unique_ptr<gpcc::cood::Object> spObj;
    spObj = std::make_unique<gpcc::cood::ObjectARRAY>("Testobject 8",
                                                      gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR,
                                                      6U,
                                                      0U,
                                                      255U,
                                                      gpcc::cood::DataType::unsigned8,
                                                      gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR,
                                                      data0x2000,
                                                      &dataMutex,
                                                      nullptr);
    pObj0x2000 = static_cast<gpcc::cood::ObjectARRAY*>(spObj.get());
    od.Add(spObj, 0x2000U);
  }

  {
    std::unique_ptr<gpcc::cood::Object> spObj;
    spObj = std::make_unique<gpcc::cood::ObjectRECORD>("Testobject 9",
                                                       11U,
                                                       &data0x3000,
                                                       sizeof(Data0x3000),
                                                       &dataMutex,
                                                       Descr0x3000,
                                                       nullptr);
    od.Add(spObj, 0x3000U);
  }

  ON_SCOPE_EXIT_DISMISS(stopLogFacility);
  ON_SCOPE_EXIT_DISMISS(unregRodanLogger);
  ON_SCOPE_EXIT_DISMISS(unregTBLogger);
  ON_SCOPE_EXIT_DISMISS(unregTCLogger);
  ON_SCOPE_EXIT_DISMISS(unregLogRecorder);
}

/**
 * \brief Destructor. Stops common components and destroys the common part of the testbench.
 *
 * The UUT is stopped and destroyed by the sub-class. The destructor of the sub-class will be executed before this.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
TestbenchBase::~TestbenchBase(void)
{
  try
  {
    od.Clear();

    logFacility.Stop();

    logFacility.Unregister(rodanLogger);
    logFacility.Unregister(tbLogger);
    logFacility.Unregister(tcLogger);
    logFacility.Unregister(logRecorder);
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "TestbenchBase::~TestbenchBase: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchBase::~TestbenchBase: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("TestbenchBase::~TestbenchBase: Caught an unknown exception");
  }
}

/**
 * \brief Publishes a variable of type uint32_t in the object dictionary using a VARIABLE-object
 *        ([ObjectVAR_wicb](gpcc::cood::ObjectVAR_wicb)).
 *
 * The following settings and configurations are applied to the VARIABLE-object:
 * - Access rights: Object::attr_ACCESS_RD | Object::attr_ACCESS_WR
 * - Mutex: @ref dataMutex
 * - Before write callback: @ref OnBeforeWriteCallback
 * - After write callback: @ref OnAfterWriteCallback
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param index
 * Desired index for the object.
 *
 * \param name
 * Desired name for the object.
 *
 * \param pData
 * Pointer to the uint32_t that shall be published.\n
 * nullptr is not allowed.
 */
void TestbenchBase::PublishVariableObjectU32(uint16_t const index, std::string const & name, uint32_t * const pData)
{
  if (pData == nullptr)
    throw std::invalid_argument("TestbenchBase::PublishVariableObjectU32: !pData");

  std::unique_ptr<gpcc::cood::Object> spObj;
  spObj = std::make_unique<gpcc::cood::ObjectVAR_wicb>(name,
                                                       gpcc::cood::DataType::unsigned32,
                                                       1U,
                                                       gpcc::cood::Object::attr_ACCESS_RD |
                                                       gpcc::cood::Object::attr_ACCESS_WR,
                                                       pData,
                                                       &dataMutex,
                                                       std::bind(&TestbenchBase::OnBeforeReadCallback,
                                                                 this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 std::placeholders::_3),
                                                       std::bind(&TestbenchBase::OnBeforeWriteCallback,
                                                                 this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 std::placeholders::_3,
                                                                 std::placeholders::_4,
                                                                 std::placeholders::_5),
                                                       std::bind(&TestbenchBase::OnAfterWriteCallback,
                                                                this,
                                                                std::placeholders::_1,
                                                                std::placeholders::_2,
                                                                std::placeholders::_3));
  od.Add(spObj, index);
}

/**
 * \brief Publishes a variable of type uint32_t in the object dictionary using a VARIABLE-object with application
 *        specific meta data ([ObjectVARwithASM](gpcc_tests::cood::ObjectVARwithASM)).
 *
 * The following settings and configurations are applied to the VARIABLE-object:
 * - Access rights: Object::attr_ACCESS_RD | Object::attr_ACCESS_WR
 * - Mutex: @ref dataMutex
 * - Before write callback: @ref OnBeforeWriteCallback
 * - After write callback: @ref OnAfterWriteCallback
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param index
 * Desired index for the object.
 *
 * \param name
 * Desired name for the object.
 *
 * \param pData
 * Pointer to the uint32_t that shall be published.\n
 * nullptr is not allowed.
 *
 * \param appSpecMetaData
 * Application specific meta data that shall be attached to the object.
 */
void TestbenchBase::PublishVariableObjectU32withASM(uint16_t const index, std::string const & name, uint32_t * const pData,
                                                    std::vector<uint8_t> const & appSpecMetaData)
{
  if (pData == nullptr)
    throw std::invalid_argument("TestbenchBase::PublishVariableObjectU32withASM: !pData");

  auto copyOfAppSpecData = appSpecMetaData;
  std::unique_ptr<gpcc::cood::Object> spObj;
  spObj = std::make_unique<ObjectVAR_wicb_withASM>(name,
                                                   gpcc::cood::DataType::unsigned32,
                                                   1U,
                                                   gpcc::cood::Object::attr_ACCESS_RD |
                                                   gpcc::cood::Object::attr_ACCESS_WR,
                                                   pData,
                                                   &dataMutex,
                                                   std::bind(&TestbenchBase::OnBeforeReadCallback,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3),
                                                   std::bind(&TestbenchBase::OnBeforeWriteCallback,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3,
                                                             std::placeholders::_4,
                                                             std::placeholders::_5),
                                                   std::bind(&TestbenchBase::OnAfterWriteCallback,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3),
                                                   std::move(copyOfAppSpecData));
  od.Add(spObj, index);
}

/**
 * \brief Publishes a variable of type uint32_t with ro-access in the object dictionary using a VARIABLE-object
 *        ([ObjectVAR](gpcc::cood::ObjectVAR)).
 *
 * The following settings and configurations are applied to the VARIABLE-object:
 * - Access rights: Object::attr_ACCESS_RD
 * - Mutex: none
 * - Callbacks: none
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param index
 * Desired index for the object.
 *
 * \param name
 * Desired name for the object.
 *
 * \param pData
 * Pointer to the uint32_t that shall be published.\n
 * nullptr is not allowed.
 */
void TestbenchBase::PublishVariableObjectU32ro(uint16_t const index, std::string const & name, uint32_t * const pData)
{
  if (pData == nullptr)
    throw std::invalid_argument("TestbenchBase::PublishVariableObjectU32ro: !pData");

  std::unique_ptr<gpcc::cood::Object> spObj;
  spObj = std::make_unique<gpcc::cood::ObjectVAR>(name,
                                                  gpcc::cood::DataType::unsigned32,
                                                  1U,
                                                  gpcc::cood::Object::attr_ACCESS_RD,
                                                  pData,
                                                  nullptr,
                                                  nullptr);
  od.Add(spObj, index);
}

/**
 * \brief Publishes a variable of type OCTET_STRING in the object dictionary using a VARIABLE-object
 *        ([ObjectVAR_wicb](gpcc::cood::ObjectVAR_wicb)).
 *
 * The following settings and configurations are applied to the VARIABLE-object:
 * - Access rights: Object::attr_ACCESS_RD | Object::attr_ACCESS_WR
 * - Mutex: @ref dataMutex
 * - Before write callback: @ref OnBeforeWriteCallback
 * - After write callback: @ref OnAfterWriteCallback
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param index
 * Desired index for the object.
 *
 * \param name
 * Desired name for the object.
 *
 * \param pData
 * Pointer to the data that shall be published.\n
 * nullptr is not allowed.
 *
 * \param s
 * Number of bytes.\n
 * Zero is not allowed.
 */
void TestbenchBase::PublishVariableObjectOctetString(uint16_t const index, std::string const & name, void * const pData, size_t const s)
{
  if (pData == nullptr)
    throw std::invalid_argument("TestbenchBase::PublishVariableObjectOctetString: !pData");

  if (s == 0U)
    throw std::invalid_argument("TestbenchBase::PublishVariableObjectOctetString: 's' is zero");

  std::unique_ptr<gpcc::cood::Object> spObj;
  spObj = std::make_unique<gpcc::cood::ObjectVAR_wicb>(name,
                                                       gpcc::cood::DataType::octet_string,
                                                       s,
                                                       gpcc::cood::Object::attr_ACCESS_RD |
                                                       gpcc::cood::Object::attr_ACCESS_WR,
                                                       pData,
                                                       &dataMutex,
                                                       std::bind(&TestbenchBase::OnBeforeReadCallback,
                                                                 this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 std::placeholders::_3),
                                                       std::bind(&TestbenchBase::OnBeforeWriteCallback,
                                                                 this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 std::placeholders::_3,
                                                                 std::placeholders::_4,
                                                                 std::placeholders::_5),
                                                       std::bind(&TestbenchBase::OnAfterWriteCallback,
                                                                this,
                                                                std::placeholders::_1,
                                                                std::placeholders::_2,
                                                                std::placeholders::_3));
  od.Add(spObj, index);
}

/**
 * \brief Before-read-callback for all object dictionary objects created by this class.
 *
 * Depending on the object being accessed, this may throw by intention or return a bad SDO abort code.\n
 * See @ref TestbenchBase for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref dataMutex will be locked by caller.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param pObject
 * Pointer to the object that shall be read.
 *
 * \param subindex
 * Subindex that shall be read or in case of complete access the first subindex that shall be read.
 *
 * \param ca
 * Access type: Complete access (true) or single subindex access (false).
 *
 * \return
 * SDO abort code. The read will be carried out if this is "OK".
 */
gpcc::cood::SDOAbortCode TestbenchBase::OnBeforeReadCallback(gpcc::cood::Object const * pObject,
                                                             uint8_t subindex,
                                                             bool ca)
{
  if (tbLogger.IsAboveLevel(gpcc::log::LogType::Info))
  {
    try
    {
      std::ostringstream oss;
      oss << "OnBeforeReadCallback: Object "
          << gpcc::string::ToHex(pObject->GetIndex(), 4U)
          << ", SI "
          << static_cast<unsigned int>(subindex)
          << ", ca = ";

      if (ca)
        oss << "true";
      else
        oss << "false";

      tbLogger.Log(gpcc::log::LogType::Info, oss.str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchBase::OnBeforeReadCallback");
    }
  }

  gpcc::osal::Thread::Sleep_ms(beforeReadCallbackDuration_ms);

  switch (pObject->GetIndex())
  {
    case 0x1001U:
      throw std::runtime_error("TestbenchBase::OnBeforeReadCallback: Intentionally thrown exception");

    case 0x1002U:
      throw std::bad_alloc();

    case 0x1004U:
      return gpcc::cood::SDOAbortCode::GeneralError;
  }

  return gpcc::cood::SDOAbortCode::OK;
}

/**
 * \brief Before-write-callback for all object dictionary objects created by this class.
 *
 * Depending on the object being accessed, this may throw by intention or return a bad SDO abort code.\n
 * See @ref TestbenchBase for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref dataMutex will be locked by caller.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param pObject
 * Pointer to the object that shall be written.
 *
 * \param subindex
 * Subindex that shall be written.
 *
 * \param ca
 * Access type: Complete access (true) or single subindex access (false).
 *
 * \param SI0
 * Value written to SI0.\n
 * This is only valid/meaningful under certain conditions. See @ref gpcc::cood::Object::tOnBeforeWriteCallback for
 * details.
 *
 * \param pData
 * Pointer to the data that shall be written for preview purposes.\n
 * See @ref gpcc::cood::Object::tOnBeforeWriteCallback for details.
 *
 * \return
 * SDO abort code. The write will be carried out if this is "OK".
 */
gpcc::cood::SDOAbortCode TestbenchBase::OnBeforeWriteCallback(gpcc::cood::Object const * pObject,
                                                              uint8_t subindex,
                                                              bool ca,
                                                              uint8_t SI0,
                                                              void const * pData)
{
  (void)SI0;
  (void)pData;

  if (tbLogger.IsAboveLevel(gpcc::log::LogType::Info))
  {
    try
    {
      std::ostringstream oss;
      oss << "OnBeforeWriteCallback: Object "
          << gpcc::string::ToHex(pObject->GetIndex(), 4U)
          << ", SI "
          << static_cast<unsigned int>(subindex)
          << ", ca = ";

      if (ca)
        oss << "true";
      else
        oss << "false";

      tbLogger.Log(gpcc::log::LogType::Info, oss.str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchBase::OnAfterWriteCallback");
    }
  }

  gpcc::osal::Thread::Sleep_ms(beforeWriteCallbackDuration_ms);

  switch (pObject->GetIndex())
  {
    case 0x1001U:
      throw std::runtime_error("TestbenchBase::OnBeforeWriteCallback: Intentionally thrown exception");

    case 0x1002U:
      throw std::bad_alloc();

    case 0x1004U:
      return gpcc::cood::SDOAbortCode::GeneralError;
  }

  return gpcc::cood::SDOAbortCode::OK;
}

/**
 * \brief After-write-callback for all object dictionary objects created by this class.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pObject
 * Pointer to the object that has been written.
 *
 * \param subindex
 * Subindex that has been written.
 *
 * \param ca
 * Indicates if complete access (true) or single subindex access (false) has been used.
 */
void TestbenchBase::OnAfterWriteCallback(gpcc::cood::Object const * pObject, uint8_t subindex, bool ca)
{
  if (tbLogger.IsAboveLevel(gpcc::log::LogType::Info))
  {
    try
    {
      std::ostringstream oss;
      oss << "OnAfterWriteCallback: Object "
          << gpcc::string::ToHex(pObject->GetIndex(), 4U)
          << ", SI "
          << static_cast<unsigned int>(subindex)
          << ", ca = ";

      if (ca)
        oss << "true";
      else
        oss << "false";

      tbLogger.Log(gpcc::log::LogType::Info, oss.str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchBase::OnAfterWriteCallback");
    }
  }
}

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
