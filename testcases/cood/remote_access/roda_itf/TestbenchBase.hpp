/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTBENCHBASE_HPP_202008192205
#define TESTBENCHBASE_HPP_202008192205

#ifndef SKIP_TFC_BASED_TESTS

#include <gpcc/cood/Object.hpp>
#include <gpcc/cood/ObjectRECORD.hpp>
#include <gpcc/cood/ObjectDictionary.hpp>
#include <gpcc/cood/sdo_abort_codes.hpp>
#include <gpcc/log/logfacilities/ThreadedLogFacility.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc_test/log/backends/Backend_Recorder.hpp>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace cood {
  class Object;
  class ObjectARRAY;
  class IRemoteObjectDictionaryAccess;
}
}

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTCASES_COOD_RA
 * \brief Base class for all testbenches of type T that shall be tested using IRemoteObjectDictionaryAccess_TestsF<T>.
 *
 * Please refer to @ref GPCC_TESTCASES_COOD_RA for detailed information about the test strategy for the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface.
 *
 * This class constains an object dictionary and a couple of CANopen objects accessible via the object dictionary.
 * Some objects implement special behaviour when they are accessed, e.g. they may throw by intention.
 *
 * Index  | Object    | ASM | Access rights        | Special behavior write                            | Special behavior read
 * ------ | --------- | --- | -------------------- | ------------------------------------------------- | ---------------------
 * 0x1000 | VARIABLE  | yes | full RD/WR           | None                                              | None
 * 0x1001 | VARIABLE  | no  | full RD/WR           | Before-write-callback throws `std::runtime_error` | Before-read-callback throws `std::runtime_error`
 * 0x1002 | VARIABLE  | no  | full RD/WR           | Before-write-callback throws `std::bad_alloc`     | Before-read-callback throws `std::bad_alloc`
 * 0x1003 | VARIABLE  | no  | full RD/WR           | None                                              | None
 * 0x1004 | VARIABLE  | no  | full RD/WR           | Before-write-callback rejects (GeneralError)      | Before-read-callback  rejects (GeneralError)
 * 0x1010 | VARIABLE  | no  | full RD/WR           | None                                              | None
 * 0x2000 | ARRAY     | no  | full RD/WR           | None                                              | None
 * 0x3000 | RECORD    | no  | RD/WR, SI0 & SI10 RO | None                                              | None
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TestbenchBase
{
  public:
    /// Log facility for logging during test execution.
    gpcc::log::ThreadedLogFacility logFacility;

    /// Recorder for log messages.
    gpcc_tests::log::Backend_Recorder logRecorder;


    /// Logger for the testcase.
    gpcc::log::Logger tcLogger;

    /// Logger for the testbench.
    gpcc::log::Logger tbLogger;

    /// Logger for the @ref RODAN_Listener instance that is part of the test fixture.
    gpcc::log::Logger rodanLogger;


    /// Mutex protecting data accessible via the object dictionary.
    gpcc::osal::Mutex dataMutex;

    /// Object 0x1000 VAR (RD/WR, dataMutex).
    uint32_t data0x1000;

    /// Object 0x1001 VAR (RD/WR, runtime_error @ before-write/before-read-callback, dataMutex).
    uint32_t data0x1001;

    /// Object 0x1002 VAR (RD/WR, std::bad_alloc @ before-write/before-read-callback, dataMutex).
    uint32_t data0x1002;

    /// Object 0x1003 VAR
    uint8_t data0x1003[128];

    /// Object 0x1004 VAR (RD/WR, callback rejects any access, dataMutex)
    uint32_t data0x1004;

    /// Object 0x1005 VAR (RD)
    uint32_t data0x1005;

    /// Object 0x1010 VAR (RD/WR) (visible_string, length 32)
    char data0x1010[33];

    /// Object 0x2000 ARRAY(RD/WR)
    uint8_t data0x2000[255];

    /// Object 0x3000 RECORD(RD/WR)
    struct Data0x3000
    {
      bool data_bool = false;
      int8_t data_i8 = 0U;
      uint8_t data_ui8 = 0U;
      uint32_t data_ui32a = 0U;
      uint8_t data_bitX[4];
      char data_visiblestring[8];
      uint32_t data_ui32b = 0U;
      uint8_t data_octectstring[4];
    } data0x3000;


    /// Object dictionary.
    gpcc::cood::ObjectDictionary od;


    TestbenchBase(TestbenchBase const &) = delete;
    TestbenchBase(TestbenchBase &&) = delete;

    TestbenchBase& operator=(TestbenchBase const &) = delete;
    TestbenchBase& operator=(TestbenchBase &&) = delete;


    virtual void StartUUT(void) = 0;
    virtual void StopUUT(void) = 0;

    virtual gpcc::cood::IRemoteObjectDictionaryAccess & GetUUT(void) = 0;

    virtual uint32_t GetOnReadyTimeout_ms(void) const = 0;
    virtual uint32_t GetMinimumResponseTime_ms(void) const = 0;
    virtual uint32_t GetResponseTimeout_ms(void) const = 0;
    virtual uint32_t GetTimeUntilMiddleOfTransmittingRequest_ms(void) const = 0;
    virtual uint32_t GetTimeUntilMiddleOfProcessing_ms(void) const = 0;
    virtual uint32_t GetTimeUntilMiddleOfTransmittingResponse_ms(void) const = 0;
    virtual size_t   GetExpectedMaxRequestSize(void) const = 0;
    virtual size_t   GetExpectedMaxResponseSize(void) const = 0;

    std::vector<uint16_t> EnumerateObjects(gpcc::cood::Object::attr_t const attrFilter);

    void Set0x2000_SI0(uint8_t const si0);
    uint16_t GetNbOfSI0x2000(void);

    void CreateDublicatesOf0x1000(uint16_t const count);
    void PrintLogMessagesToStdout(void);

  protected:
    /// Duration of the before-read-callback in ms.
    static uint32_t const beforeReadCallbackDuration_ms = 10U;

    /// Duration of the before-write-callback in ms.
    static uint32_t const beforeWriteCallbackDuration_ms = 10U;

    // Description of RECORD object at 0x3000.
    static gpcc::cood::ObjectRECORD::SubIdxDescr const Descr0x3000[11];


    /// Object 0x2000.
    gpcc::cood::ObjectARRAY* pObj0x2000;

    TestbenchBase(void);
    virtual ~TestbenchBase(void);

  private:
    void PublishVariableObjectU32(uint16_t const index, std::string const & name, uint32_t * const pData);
    void PublishVariableObjectU32withASM(uint16_t const index, std::string const & name, uint32_t * const pData,
                                        std::vector<uint8_t> const & appSpecMetaData);
    void PublishVariableObjectU32ro(uint16_t const index, std::string const & name, uint32_t * const pData);
    void PublishVariableObjectOctetString(uint16_t const index, std::string const & name, void * const pData, size_t const s);

    gpcc::cood::SDOAbortCode OnBeforeReadCallback(gpcc::cood::Object const * pObject,
                                                  uint8_t subindex,
                                                  bool ca);

    gpcc::cood::SDOAbortCode OnBeforeWriteCallback(gpcc::cood::Object const * pObject,
                                                   uint8_t subindex,
                                                   bool ca,
                                                   uint8_t SI0,
                                                   void const * pData);
    void OnAfterWriteCallback(gpcc::cood::Object const * pObject, uint8_t subindex, bool ca);
};

/**
 * \fn void TestbenchBase::StartUUT(void)
 * \brief Starts the UUT (unit under test).
 *
 * Usually "UUT" refers to the component providing the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface. It is provided by the
 * class derived from this. This method shall start the UUT and -in some cases- additional components closely coupled
 * to the UUT.
 *
 * This shall not be mixed up with starting the testbench in general. The common components of the testbench like
 * the log facility are started upon creation of the testbench object and are stopped upon destruction of the
 * testbench object.
 *
 * \pre   The UUT is not running.
 *
 * \post  The UUT is running.
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
 */

/**
 * \fn void TestbenchBase::StopUUT(void)
 * \brief Stops the UUT (unit under test).
 *
 * This method shall stop the UUT and -in some cases- additional components closely coupled to the UUT.\n
 * This is the counterpart of [StartUUT()](@ref gpcc_tests::cood::TestbenchBase::StartUUT). See
 * [StartUUT()](@ref gpcc_tests::cood::TestbenchBase::StartUUT) for details.
 *
 * \pre   The UUT is running.
 *
 * \post  The UUT is not running.
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
 * Deferred cancellation is not allowed.
 */

/**
 * \fn gpcc::cood::IRemoteObjectDictionaryAccess & TestbenchBase::GetUUT(void)
 * \brief Retrieves a pointer to the RODA-interface offered by the UUT.
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
 * \return
 * Pointer to the RODA-interface offered by the UUT.
 */

/**
 * \fn uint32_t TestbenchBase::GetOnReadyTimeout_ms(void) const
 * \brief Retrieves the recommended timeout (in ms) for waiting for the OnReady(...)-callback.
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
 * \return
 * Recommended timeout (in ms) for waiting for the OnReady(...)-callback.
 */

/**
 * \fn uint32_t TestbenchBase::GetMinimumResponseTime_ms(void) const
 * \brief Retrieves the minimum time span (in ms) between sending a request and reception of the response.
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
 * \return
 * Minimum time span (in ms) between transmission of a request (return of
 * [IRemoteObjectDictionaryAccess::Send(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send) and reception of
 * the response.
 */

/**
 * \fn uint32_t TestbenchBase::GetResponseTimeout_ms(void) const
 * \brief Retrieves the recommended timeout (in ms) for waiting for a response.
 *
 * Assumption: There is only one request and there are no responses in the pipe.
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
 * \return
 * Recommended timeout (in ms) for waiting for a response. The timeout starts after transmission of the request
 * (return of [IRemoteObjectDictionaryAccess::Send(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send).
 */

/**
 * \fn uint32_t TestbenchBase::GetTimeUntilMiddleOfTransmittingRequest_ms(void) const
 * \brief Retrieves the time span since call to `Send(...)` until the request has travelled half-way to the server when
 *        the ideal time point has come to test disconnection.
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
 * \return
 * Time span (in ms) since return of the call to [IRemoteObjectDictionaryAccess::Send(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * until the request has travelled half-way to the server.\n
 * __Zero__ means, that the test bench does not support this scenario.
 */

/**
 * \fn uint32_t TestbenchBase::GetTimeUntilMiddleOfProcessing_ms(void) const
 * \brief Retrieves the time span since call to `Send(...)` until middle of processing when the ideal time point has
 *        come to test disconnection.
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
 * \return
 * Time span (in ms) since return of the call to [IRemoteObjectDictionaryAccess::Send(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * until middle of processing of the request.\n
 * What "processing" refers to is defined by the derived class.
 */

/**
 * \fn uint32_t TestbenchBase::GetTimeUntilMiddleOfTransmittingResponse_ms(void) const
 * \brief Retrieves the time span since call to `Send(...)` until the response has travelled half-way from the server
 *        back to the client when the ideal time point has come to test disconnection.
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
 * \return
 * Time span (in ms) since return of the call to [IRemoteObjectDictionaryAccess::Send(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * until the response has travelled half-way from the server back to the client.\n
 * __Zero__ means, that the test bench does not support this scenario.
 */

/**
 * \fn size_t TestbenchBase::GetExpectedMaxRequestSize(void) const
 * \brief Retrieves the expected value of the maximum permitted request size passed to
 *        [IRemoteObjectDictionaryAccessNotifiable::OnReady(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady).
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
 * \return
 * Expected value for the maximum permitted request size passed to
 * [IRemoteObjectDictionaryAccessNotifiable::OnReady(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady).
 */

/**
 * \fn size_t TestbenchBase::GetExpectedMaxResponseSize(void) const
 * \brief Retrieves the expected value of the maximum permitted response size passed to
 *        [IRemoteObjectDictionaryAccessNotifiable::OnReady(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady).
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
 * \return
 * Expected value for the maximum permitted response size passed to
 * [IRemoteObjectDictionaryAccessNotifiable::OnReady(...)](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady).
 */

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTBENCHBASE_HPP_202008192205
