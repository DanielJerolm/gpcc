/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef SINGLERODACLICLIENTBASE_HPP_202105221903
#define SINGLERODACLICLIENTBASE_HPP_202105221903

#include <gpcc/cood/remote_access/infrastructure/RODACLIClientBase.hpp>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Base class for classes offering CLI-access to a single RODA interface.
 *
 * The CLI command will be registered when this class is instantiated. At the same time the instance will connect to a
 * RODA interface. The instance remains connected to the RODA interface until it is destroyed.
 *
 * The offered CLI command allows to:
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
 * implementing an own class: @ref SingleRODACLIClient.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class SingleRODACLIClientBase : private RODACLIClientBase
{
  public:
    SingleRODACLIClientBase(void) = delete;
    SingleRODACLIClientBase(SingleRODACLIClientBase const &) = delete;
    SingleRODACLIClientBase(SingleRODACLIClientBase &&) = delete;

    SingleRODACLIClientBase& operator=(SingleRODACLIClientBase const &) = delete;
    SingleRODACLIClientBase& operator=(SingleRODACLIClientBase &&) = delete;

  protected:
    SingleRODACLIClientBase(IRemoteObjectDictionaryAccess & _rodaItf,
                            gpcc::cli::CLI & _cli,
                            std::string const & _cmdName,
                            uint8_t const _attributeStringMaxLength);
    virtual ~SingleRODACLIClientBase(void) override;

  private:
    /// RODA interface where this client shall connect to.
    IRemoteObjectDictionaryAccess & rodaItf;

    /// Name of the published CLI command.
    std::string const cmdName;


    void CLICommandHandler(std::string const & restOfLine, gpcc::cli::CLI & cli);
};

} // namespace cood
} // namespace gpcc

#endif // SINGLERODACLICLIENTBASE_HPP_202105221903
