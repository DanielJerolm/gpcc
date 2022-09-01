/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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