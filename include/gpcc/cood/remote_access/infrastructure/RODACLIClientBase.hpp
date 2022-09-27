/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef RODACLICLIENTBASE_HPP_202105221835
#define RODACLICLIENTBASE_HPP_202105221835

#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
#include <gpcc/cood/Object.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace cli  {
  class CLI;
}}

namespace gpcc {
namespace cood {
  class IRemoteObjectDictionaryAccess;
  class ObjectEnumResponse;
  class ObjectInfoResponse;
  class ReadRequestResponse;
  class RequestBase;
  class ResponseBase;
}}


namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Base class for classes implementing a client for a [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess)
 *        interface that offers access to the remote object dictionary via CLI.
 *
 * Derived classes shall invoke @ref Connect() and @ref Disconnect() to connect an disconnect the client to or from
 * a RODA interface.
 *
 * Upon connection, this class will register a [RODAN](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 * interface at the provided RODA interface and transmit and receive messages if necessary.
 *
 * The derived class shall also install a CLI command. The derived class' CLI command handler shall delegate
 * requests to access the object dictionary to the following methods provided by this class:
 * - @ref CLI_Enumerate()
 * - @ref CLI_Info()
 * - @ref CLI_Read()
 * - @ref CLI_Write()
 * - @ref CLI_CARead()
 * - @ref CLI_CAWrite()
 *
 * Further the derived class shall provide an implementation of @ref AttributesToStringHook() and it shall
 * overwrite @ref AppSpecificMetaDataToStringHook() if required to customize displaying of object attributes and
 * to customize displaying of application specific meta data.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class RODACLIClientBase : private IRemoteObjectDictionaryAccessNotifiable
{
  public:
    RODACLIClientBase(void) = delete;
    RODACLIClientBase(RODACLIClientBase const &) = delete;
    RODACLIClientBase(RODACLIClientBase &&) = delete;

    RODACLIClientBase& operator=(RODACLIClientBase const &) = delete;
    RODACLIClientBase& operator=(RODACLIClientBase &&) = delete;

  protected:
    /// CLI where the CLI command is registered.
    gpcc::cli::CLI & cli;


    RODACLIClientBase(gpcc::cli::CLI & _cli, uint8_t const _attributeStringMaxLength);
    virtual ~RODACLIClientBase(void);


    void Connect(IRemoteObjectDictionaryAccess & rodaItf);
    void Disconnect(void);
    IRemoteObjectDictionaryAccess* GetCurrentlyConnectedRODAItf(void);
    bool WaitForRODAItfReady(uint16_t const timeout_ms);

    void CLI_Enumerate(std::string const & restOfLine);
    void CLI_Info(std::string const & restOfLine);
    void CLI_Read(std::string const & restOfLine);
    void CLI_Write(std::string const & restOfLine);
    void CLI_CARead(std::string const & restOfLine);
    void CLI_CAWrite(std::string const & restOfLine);

    virtual std::string AttributesToStringHook(Object::attr_t const attributes) = 0;
    virtual std::string AppSpecificMetaDataToStringHook(std::vector<uint8_t> const & data);

  private:
    /// Enumeration of states of the client.
    enum class States
    {
      notRegistered,  ///<Client is not connected to a RODA interface.
      notReady,       ///<Client is connected to a RODA interface, but RODA interface is not ready.
      ready           ///<Client is connected to a RODA interface and RODA interface is ready.
    };

    /// Timeout while waiting for reception of a response in ms.
    static uint16_t constexpr rxTimeout_ms = 1000U;


    /// Maximum length of any string that could be returned by @ref AttributesToStringHook().
    uint8_t const attributeStringMaxLength;

    /// Owner ID used to tag requests and to check responses.
    uint32_t ownerID;

    /// Mutex used to make @ref Connect() and @ref Disconnect() thread safe.
    /** Locking order: @ref connectMutex -> @ref internalMutex */
    gpcc::osal::Mutex connectMutex;

    /// Mutex used to make this class thread-safe.
    /** Locking order: @ref connectMutex -> @ref internalMutex */
    gpcc::osal::Mutex internalMutex;


    /// Current state of the client.
    /** @ref internalMutex is required. */
    States state;

    /// Pointer to the RODA interface the client is connected to. nullptr = none.
    /** RD: @ref connectMutex OR @ref internalMutex is required.\n
        WR: @ref connectMutex AND @ref internalMutex are both required. */
    IRemoteObjectDictionaryAccess* pRODA;

    /// Maximum request size the client is allowed to transmit.
    /** @ref internalMutex is required.\n
        This is only valid if @ref state is @ref States::ready. \n
        This is set by @ref OnReady(). The size for a @ref ReturnStackItem is already subtracted. */
    size_t maxRequestSize;

    /// Maximum response size the client can receive.
    /** @ref internalMutex is required.\n
        This is only valid if @ref state is @ref States::ready. \n
        This is set by @ref OnReady(). The size for a @ref ReturnStackItem is already subtracted. */
    size_t maxResponseSize;

    /// Session counter.
    /** @ref internalMutex is required. */
    uint32_t sessionCnt;

    /// Response received via the RODAN interface. nullptr = none available.
    /** @ref internalMutex is required. */
    std::unique_ptr<ResponseBase> spReceivedResponse;

    /// Flag indicating that a new response has been received via the RODAN interface before the previous response in
    /// @ref spReceivedResponse has been consumed.
    /** @ref internalMutex is required. */
    bool receiveOverflow;

    /// Condition variable indicating that @ref spReceivedResponse has been filled with a received response.
    /** This shall be used in conjunction with @ref internalMutex. */
    gpcc::osal::ConditionVariable respReceivedConVar;

    /// Condition variable indicating that @ref state has changed.
    /** This shall be used in conjunction with @ref internalMutex. */
    gpcc::osal::ConditionVariable stateChangeConVar;


    static uint_fast8_t DigitsInSubindex(uint8_t const si) noexcept;


    // <-- IRemoteObjectDictionaryAccessNotifiable
    void OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept override;
    void OnDisconnected(void) noexcept override;
    void OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept override;

    void LoanExecutionContext(void) noexcept override;
    // --> IRemoteObjectDictionaryAccessNotifiable

    void Reset(States const newState) noexcept;
    std::unique_ptr<ResponseBase> WaitAndFetchResponse(uint32_t const timeout_ms);
    std::unique_ptr<ResponseBase> TxAndRx(std::unique_ptr<RequestBase> spReq);

    std::unique_ptr<ObjectEnumResponse> Enumerate(uint16_t const firstIndex,
                                                  uint16_t const lastIndex,
                                                  uint16_t maxFragments,
                                                  Object::attr_t const attrFilter);
    std::unique_ptr<ObjectInfoResponse> GetInfo(uint16_t const index,
                                                bool const inclNames,
                                                bool const inclASM);
    std::unique_ptr<ObjectInfoResponse> GetInfoSingleSI(uint16_t const index,
                                                        uint8_t const subIndex,
                                                        bool const inclNames,
                                                        bool const inclASM);
    std::unique_ptr<ReadRequestResponse> Read(uint16_t const index,
                                              uint8_t const subindex,
                                              bool const ca);
    void Write(uint16_t const index,
               uint8_t const subindex,
               bool const ca,
               std::vector<uint8_t> && data);
};


/**
 * \fn std::string RODACLIClientBase::AttributesToStringHook(gpcc::cood::Object::attr_t const attributes)
 * \brief Converts object attributes into a human-readable string.
 *
 * Depending on the application of the object dictionary (e.g. EtherCAT or CANopen), the
 * [object attributes](@ref gpcc::cood::Object::attr_ACCESS_RD) defined by GPCC have a different meaning or are not
 * applicable in EtherCAT or CANopen. There are also bits in [Object::attr_t](@ref gpcc::cood::Object::attr_t) that are
 * not defined by GPCC. Users can assign any custom meaning to them.
 *
 * This method shall convert the attributes into a human-readable string according to the application. For standard
 * applications (e.g. EtherCAT or CANopen), subclasses may delegate the call to
 * [Object::AttributeToString()](@ref gpcc::cood::Object::AttributeToString) if they want. If custom attribute bits are
 * defined, then subclasses should implement this method on their own.
 *
 * The output of this method will be used to compose table-structured CLI output. The length of the returned string must
 * be constant and match the length passed to
 * [RODACLIClientBase::RODACLIClientBase()](@ref gpcc::cood::RODACLIClientBase::RODACLIClientBase), parameter
 * '_attributeStringMaxLength'. If necessary, short output should be extended with space characters. This will ensure
 * that the rows of any table-structured CLI output are properly aligned.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This will be invoked by the CLI thread only.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param attributes
 * Attributes value that shall be converted into a string.
 *
 * \return
 * String object containing a human-readable textual representation of parameter 'attributes'.
 */

/**
 * \fn std::string RODACLIClientBase::AppSpecificMetaDataToStringHook(std::vector<uint8_t> const & data)
 * \brief Converts application-specific meta data into a human-readable string.
 *
 * Depending on the application of the object dictionary, application-specific meta data may be attached to some or
 * all subindices.
 *
 * Derived classes may implement this method to convert the application-specific meta data into a human-readable string
 * according to the custom structure, type, and format of the application-specific meta data. The default implementation
 * provided by base class [RODACLIClientBase](@ref gpcc::cood::RODACLIClientBase) will convert the application-specific
 * meta data into hexadecimal byte values.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This will be invoked by the CLI thread only.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param data
 * Unmodifiable reference to an std::vector<uint8_t> containing the application-specific meta data.
 *
 * \return
 * String object containing a human-readable string representation of parameter 'data'.
 */

} // namespace cood
} // namespace gpcc

#endif // RODACLICLIENTBASE_HPP_202105221835
