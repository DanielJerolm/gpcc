/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/file_systems/eeprom_section_system/cli/ESSCLICommands.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/file_systems/eeprom_section_system/EEPROMSectionSystem.hpp>

namespace gpcc
{
namespace file_systems
{
namespace eeprom_section_system
{

void CLICMDGetState(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS)
/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI
 * \brief CLI command handler: Prints the state of an @ref EEPROMSectionSystem instance.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("ess_getState", "\nRetrieves EEPROM Section System's state.",
 *                  std::bind(&gpcc::file_systems::eeprom_section_system::CLICMDGetState,
 *                  std::placeholders::_1, std::placeholders::_2, pESS));
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - content of terminal's screen could be undefined
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen could be undefined
 *
 * ---
 *
 * \param restOfLine
 * Any stuff entered behind the command. This is required by @ref gpcc::cli::CLI. \n
 * This function does not expect any arguments entered on the CLI.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pESS
 * Pointer to the @ref EEPROMSectionSystem instance whose state shall be retrieved.
 */
{
  if (restOfLine.length() != 0)
  {
    cli.WriteLine("Error: No parameters expected");
    return;
  }

  cli.WriteLine(EEPROMSectionSystem::States2String(pESS->GetState()));
}
void CLICMDFormat(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS, uint16_t blockSize)
/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI
 * \brief CLI command handler: Formats an @ref EEPROMSectionSystem instance.
 *
 * Before formatting, the user is requested to enter "yes" into the CLI.\n
 * The EEPROM Section System must be in state
 * (States::not_mounted)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::not_mounted].
 * Otherwise formatting will be refused.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("ess_format", "\nFormats an EEPROM Section System.",
 *                  std::bind(&gpcc::file_systems::eeprom_section_system::CLICMDFormat,
 *                  std::placeholders::_1, std::placeholders::_2, pESS, blockSize));
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the storage content may be undefined.
 * - the state of the EEPROM Section System may be (States::not_mounted)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::not_mounted].
 * - content of terminal's screen could be undefined
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the storage content may be undefined.
 * - the state of the EEPROM Section System may be (States::not_mounted)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::not_mounted].
 * - content of terminal's screen could be undefined
 *
 * ---
 *
 * \param restOfLine
 * Any stuff entered behind the command. This is required by @ref gpcc::cli::CLI. \n
 * This function does not expect any arguments entered on the CLI.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pESS
 * Pointer to the @ref EEPROMSectionSystem instance which shall be formatted.
 * \param blockSize
 * Desired block size in bytes.
 */
{
  if (restOfLine.length() != 0)
  {
    cli.WriteLine("Error: No parameters expected");
    return;
  }

  // ask user if he/she is really sure to format the storage
  auto answer = cli.ReadLine("Format storage, all data will be lost! Sure? (yes/no):");

  if (answer != "yes")
  {
    cli.WriteLine("Aborted. Storage has not been touched.");
    return;
  }

  // check if unmounted
  if (pESS->GetState() != EEPROMSectionSystem::States::not_mounted)
  {
    cli.WriteLine("Error: EEPROMSectionSystem must be unmounted!");
    return;
  }

  // format storage
  cli.WriteLine("Formatting EEPROMSectionSystem with block size " + std::to_string(blockSize) + " bytes.\n"\
                "This make take a few seconds...");
  pESS->Format(blockSize);
  cli.WriteLine("Done");
}
void CLICMDUnmount(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS)
/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI
 * \brief CLI command handler: Unmounts an @ref EEPROMSectionSystem instance.
 *
 * The EEPROM Section System may be in any state when this is invoked.\n
 * Unmount will be refused if any section is still open for reading or writing.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("ess_unmount", "\nUnmounts an EEPROM Section System.",
 *                  std::bind(&gpcc::file_systems::eeprom_section_system::CLICMDUnmount,
 *                  std::placeholders::_1, std::placeholders::_2, pESS));
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - content of terminal's screen could be undefined
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen could be undefined
 *
 * ---
 *
 * \param restOfLine
 * Any stuff entered behind the command. This is required by @ref gpcc::cli::CLI. \n
 * This function does not expect any arguments entered on the CLI.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pESS
 * Pointer to the @ref EEPROMSectionSystem instance which shall be unmounted.
 */
{
  if (restOfLine.length() != 0)
  {
    cli.WriteLine("Error: No parameters expected");
    return;
  }

  if (pESS->GetState() != EEPROMSectionSystem::States::not_mounted)
    pESS->Unmount();
  cli.WriteLine("Unmounted");
}
void CLICMDMount(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS)
/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI
 * \brief CLI command handler: Mounts an @ref EEPROMSectionSystem instance for rw-access and performs
 * integrity checks and any necessary repair operations.
 *
 * The @ref EEPROMSectionSystem may be in one of the following states:
 * - (States::not_mounted)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::not_mounted]\n
 *   The EEPROM Section System will be mounted in two steps: First ro-acccess then rw-access
 * - (States::ro_mount)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::ro_mount]\n
 *   The EEPROM Section System will be mounted for rw-access
 * - (States::defect)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::defect]\n
 *   The EEPROM Section System will be checked, repaired (if necessary), and mounted for rw-access
 * - (States::mounted)[@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem::States::mounted]\n
 *   The EEPROM Section System will be checked, repaired (if necessary), and mounted for rw-access
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("ess_mount", "\nMounts an EEPROM Section System for read-write access.\n"\
 *                                                          "Mounting includes full integrity checks and any necessary repair operations.",
 *                  std::bind(&gpcc::file_systems::eeprom_section_system::CLICMDMount,
 *                  std::placeholders::_1, std::placeholders::_2, pESS));
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - content of terminal's screen could be undefined
 * - If an exception is thrown while mount step 2 (rw-access) repairs the section system, then the storage content
 *   could be left in an invalid state that differs from the state before invoking this command. However, things
 *   cannot get worse than they have been when before this was invoked. A subsequent call to this is able to
 *   recover the section system if the underlying storage is physically OK.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen could be undefined
 * - the storage content could be left in an invalid state if the thread is cancelled while mount step 2
 *   (rw-access) repairs the section system. A subsequent call to this is able to recover the section system
 *   if the underlying storage is physically OK.
 *
 * ---
 *
 * \param restOfLine
 * Any stuff entered behind the command. This is required by @ref gpcc::cli::CLI. \n
 * This function does not expect any arguments entered on the CLI.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pESS
 * Pointer to the @ref EEPROMSectionSystem instance which shall be mounted for read-write-access.
 */
{
  if (restOfLine.length() != 0)
  {
    cli.WriteLine("Error: No parameters expected");
    return;
  }

  if (pESS->GetState() == EEPROMSectionSystem::States::not_mounted)
  {
    cli.WriteLine("Mounting for ro-access...");
    pESS->MountStep1();
    cli.WriteLine("Mounted for ro-access.");
  }

  cli.WriteLine("Mounting for rw-access...");
  pESS->MountStep2();
  cli.WriteLine("Mounted for rw-access.");
}

} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc
