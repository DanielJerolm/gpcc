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

#ifndef SINGLERODACLICLIENTBASE_HPP_202105221903
#define SINGLERODACLICLIENTBASE_HPP_202105221903

#include "RODACLIClientBase.hpp"

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
