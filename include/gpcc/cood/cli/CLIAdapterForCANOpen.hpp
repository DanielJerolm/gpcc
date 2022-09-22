/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#ifndef CLIADAPTERFORCANOPEN_HPP_201905172140
#define CLIADAPTERFORCANOPEN_HPP_201905172140

#include <gpcc/cood/cli/CLIAdapterBase.hpp>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_CLI
 * \brief This class offers access to an object dictionary via [CLI](@ref gpcc::cli::CLI) in a CANopen application.
 *
 * The actual functionality is offered by base class @ref CLIAdapterBase. See class @ref CLIAdapterBase for details.
 *
 * This class just provides an implementation for the virtual methods defined in @ref CLIAdapterBase. The virtual
 * methods fine-tune the behavior according to the needs of a specific application.
 *
 * The implementation provided by this class is tailored to a CANopen application.\n
 * __This adapter is not suitable for use in a EtherCAT application.__
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class CLIAdapterForCANOpen final : public CLIAdapterBase
{
  public:
    CLIAdapterForCANOpen(void) = delete;
    CLIAdapterForCANOpen(IObjectAccess & _od,
                         gpcc::cli::CLI & _cli,
                         std::string const & _cmdName);
    CLIAdapterForCANOpen(CLIAdapterForCANOpen const &) = delete;
    CLIAdapterForCANOpen(CLIAdapterForCANOpen &&) = delete;
    ~CLIAdapterForCANOpen(void) override;

    CLIAdapterForCANOpen& operator=(CLIAdapterForCANOpen const &) = delete;
    CLIAdapterForCANOpen& operator=(CLIAdapterForCANOpen &&) = delete;

  private:
    Object::attr_t BeginAccessHook(void) override;
    void EndAccessHook(void) noexcept override;
    std::string AttributesToStringHook(Object::attr_t const attributes) override;
};

} // namespace cood
} // namespace gpcc

#endif // CLIADAPTERFORCANOPEN_HPP_201905172140
