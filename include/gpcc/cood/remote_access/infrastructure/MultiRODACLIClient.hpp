/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef MULTIRODACLICLIENT_HPP_202106122044
#define MULTIRODACLICLIENT_HPP_202106122044

#include <gpcc/cood/remote_access/infrastructure/MultiRODACLIClientBase.hpp>

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