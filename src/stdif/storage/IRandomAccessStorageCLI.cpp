/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/stdif/storage/IRandomAccessStorageCLI.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/exceptions.hpp>
#include <gpcc/stdif/storage/IRandomAccessStorage.hpp>
#include <gpcc/string/tools.hpp>
#include <limits>
#include <vector>

namespace gpcc  {
namespace stdif {

/**
 * \ingroup GPCC_STDIF_STORAGE
 * \brief [CLI](@ref gpcc::cli::CLI) command for reading from an @ref IRandomAccessStorage interface.
 *
 * Usage example for an EEPROM:
 * ~~~{.cpp}
 * using gpcc::cli::Command;
 * cli.AddCommand(Command::Create("rdeeprom", " 0xADDRESS n\n"\
 *                                             "Reads n (1..1024) bytes from EEPROM, starting at address 0xADDRESS\n"\
 *                                             "and dumps the data to the terminal.",
 *                                std::bind(&gpcc::stdif::CliCmdReadIRandomAccessStorage,
 *                                          std::placeholders::_1,
 *                                          std::placeholders::_2,
 *                                          pMyRASInterface)));
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:\n
 * - content of terminal's screen maybe incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen maybe incomplete
 *
 * - - -
 *
 * \param restOfLine
 * Arguments passed to the CLI command.
 *
 * \param cli
 * `gpcc::cli::CLI` instance executing this.
 *
 * \param pRAS
 * Pointer to the @ref IRandomAccessStorage interface accessed through this command handler.
 */
void CliCmdReadIRandomAccessStorage(std::string const & restOfLine,
                                    gpcc::cli::CLI & cli,
                                    IRandomAccessStorage* const pRAS)
{
  auto params = gpcc::string::Split(restOfLine, ' ', true);

  if (params.size() != 2U)
    throw gpcc::cli::UserEnteredInvalidArgsError();

  // read parameters into "address" and "n"
  uint32_t address;
  uint16_t n;

  try
  {
    auto it = params.begin();

    // address
    address = gpcc::string::HexToU32(*it);
    ++it;

    // number of bytes
    n = gpcc::string::AnyNumberToU32(*it, 0U, 1024U);
    if (n == 0U)
      return;

    // check: address overflow?
    if ((std::numeric_limits<uint32_t>::max() - address) < (n - 1U))
      throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");
  }
  catch (gpcc::cli::UserEnteredInvalidArgsError const &)
  {
    throw;
  }
  catch (std::bad_alloc const &)
  {
    throw;
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
  }

  params.clear();

  // check "address" and "n" against properties of pRAS
  if ((address + (n - 1U)) >= pRAS->GetSize())
    throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");

  // allocate buffer and read
  std::vector<uint8_t> buffer;
  buffer.resize(n);
  pRAS->Read(address, n, buffer.data());

  // print to CLI
  cli.WriteLine("Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");

  uint32_t offset = 0U;
  while (n != 0U)
  {
    size_t bytes = n;
    if (bytes > 16U)
      bytes = 16U;
    cli.WriteLine(gpcc::string::HexDump(address, buffer.data() + offset, bytes, 1U, 16U));
    n -= bytes;
    address += bytes;
    offset += bytes;
  }
}

/**
 * \ingroup GPCC_STDIF_STORAGE
 * \brief [CLI](@ref gpcc::cli::CLI) command for writing to an @ref IRandomAccessStorage interface.
 *
 * Usage example for an EEPROM:
 * ~~~{.cpp}
 * using gpcc::cli::Command;
 * cli.AddCommand(Command::Create("wreeprom", " 0xADDRESS [0x]Data1 [[0x]Data2 .. [0x]DataN]\n"\
 *                                            "Writes Data1..DataN to the EEPROM, starting at address 0xADDRESS",
 *                                std::bind(&gpcc::stdif::CliCmdWriteIRandomAccessStorage,
 *                                          std::placeholders::_1,
 *                                          std::placeholders::_2,
 *                                          pMyRASInterface)));
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:\n
 * - content of terminal's screen maybe incomplete
 * - incomplete data may be written to the underlying storage
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen maybe incomplete
 * - incomplete data may be written to the underlying storage
 *
 * - - -
 *
 * \param restOfLine
 * Arguments passed to the CLI command.
 *
 * \param cli
 * `gpcc::cli::CLI` instance executing this.
 *
 * \param pRAS
 * Pointer to the @ref IRandomAccessStorage interface accessed through this command handler.
 */
void CliCmdWriteIRandomAccessStorage(std::string const & restOfLine,
                                     gpcc::cli::CLI & cli,
                                     IRandomAccessStorage* const pRAS)
{
  (void)cli;

  auto params = gpcc::string::Split(restOfLine, ' ', true);

  if (params.size() < 2U)
    throw gpcc::cli::UserEnteredInvalidArgsError();

  // read parameters into "address" and "data".
  uint32_t address;
  std::vector<uint8_t> data;
  data.reserve(params.size() - 1U);

  try
  {
    auto it = params.begin();

    // address
    address = gpcc::string::HexToU32(*it);
    ++it;

    // read data
    while (it != params.end())
    {
      data.push_back(gpcc::string::AnyStringToU8(*it));
      ++it;
    }

    // check: address overflow?
    if ((std::numeric_limits<uint32_t>::max() - address) < (data.size() - 1U))
      throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");
  }
  catch (gpcc::cli::UserEnteredInvalidArgsError const &)
  {
    throw;
  }
  catch (std::bad_alloc const &)
  {
    throw;
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
  }

  params.clear();

  // check "address" and "n" against properties of pRAS
  if ((address + (data.size() - 1U)) >= pRAS->GetSize())
    throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");

  // write
  pRAS->Write(address, data.size(), data.data());
}

} // namespace stdif
} // namespace gpcc
