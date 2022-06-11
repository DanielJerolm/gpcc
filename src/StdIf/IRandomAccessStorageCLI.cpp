/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "IRandomAccessStorageCLI.hpp"
#include "IRandomAccessStorage.hpp"
#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/string/tools.hpp"
#include <limits>
#include <vector>

namespace gpcc
{
namespace StdIf
{

void CliCmdReadIRandomAccessStorage(std::string const & restOfLine,
                                    gpcc::cli::CLI & cli,
                                    IRandomAccessStorage* const pRAS)
/**
 * \ingroup GPCC_STDIF_CLI
 * \brief [CLI](@ref gpcc::cli::CLI) command for reading from an @ref IRandomAccessStorage interface.
 *
 * Usage example:
 * ~~~{.cpp}
 * using gpcc::cli::Command;
 * cli.AddCommand(Command::Create("rdeeprom", " 0xADDRESS n\n"\
 *                                             "Reads n bytes from EEPROM, starting at address 0xADDRESS and dumps\n"\
 *                                             "the data to the terminal.",
 *                                std::bind(&gpcc::StdIf::CliCmdReadIRandomAccessStorage,
 *                                          std::placeholders::_1,
 *                                          std::placeholders::_2,
 *                                          pMyRASInterface)));
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - content of terminal's screen maybe incomplete
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen maybe incomplete
 *
 * ---
 *
 * \param restOfLine
 * Arguments passed to the CLI command.
 * \param cli
 * `gpcc::cli::CLI` instance executing this.
 * \param pRAS
 * Pointer to the @ref IRandomAccessStorage interface accessed through this command handler.
 */
{
  auto params = gpcc::string::Split(restOfLine, ' ', true);

  if (params.size() != 2)
  {
    cli.WriteLine("Error: 2 parameters expected!\nTry 'rdeeprom help'");
    return;
  }

  // read parameters into "address" and "n"
  uint32_t address;
  uint16_t n;

  try
  {
    auto it = params.begin();
    long long value;

    // address
    if (!gpcc::string::StartsWith(*it, "0x"))
      throw std::exception();

    value = std::stoll((*it).substr(2), nullptr, 16);
    if ((value < 0) || (value > std::numeric_limits<uint32_t>::max()))
      throw std::exception();
    address = value;
    ++it;

    // number of bytes
    value = std::stoll(*it);
    if ((value < 0) || (value > 1024U))
      throw std::exception();
    n = value;

    if (n == 0)
      return;

    // check: address overflow?
    if (std::numeric_limits<uint32_t>::max() - address < n - 1U)
      throw std::exception();
  }
  catch (std::exception const &)
  {
    cli.WriteLine("Error: Invalid parameter(s)");
    return;
  }

  params.clear();

  // check "address" and "n" against properties of pRAS
  if (address + (n - 1U) >= pRAS->GetSize())
  {
    cli.WriteLine("Error: Attempt to read out of bounds");
    return;
  }

  // allocate buffer and read
  std::vector<uint8_t> buffer;
  buffer.resize(n);
  pRAS->Read(address, n, buffer.data());

  // print to CLI
  cli.WriteLine("Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");

  uint32_t offset = 0;
  while (n != 0)
  {
    size_t bytes = n;
    if (bytes > 16U)
      bytes = 16U;
    cli.WriteLine(gpcc::string::HexDump(address, buffer.data() + offset, bytes, 1, 16));
    n -= bytes;
    address += bytes;
    offset += bytes;
  }
}

void CliCmdWriteIRandomAccessStorage(std::string const & restOfLine,
                                     gpcc::cli::CLI & cli,
                                     IRandomAccessStorage* const pRAS)
/**
 * \ingroup GPCC_STDIF_CLI
 * \brief [CLI](@ref gpcc::cli::CLI) command for writing to an @ref IRandomAccessStorage interface.
 *
 * Usage example:
 * ~~~{.cpp}
 * using gpcc::cli::Command;
 * cli.AddCommand(Command::Create("wreeprom", " 0xADDRESS [0x]Data1 [[0x]Data2 .. [0x]DataN]\n"\
 *                                            "Writes n bytes of data to the EEPROM, starting at address 0xADDRESS",
 *                                std::bind(&gpcc::StdIf::CliCmdWriteIRandomAccessStorage,
 *                                          std::placeholders::_1,
 *                                          std::placeholders::_2,
 *                                          pMyRASInterface)));
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - content of terminal's screen maybe incomplete
 * - data written to @ref IRandomAccessStorage may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen maybe incomplete
 * - data written to @ref IRandomAccessStorage may be incomplete
 *
 * ---
 *
 * \param restOfLine
 * Arguments passed to the CLI command.
 * \param cli
 * `gpcc::cli::CLI` instance executing this.
 * \param pRAS
 * Pointer to the @ref IRandomAccessStorage interface accessed through this command handler.
 */
{
  auto params = gpcc::string::Split(restOfLine, ' ', true);

  if (params.size() < 2)
  {
    cli.WriteLine("Error: At least 2 parameters expected!\nTry 'wreeprom help'");
    return;
  }

  // read parameters into "address" and "data".
  uint32_t address;
  std::vector<uint8_t> data;
  data.reserve(params.size() - 1U);

  try
  {
    auto it = params.begin();
    long long value;

    // address
    if (!gpcc::string::StartsWith(*it, "0x"))
      throw std::exception();

    value = std::stoll((*it).substr(2), nullptr, 16);
    if ((value < 0) || (value > std::numeric_limits<uint32_t>::max()))
      throw std::exception();
    address = value;
    ++it;

    // read data
    while (it != params.end())
    {
      if (gpcc::string::StartsWith(*it, "0x"))
        value = std::stoll((*it).substr(2), nullptr, 16);
      else if (gpcc::string::StartsWith(*it, "'"))
      {
        if (((*it).length() != 3) ||
            ((*it)[2] != '\''))
          throw std::exception();
        data.push_back(static_cast<uint8_t>((*it)[1]));
      }
      else if (gpcc::string::StartsWith(*it, "-"))
        throw std::exception();
      else
        value = std::stoll(*it);
      if ((value < 0) || (value > std::numeric_limits<uint8_t>::max()))
        throw std::exception();
      data.push_back(static_cast<uint8_t>(value));
      ++it;
    }

    // check: address overflow?
    if (std::numeric_limits<uint32_t>::max() - address < data.size() - 1U)
      throw std::exception();
  }
  catch (std::exception const &)
  {
    cli.WriteLine("Error: Invalid parameter(s)");
    return;
  }

  params.clear();

  // check "address" and "n" against properties of pRAS
  if (address + (data.size() - 1U) >= pRAS->GetSize())
  {
    cli.WriteLine("Error: Attempt to write out of bounds");
    return;
  }

  // write
  pRAS->Write(address, data.size(), data.data());
}

} // namespace StdIf
} // namespace gpcc
