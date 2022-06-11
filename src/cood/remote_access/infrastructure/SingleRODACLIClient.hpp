/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#ifndef SINGLERODACLICLIENT_HPP_202105280815
#define SINGLERODACLICLIENT_HPP_202105280815

#include "SingleRODACLIClientBase.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Offers CLI-access to a single RODA interface. Output is configurable for CANopen and EtherCAT.
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
 * Application specific meta data potentially attached to objects is printed as hexadecimal binary.\n
 * If a specific conversion of application specific meta data or object attributes is required, then clients shall
 * derive a own class from @ref SingleRODACLIClientBase. This class can be used as a template.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class SingleRODACLIClient final : private SingleRODACLIClientBase
{
  public:
    SingleRODACLIClient(void) = delete;
    SingleRODACLIClient(IRemoteObjectDictionaryAccess & _rodaItf,
                        gpcc::cli::CLI & _cli,
                        std::string const & _cmdName,
                        bool const _ethercatStyleNotCanOpenStyle);
    SingleRODACLIClient(SingleRODACLIClient const &) = delete;
    SingleRODACLIClient(SingleRODACLIClient &&) = delete;
    ~SingleRODACLIClient(void) override = default;

    SingleRODACLIClient& operator=(SingleRODACLIClient const &) = delete;
    SingleRODACLIClient& operator=(SingleRODACLIClient &&) = delete;

  private:
    /// Configured style for output.
    /** true = EtherCAT\n
        false = CANopen */
    bool const ethercatStyleNotCanOpenStyle;


    std::string AttributesToStringHook(Object::attr_t const attributes) override;
};

} // namespace cood
} // namespace gpcc

#endif // SINGLERODACLICLIENT_HPP_202105280815