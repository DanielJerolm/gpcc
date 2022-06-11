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

#ifndef MULTIRODACLICLIENT_HPP_202106122044
#define MULTIRODACLICLIENT_HPP_202106122044

#include "MultiRODACLIClientBase.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Offers CLI-access to multiple RODA interfaces. Output is configurable for CANopen and EtherCAT.
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
 * Application specific meta data potentially attached to objects is printed as hexadecimal binary.\n
 * If a specific conversion of application specific meta data or object attributes is required, then clients shall
 * derive an own class from @ref MultiRODACLIClientBase. This class can be used as a template.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class MultiRODACLIClient final : public MultiRODACLIClientBase
{
  public:
    MultiRODACLIClient(void) = delete;
    MultiRODACLIClient(gpcc::cli::CLI & _cli,
                       std::string const & _cmdName,
                       bool const _ethercatStyleNotCanOpenStyle);
    MultiRODACLIClient(MultiRODACLIClient const &) = delete;
    MultiRODACLIClient(MultiRODACLIClient &&) = delete;
    ~MultiRODACLIClient(void) override = default;

    MultiRODACLIClient& operator=(MultiRODACLIClient const &) = delete;
    MultiRODACLIClient& operator=(MultiRODACLIClient &&) = delete;

  private:
    /// Configured style for output.
    /** true = EtherCAT\n
        false = CANopen */
    bool const ethercatStyleNotCanOpenStyle;


    std::string AttributesToStringHook(Object::attr_t const attributes) override;
};

} // namespace cood
} // namespace gpcc

#endif // MULTIRODACLICLIENT_HPP_202106122044