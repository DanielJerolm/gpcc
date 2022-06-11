/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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

#ifndef MULTIRODACLICLIENTBASE_HPP_202106121953
#define MULTIRODACLICLIENTBASE_HPP_202106121953

#include "IMultiRODACLIClient.hpp"
#include "RODACLIClientBase.hpp"
#include "gpcc/src/osal/Mutex.hpp"
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
