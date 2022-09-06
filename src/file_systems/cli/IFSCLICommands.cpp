/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/file_systems/cli/IFSCLICommands.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/file_systems/IFileStorage.hpp>
#include <gpcc/file_systems/exceptions.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include <cstddef>

namespace gpcc
{
namespace file_systems
{

void CLICMDDelete(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS)
/**
 * \ingroup GPCC_FILESYSTEMS_CLI
 * \brief CLI command handler: Deletes a file (or EEPROM section or similar).
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("file_delete", " Fullname1 [Fullname2..Fullname n]\n"\
 *                                                            "Deletes one more files.",
 *                  std::bind(&gpcc::file_systems::CLICMDDelete,
 *                  std::placeholders::_1, std::placeholders::_2, pIFS));
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
 * - not all files may have been deleted
 * - the file system implementation may switch to a "defect state".
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen could be undefined
 * - not all files may have been deleted
 * - the file system implementation may switch to a "defect state".
 *
 * ---
 *
 * \param restOfLine
 * Arguments entered behind the command.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pIFS
 * Pointer to the @ref IFileStorage interface used to access the files.
 */
{
  auto const files = gpcc::string::Split(restOfLine, ' ', true);
  if (files.size() == 0)
  {
    cli.WriteLine("Error: At least one parameter expected!\nTry 'file_delete help'");
    return;
  }

  for (auto const & e: files)
  {
    try
    {
      pIFS->Delete(e);
      cli.WriteLine("Deleted: " + e);
    }
    catch (NoSuchFileError const &)
    {
      cli.WriteLine("Error, no such file: " + e);
    }
  }
}
void CLICMDRename(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS)
/**
 * \ingroup GPCC_FILESYSTEMS_CLI
 * \brief CLI command handler: Renames a file (or EEPROM section or similar).
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("file_rename", " old_name new_name\n"\
 *                                                            "Renames a file.",
 *                  std::bind(&gpcc::file_systems::CLICMDRename,
 *                  std::placeholders::_1, std::placeholders::_2, pIFS));
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
 * - the file system implementation may switch to a "defect state".
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen could be undefined
 * - the file system implementation may switch to a "defect state".
 *
 * ---
 *
 * \param restOfLine
 * Arguments entered behind the command.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pIFS
 * Pointer to the @ref IFileStorage interface used to access the files.
 */
{
  auto const args = gpcc::string::Split(restOfLine, ' ', true);
  if (args.size() != 2)
  {
    cli.WriteLine("Error: Two arguments expected!\nTry 'file_rename help'");
    return;
  }

  pIFS->Rename(args[0], args[1]);
}
void CLICMDEnumerate(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS)
/**
 * \ingroup GPCC_FILESYSTEMS_CLI
 * \brief CLI command handler: Enumerates all files stored in an @ref IFileStorage.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("file_enumerate", " [-s]\n"\
 *                                                               "Enumerates all files.\n"\
 *                                                               "If -s is given, then file sizes are also shown.",
 *                  std::bind(&gpcc::file_systems::CLICMDEnumerate,
 *                  std::placeholders::_1, std::placeholders::_2, pIFS));
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
 * Arguments entered behind the command.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pIFS
 * Pointer to the @ref IFileStorage interface used to access the files.
 */
{
  bool showSizes = false;
  if (restOfLine == "-s")
    showSizes = true;
  else if (restOfLine.length() != 0)
  {
    cli.WriteLine("Error: Bad arguments!\nTry 'file_enumerate help'");
    return;
  }

  auto const files = pIFS->Enumerate();

  if (showSizes)
  {
    for (auto const & e: files)
    {
      size_t const size = pIFS->DetermineSize(e, nullptr);
      cli.WriteLine(e + " (" + std::to_string(size) + " byte)");
    }
  }
  else
  {
    for (auto const & e: files)
      cli.WriteLine(e);
  }

  cli.WriteLine(std::to_string(files.size()) + " files");
}
void CLICMDFreeSpace(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS)
/**
 * \ingroup GPCC_FILESYSTEMS_CLI
 * \brief CLI command handler: Retrieves the amounth of free space.
 *
 * Note:\n
 * The exact meaning of the retrieved value depends on the underlying file system.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("file_freespace", "\nDetermines the amounth of free space.",
 *                  std::bind(&gpcc::file_systems::CLICMDFreeSpace,
 *                  std::placeholders::_1, std::placeholders::_2, pIFS));
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
 * Arguments entered behind the command. This command handler expects no arguments.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pIFS
 * Pointer to the @ref IFileStorage interface used to access the files.
 */
{
  if (restOfLine.length() != 0)
  {
    cli.WriteLine("Error: No arguments expected!\nTry 'file_freespace help'");
    return;
  }

  size_t const freeSpace = pIFS->GetFreeSpace();
  cli.WriteLine(std::to_string(freeSpace) + " byte");
}
void CLICMDDump(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS)
/**
 * \ingroup GPCC_FILESYSTEMS_CLI
 * \brief CLI command handler: Dumps the content of a file.
 *
 * After each kilobyte of dumped data the user is asked if he/she likes to continue.
 * This prevents issued with very large files.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("file_dump", " full_name\n"\
 *                                                          "Dumps the content of a file.",
 *                  std::bind(&gpcc::file_systems::CLICMDDump,
 *                  std::placeholders::_1, std::placeholders::_2, pIFS));
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
 * Arguments entered behind the command.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pIFS
 * Pointer to the @ref IFileStorage interface used to access the files.
 */
{
  auto fileReader = pIFS->Open(restOfLine);
  ON_SCOPE_EXIT() { try { fileReader->Close(); } catch (std::exception const &) {}; };

  uint8_t buffer[16];
  uint8_t bufLevel = 0;
  uint32_t offset = 0;

  cli.WriteLine("Offset      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");
  while (true)
  {
    while ((fileReader->GetState() != gpcc::Stream::IStreamReader::States::empty) && (bufLevel != sizeof(buffer)))
      buffer[bufLevel++] = fileReader->Read_uint8();

    if (bufLevel == 0)
      break;

    cli.WriteLine(gpcc::string::HexDump(offset, buffer, bufLevel, 1U, 16U));
    offset += bufLevel;
    bufLevel = 0;

    if (offset % 1024U == 0)
    {
      auto const userEntry = cli.ReadLine("Continue? (no = stop, anything else = continue):");
      if (gpcc::string::Trim(userEntry) == "no")
      {
        cli.WriteLine("aborted");
        return;
      }
    }
  }

  cli.WriteLine("Dumped " + std::to_string(offset + bufLevel) + " byte");
}
void CLICMDCopy(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS)
/**
 * \ingroup GPCC_FILESYSTEMS_CLI
 * \brief CLI command handler: Creates a copy of an existing file.
 *
 * Example for registration at an CLI:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("file_copy", " current_name new_name\n"\
 *                                                          "Creates a copy of an existing file.",
 *                  std::bind(&gpcc::file_systems::CLICMDCopy,
 *                  std::placeholders::_1, std::placeholders::_2, pIFS));
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
 * - the file system implementation may switch to a "defect state".
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - content of terminal's screen could be undefined
 *
 * ---
 *
 * \param restOfLine
 * Arguments entered behind the command.
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 * \param pIFS
 * Pointer to the @ref IFileStorage interface used to access the files.
 */
{
  auto const args = gpcc::string::Split(restOfLine, ' ', true);
  if (args.size() != 2)
  {
    cli.WriteLine("Error: Two arguments expected!\nTry 'file_copy help'");
    return;
  }

  if (args[0] == args[1])
  {
    cli.WriteLine("Error: Cannot copy file to itself");
    return;
  }

  // open source file
  auto fileReader = pIFS->Open(args[0]);
  ON_SCOPE_EXIT(closeReader) { try { fileReader->Close(); } catch (std::exception const &) {}; };

  // Create destination file (no overwrite if already existing) and
  // ensure that the destination file is closed and deleted if something goes wrong.
  auto fileWriter = pIFS->Create(args[1], false);
  ON_SCOPE_EXIT(deleteDestFile) { try { pIFS->Delete(args[1]); } catch (std::exception const &) {}; };
  ON_SCOPE_EXIT(closeWriter) { try { fileWriter->Close(); } catch (std::exception const &) {};};

  // copy
  while (fileReader->GetState() != gpcc::Stream::IStreamReader::States::empty)
    fileWriter->Write_uint8(fileReader->Read_uint8());

  // Close destination file. It will not be deleted if everything succeeds.
  ON_SCOPE_EXIT_DISMISS(closeWriter);
  fileWriter->Close();
  ON_SCOPE_EXIT_DISMISS(deleteDestFile);

  cli.WriteLine("Copy done");
}

} // namespace file_systems
} // namespace gpcc
