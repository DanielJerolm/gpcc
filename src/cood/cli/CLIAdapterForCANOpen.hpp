/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019, 2022 Daniel Jerolm

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

#ifndef CLIADAPTERFORCANOPEN_HPP_201905172140
#define CLIADAPTERFORCANOPEN_HPP_201905172140

#include "CLIAdapterBase.hpp"

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
