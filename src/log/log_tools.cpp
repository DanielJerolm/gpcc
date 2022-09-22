/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/log/log_tools.hpp>
#include <gpcc/file_systems/IFileStorage.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include <stdexcept>
#include <cstdint>

// Version of the format of binary files
#define VERSION (0x00000001UL)

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG
 * \brief Stores an std::vector of @ref ILogFacilityCtrl::tLogSrcConfig into a binary file.
 *
 * The stored data can be loaded using @ref ReadLogSrcConfigFromFile().
 *
 * If the content of the file shall be human readable (and editable), then consider using
 * @ref WriteLogSrcConfigToTextFile() instead of this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee, if [EEPROMSectionSystem](@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem) is
 * the underlying file system implementation.\n
 * Basic guarantee for other file system implementations:\n
 * - An incomplete file may be created
 * - An already existing file may be overwritten and the new file may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An incomplete file may be created
 * - An already existing file may be overwritten and the new file may be incomplete
 *
 * - - -
 *
 * \param config
 * Log source configuration that shall be written into the file.
 *
 * \param fs
 * Reference to the file storage.
 *
 * \param fileName
 * Path and name of the file.\n
 * If a file with the given name is already existing, then it will be overwritten.
 */
void WriteLogSrcConfigToFile(std::vector<ILogFacilityCtrl::tLogSrcConfig> const & config,
                             file_systems::IFileStorage& fs,
                             std::string const & fileName)
{
  // prepare file for writing
  auto f = fs.Create(fileName, true);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) { /* intentionally empty */ } };

  *f << static_cast<uint32_t>(VERSION);
  *f << static_cast<uint64_t>(config.size());

  for (auto & e: config)
  {
    *f << e.first;
    *f << static_cast<uint8_t>(e.second);
  }

  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();
}

/**
 * \ingroup GPCC_LOG
 * \brief Loads an std::vector of @ref ILogFacilityCtrl::tLogSrcConfig elements from a binary file.
 *
 * This is the counterpart to @ref WriteLogSrcConfigToFile().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws InvalidVersionError   Version of binary is not supported ([details](@ref gpcc::log::InvalidVersionError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param fs
 * Reference to the file storage.
 *
 * \param fileName
 * Path and name of the file.
 *
 * \return
 * `std::vector` containing the loaded log source configuration.
 */
std::vector<ILogFacilityCtrl::tLogSrcConfig> ReadLogSrcConfigFromFile(file_systems::IFileStorage& fs,
                                                                      std::string const & fileName)
{
  // open file for reading
  auto f = fs.Open(fileName);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) { /* intentionally empty */ } };

  // read and check version
  if (f->Read_uint32() != VERSION)
    throw InvalidVersionError();

  // read number of entries and prepare field for loading
  size_t remainingItems = f->Read_uint64();
  std::vector<ILogFacilityCtrl::tLogSrcConfig> config;
  config.reserve(remainingItems);

  // load items
  while (remainingItems-- != 0)
  {
    std::string const name(f->Read_string());
    LogLevel const level = static_cast<LogLevel>(f->Read_uint8());
    if (level > LogLevel::Nothing)
      throw std::runtime_error("Inconsistent file content");
    config.push_back(ILogFacilityCtrl::tLogSrcConfig(name, level));
  }

  // cross-check: file must be empty after loading all items
  if (f->GetState() != gpcc::stream::IStreamReader::States::empty)
    throw std::runtime_error("Inconsistent file content");

  // close file
  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();

  return config;
}

/**
 * \ingroup GPCC_LOG
 * \brief Stores an std::vector of @ref ILogFacilityCtrl::tLogSrcConfig into a human readable (and editable) text file.
 *
 * The stored data can be loaded using @ref ReadLogSrcConfigFromTextFile().
 *
 * The format of the created file is human readable and allows humans to edit the file using a text editor. The
 * created file looks like this:
 *
 * ~~~
 * # Log Levels
 * # Format: One entry per line, name and log level separated by ':'. Example:
 * # <Name of log source> : <log level>
 * # Valid log levels: debug, info, warning, error, fatal, nothing
 * # Leading and trailing whitespaces, and whitespaces around the ':' are removed.
 * <First log source's name> : <log level>
 * ...
 * <Last log source's name> : <log level>
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee, if [EEPROMSectionSystem](@ref gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem) is
 * the underlying file system implementation.\n
 * Basic guarantee for other file system implementations:\n
 * - An incomplete file may be created
 * - An already existing file may be overwritten and the new file may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An incomplete file may be created
 * - An already existing file may be overwritten and the new file may be incomplete
 *
 * - - -
 *
 * \param config
 * Log source configuration that shall be written into the file.
 *
 * \param fs
 * Reference to the file storage.
 *
 * \param fileName
 * Path and name of the file.\n
 * If a file with the given name is already existing, then it will be overwritten.
 *
 * \param headline
 * Headline for the file:\n
 * If this is not an empty string, then the first line of the file will be "# <headline>".\n
 * If this is an empty string, then the first line of the file will be "# Log Levels".\n
 * This must not contain any '\\n' characters.
 */
void WriteLogSrcConfigToTextFile(std::vector<ILogFacilityCtrl::tLogSrcConfig> const & config,
                                 file_systems::IFileStorage& fs,
                                 std::string const & fileName,
                                 std::string const & headline)
{
  if (headline.find('\n') != std::string::npos)
    throw std::invalid_argument("WriteLogSrcConfigToTextFile: 'headline' contains a \\n");

  auto f = fs.Create(fileName, true);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) { /* intentionally empty */ } };

  if (!headline.empty())
    f->Write_line("# " + headline);
  else
    f->Write_line("# Log Levels");

  f->Write_line("# Format: One entry per line, name and log level separated by ':'. Example:");
  f->Write_line("# <Name of log source> : <log level>");
  f->Write_line("# Valid log levels: debug, info, warning, error, fatal, nothing");
  f->Write_line("# Leading and trailing whitespaces, and whitespaces around the ':' are removed.");

  for (auto & e: config)
  {
    f->Write_line(e.first + " : " + LogLevel2String(e.second));
  }

  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();
}

/**
 * \ingroup GPCC_LOG
 * \brief Loads an std::vector of @ref ILogFacilityCtrl::tLogSrcConfig elements from a text file.
 *
 * This is the counterpart to @ref WriteLogSrcConfigToTextFile().
 *
 * The content of the file must comply to the following rules:
 * - Lines starting with '#' are ignored
 * - Empty lines are ignored
 * - Entries must have the following format: _Name of log source : log level_
 * - Valid log-levels are:\n
 *   debug, info, warning, error, fatal, nothing
 * - Lines are separated by '\\n'. '\\r' in front of or behind \\n are ignored.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param fs
 * Reference to the file storage.
 *
 * \param fileName
 * Path and name of the file.
 *
 * \return
 * `std::vector` containing the loaded log source configuration.
 */
std::vector<ILogFacilityCtrl::tLogSrcConfig> ReadLogSrcConfigFromTextFile(file_systems::IFileStorage& fs,
                                                                          std::string const & fileName)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> config;

  // open file for reading
  auto f = fs.Open(fileName);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) { /* intentionally empty */ } };

  // loop and read lines from file until EOF
  while (f->GetState() != gpcc::stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    // remove any leading or trailing '\r'
    if (line.empty())
      continue;

    if (line.front() == '\r')
      line.erase(0U, 1U);

    if (line.empty())
      continue;

    if (line.back() == '\r')
      line.pop_back();

    // remove leading and trailing spaces
    line = gpcc::string::Trim(line);

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    auto portions = gpcc::string::Split(line, ':', false);

    if (portions.size() != 2U)
      throw std::runtime_error("ReadLogSrcConfigFromFile: Malformed line. Expectation: <Name> : <log level>");

    portions[0] = gpcc::string::Trim(portions[0]);
    portions[1] = gpcc::string::Trim(portions[1]);

    if (portions[0].empty())
      throw std::runtime_error("ReadLogSrcConfigFromFile: Log source name length is zero");

    auto const loglevel = String2LogLevel(portions[1]);

    config.emplace_back(portions[0], loglevel);
  }

  // close file
  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();

  return config;
}

} // namespace log
} // namespace gpcc
