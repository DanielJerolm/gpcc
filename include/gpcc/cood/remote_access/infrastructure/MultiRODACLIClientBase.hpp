/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef MULTIRODACLICLIENTBASE_HPP_202106121953
#define MULTIRODACLICLIENTBASE_HPP_202106121953

#include <gpcc/cood/remote_access/infrastructure/IMultiRODACLIClient.hpp>
#include <gpcc/cood/remote_access/infrastructure/RODACLIClientBase.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <unordered_map>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Base class for classes offering CLI-access to multiple RODA interfaces.
 *
 * The CLI command will be registered upon instantiation of this class.
 *
 * After instantiation, RODA interfaces can be registered and unregistered via interface @ref IMultiRODACLIClient at
 * any time. Before destruction, all RODA interfaces must be unregistered again. The different RODA interfaces
 * are distinguished by a 32bit ID. See @ref IMultiRODACLIClient for details.
 *
 * The offered CLI command allows to access the object dictionaries, which are accessible through the registered
 * RODA interfaces. The following types of access are offered via CLI:
 * - enumerate objects
 * - query object's meta data
 * - read subindices
 * - write subindices
 * - read objects (complete access)
 * - write objects (complete access)
 *
 * The name of the CLI command is configurable. There is only one CLI command. The type of access is passed as argument
 * by the user.
 *
 * This shall be subclassed in order to specialize the textual output of...
 * - ...object's attributes to EtherCAT encoding, CANopen encoding, or a custom encoding.
 * - ..._application specific meta data_ that may be attached to objects.
 *
 * Subclasses shall implement @ref RODACLIClientBase::AttributesToStringHook() to generate appropriate strings from
 * object's attributes.
 *
 * Subclasses may overwrite @ref RODACLIClientBase::AppSpecificMetaDataToStringHook() to generate specialized strings
 * from _application specific meta data_ that might be attached to objects. If the method is not overwritten, then
 * any _application specific meta data_ will be displayed as binary in hexadecimal format.
 *
 * There is a sub-class prepared for EtherCAT and CANopen available, that can also be used as a template for
 * implementing an own class: @ref MultiRODACLIClient.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class MultiRODACLIClientBase : public IMultiRODACLIClient, private RODACLIClientBase
{
  public:
    MultiRODACLIClientBase(void) = delete;
    MultiRODACLIClientBase(MultiRODACLIClientBase const &) = delete;
    MultiRODACLIClientBase(MultiRODACLIClientBase &&) = delete;

    MultiRODACLIClientBase& operator=(MultiRODACLIClientBase const &) = delete;
    MultiRODACLIClientBase& operator=(MultiRODACLIClientBase &&) = delete;

    // <-- IMultiRODACLIClient
    void Register(IRemoteObjectDictionaryAccess & rodaItf, uint32_t const id) override;
    void Unregister(uint32_t const id) override;
    // --> IMultiRODACLIClient

  protected:
    MultiRODACLIClientBase(gpcc::cli::CLI & _cli,
                           std::string const & _cmdName,
                           uint8_t const _attributeStringMaxLength);
    virtual ~MultiRODACLIClientBase(void) override;

  private:
    /// Timeout when waiting for the connected RODA interface to enter the ready-state.
    static uint16_t constexpr rodaReadyTimeout_ms = 1000U;


    /// Name of the published CLI command.
    std::string const cmdName;

    /// Mutex used to make registration, unregistration and querying of RODA interfaces thread-safe.
    /** Locking order: @ref rodaItfMutex -> base class' mutexes */
    gpcc::osal::Mutex rodaItfMutex;

    /// Container for registered RODA interfaces.
    /** @ref rodaItfMutex required. */
    std::unordered_map<uint32_t, IRemoteObjectDictionaryAccess*> registeredRODAItfs;


    void CLICommandHandler(std::string const & restOfLine, gpcc::cli::CLI & cli);
};

} // namespace cood
} // namespace gpcc

#endif // MULTIRODACLICLIENTBASE_HPP_202106121953
