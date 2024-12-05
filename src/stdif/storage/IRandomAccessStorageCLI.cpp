/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
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
 * \brief [CLI](@ref GPCC_CLI) command handler for reading from an @ref IRandomAccessStorage interface.
 *
 * Example using this command handler to read from an EEPROM accessible via `pMyRASInterface`:
 * ~~~{.cpp}
 * using gpcc::cli::Command;
 * cli.AddCommand(Command::Create("rdeeprom", " 0xADDRESS n\n"\
 *                                "Reads n (1..1024) bytes from EEPROM, starting at address 0xADDRESS\n"\
 *                                "and dumps the data to the terminal.",
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
 * @ref gpcc::cli::CLI instance invoking this.
 *
 * \param pRAS
 * Pointer to the @ref IRandomAccessStorage interface accessed through this command handler.
 */
void CliCmdReadIRandomAccessStorage(std::string const & restOfLine,
                                    gpcc::cli::CLI & cli,
                                    IRandomAccessStorage* const pRAS)
{
  // read parameters into "address" and "n"
  uint32_t address;
  size_t n;

  auto params = gpcc::string::Split(restOfLine, ' ', true);
  if (params.size() != 2U)
    throw gpcc::cli::UserEnteredInvalidArgsError();

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
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
  }

  params.clear();

  // check for address overflow
  if ((std::numeric_limits<decltype(address)>::max() - address) < (n - 1U))
    throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");

  // check "address" and "n" against properties of pRAS
  if ((address + (n - 1U)) >= pRAS->GetSize())
    throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");

  // allocate buffer and read from pRAS
  std::vector<uint8_t> buffer(n);
  pRAS->Read(address, n, buffer.data());

  // print to CLI
  cli.WriteLine("Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");

  static_assert(sizeof(uintptr_t) >= sizeof(decltype(address)));
  uintptr_t address_uiptr = address;
  void const * pData = buffer.data();
  while (n != 0U)
    cli.WriteLine(gpcc::string::HexDump(address_uiptr, 8U, pData, n, 1U, 16U));
}

/**
 * \ingroup GPCC_STDIF_STORAGE
 * \brief [CLI](@ref GPCC_CLI) command handler for writing to an @ref IRandomAccessStorage interface.
 *
 * Example using this command handler to write to an EEPROM accessible via `pMyRASInterface`:
 * ~~~{.cpp}
 * using gpcc::cli::Command;
 * cli.AddCommand(Command::Create("wreeprom", " 0xADDRESS [0x]Data1 [[0x]Data2 .. [0x]DataN]\n"\
 *                                "Writes Data1..DataN to the EEPROM, starting at address 0xADDRESS",
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
 * - incomplete data may be written to the underlying storage, depends on guarantee provided by @p pRAS
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen maybe incomplete
 * - incomplete data may be written to the underlying storage, depends on guarantee provided by @p pRAS
 *
 * - - -
 *
 * \param restOfLine
 * Arguments passed to the CLI command.
 *
 * \param cli
 * @ref gpcc::cli::CLI instance invoking this.
 *
 * \param pRAS
 * Pointer to the @ref IRandomAccessStorage interface accessed through this command handler.
 */
void CliCmdWriteIRandomAccessStorage(std::string const & restOfLine,
                                     gpcc::cli::CLI & cli,
                                     IRandomAccessStorage* const pRAS)
{
  (void)cli;

  // read parameters into "address" and "data".
  uint32_t address;
  std::vector<uint8_t> data;

  auto params = gpcc::string::Split(restOfLine, ' ', true);
  if (params.size() < 2U)
    throw gpcc::cli::UserEnteredInvalidArgsError();

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
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
  }

  params.clear();

  // check for address overflow
  if ((std::numeric_limits<decltype(address)>::max() - address) < (data.size() - 1U))
    throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");

  // check "address" and number of bytes against properties of pRAS
  if ((address + (data.size() - 1U)) >= pRAS->GetSize())
    throw gpcc::cli::UserEnteredInvalidArgsError("Address out of bounds");

  // write
  pRAS->Write(address, data.size(), data.data());
}

} // namespace stdif
} // namespace gpcc
