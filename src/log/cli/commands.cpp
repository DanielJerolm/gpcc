/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/log/cli/commands.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/file_systems/IFileStorage.hpp>
#include <gpcc/log/logfacilities/ILogFacilityCtrl.hpp>
#include <gpcc/log/log_tools.hpp>
#include <gpcc/string/tools.hpp>
#include <algorithm>
#include <stdexcept>

namespace
{
  // enum with actions that can be taken in conjunction with log level manipulation
  enum class Action
  {
    lower,          // log level will be lowered only
    raise,          // log level will be raised only
    set,            // log level will be set
    notSpecified    // no action specified (yet)
  };
}

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG_CLI
 * \brief Applies log levels from an std::vector of @ref ILogFacilityCtrl::tLogSrcConfig to the log sources registered
 *        at a log facility and prints information to a CLI.
 *
 * The following information is printed to the referenced CLI:
 * - The names of log sources contained in `newConfig` but which are not known at the log facility.
 * - The names of log sources registered at the log facility but which are not contained in `newConfig`.
 *
 * Finally "done" will be printed to the CLI.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe, __but__\n
 * appliance of the log levels is not an atomic operation. Log levels are applied one by one and log sources may be
 * registered or unregistered in between.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Configuration of log sources may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Configuration of log sources may be incomplete
 *
 * - - -
 *
 * \param newConfig
 * Log source configuration that shall be applied.
 *
 * \param pLogFacilityCtrl
 * Pointer to the @ref ILogFacilityCtrl interface of the log facility that shall be configured.
 *
 * \param cli
 * The names of unknown log sources and the names of log sources that did not receive a configuration will be printed
 * to the referenced CLI. A final "Done" will also be printed to the CLI.
 */
static void ApplyLogSrcConfig(std::vector<ILogFacilityCtrl::tLogSrcConfig> const & newConfig,
                              ILogFacilityCtrl* const pLogFacilityCtrl,
                              gpcc::cli::CLI & cli)
{
  auto currentConfig = pLogFacilityCtrl->EnumerateLogSources();

  bool first = true;
  for (auto it_nc = newConfig.begin(); it_nc != newConfig.end(); ++it_nc)
  {
    std::string const & ncLogSrcName = (*it_nc).first;

    auto it_cc = std::find_if(currentConfig.begin(),
                              currentConfig.end(),
                              [&ncLogSrcName](ILogFacilityCtrl::tLogSrcConfig const & e) { return (e.first == ncLogSrcName); });

    // no entry in currentConfig?
    if (it_cc == currentConfig.end())
    {
      if (first)
      {
        cli.WriteLine("The following log sources are unknown:");
        first = false;
      }
      cli.WriteLine("  " + ncLogSrcName);
      continue;
    }

    // update of log level required?
    if ((*it_cc).second != (*it_nc).second)
    {
      if (!pLogFacilityCtrl->SetLogLevel(ncLogSrcName, (*it_nc).second))
      {
        if (first)
        {
          cli.WriteLine("The following log sources are unknown:");
          first = false;
        }
        cli.WriteLine("  " + ncLogSrcName);
      }
    }

    currentConfig.erase(it_cc);
  }

  if (!currentConfig.empty())
  {
    cli.WriteLine("There were no settings provided for the following log sources:");
    for (auto & e : currentConfig)
      cli.WriteLine("  " + e.first);
  }

  cli.WriteLine("Done");
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief Prints a list of @ref ILogFacilityCtrl::tLogSrcConfig entries to a command line interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content printed to CLI may be incomplete or disturbed
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content printed to CLI may be incomplete or disturbed
 *
 * - - -
 *
 * \param cli
 * @ref gpcc::cli::CLI instance to which the list shall be printed to.
 *
 * \param logSources
 * List of @ref ILogFacilityCtrl::tLogSrcConfig entries to be printed.
 */
static void PrintLogSrcInfo(gpcc::cli::CLI & cli, std::vector<ILogFacilityCtrl::tLogSrcConfig> const & logSources)
{
  // special case: no entries
  if (logSources.empty())
  {
    cli.WriteLine("No log sources");
    return;
  }

  // determine maximum log source name length
  size_t length = 0U;
  for (auto & e: logSources)
  {
    if (e.first.size() > length)
      length = e.first.size();
  }

  // prepare headline and extend length
  std::string s = "Idx  | Log source name ";

  length += 7U + 1U; // for "Idx  | " (7) and trailing white-space (1)

  if (length < s.length())
    length = s.length();
  else
    s.resize(length, ' ');

  s += "| Current log level"; // (19 chars)

  // print headline
  cli.WriteLine(s);

  // print horizontal divider
  s = "";
  s.resize(length + 19U, '-');
  s[5] = '+';
  s[length] = '+';
  cli.WriteLine(s);

  // print log source names and log levels
  uint32_t i = 0U;
  for (auto & e: logSources)
  {
    s = std::to_string(i);
    if (s.length() < 5U)
      s.resize(5U, ' ');
    s += "| ";
    s += e.first;
    s.resize(length, ' ');
    s += "| ";
    s += LogLevel2String(e.second);
    cli.WriteLine(s);
    i++;
  }
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief Lowers, raises, or sets the log level of a log source.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Log level may have been modified or not
 * - content of terminal's screen may be incomplete or undefined
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Log level may have been modified or not
 * - content of terminal's screen may be incomplete or undefined
 *
 * - - -
 *
 * \param cli
 * An acknowledge message will be printed to the referenced [CLI](@ref gpcc::cli::CLI) instance
 * after the log level has been modified.
 *
 * \param pLogFacilityCtrl
 * Interface used to modify the log level of the log source.
 *
 * \param logSrcName
 * Name of the log source, whose log level shall be modified.
 *
 * \param action
 * Action to be performed on the log level:
 * - `Action::lower` - Reduces the log level if higher than `level`
 * - `Action::raise` - Raises the log level if lower then `level`
 * - `Action::set` - Sets the log level to `level`
 *
 * \param level
 * Log level that shall be configured according to parameter `action` at the
 * log source referenced by parameter `logSrcName`.
 */
static void ModifyLogLevel(gpcc::cli::CLI & cli,
                           ILogFacilityCtrl* const pLogFacilityCtrl,
                           std::string const & logSrcName,
                           Action const action,
                           LogLevel const level)
{
  bool result;
  switch (action)
  {
    case Action::lower:
      result = pLogFacilityCtrl->LowerLogLevel(logSrcName, level);
      if (result)
        cli.WriteLine(logSrcName + ": <= " + LogLevel2String(level));
      break;

    case Action::raise:
      result = pLogFacilityCtrl->RaiseLogLevel(logSrcName, level);
      if (result)
        cli.WriteLine(logSrcName + ": >= " + LogLevel2String(level));
      break;

    case Action::set:
      result = pLogFacilityCtrl->SetLogLevel(logSrcName, level);
      if (result)
        cli.WriteLine(logSrcName + ": = " + LogLevel2String(level));
      break;

    default:
      throw std::invalid_argument("ModifyLogLevel: Bad action");
  }

  if (!result)
    cli.WriteLine(logSrcName + ": Ignored, no such log source");
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief CLI command handler: Prints log sources and current log levels to CLI and allows the user
 *        to modify log levels.
 *
 * This function does not expect any parameters entered on the command line.
 *
 * After invocation of the command through the CLI, the names and the currently configured log levels
 * of all log sources are printed to the CLI. The lines in the output are indexed.
 *
 * The function then requests the user to enter a command string for log level manipulation. The command
 * string syntax is printed to the CLI to simplify usage. The command string syntax allows the user to
 * raise, lower, or set the log level of one, multiple, or all log sources. The user is also able to leave
 * without modifying any log level.
 *
 * Usage example:
 * ~~~{.cpp}
 * // pCLI points to an gpcc::cli::CLI instance
 * // pLogFacility points to an gpcc::log::ThreadedLogFacility instance
 *
 * pCLI->AddCommand(gpcc::cli::Command::Create("logsys", "\nInteractive log system configuration.",
 *                  std::bind(&gpcc::log::CLI_Cmd_LogCtrl,
 *                            std::placeholders::_1,
 *                            std::placeholders::_2,
 *                            static_cast<gpcc::log::ILogFacilityCtrl*>(pLogFacility))));
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The terminal's screen may be left with incomplete or undefined content
 * - Not all log levels may have been modified
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The terminal's screen may be left with incomplete or undefined content
 * - Not all log levels may have been modified
 *
 * - - -
 *
 * \param restOfLine
 * Any stuff entered behind the command. This is required by @ref gpcc::cli::CLI. \n
 * This function does not expect any arguments entered on the CLI.
 *
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 *
 * \param pLogFacilityCtrl
 * Pointer to the @ref ILogFacilityCtrl interface of the log facility, which shall be controlled via this function.
 */
void CLI_Cmd_LogCtrl(std::string const & restOfLine, gpcc::cli::CLI & cli, ILogFacilityCtrl* const pLogFacilityCtrl)
{
  // check: no parameters expected
  if (restOfLine.length() != 0U)
  {
    cli.WriteLine("Error: No parameters expected");
    return;
  }

  // get list of currently registered log sources
  auto logSources = pLogFacilityCtrl->EnumerateLogSources();

  // print currently registered log sources
  cli.WriteLine("Currently registered log sources:");
  PrintLogSrcInfo(cli, logSources);

  // no log sources? -> nothing to configure -> exit
  if (logSources.empty())
    return;

  // print choices
  cli.WriteLine("Available choices: (Enter nothing in order to leave)\n"\
                "(lower | raise | [set]) D|I|W|E|F|N (index1 [index2 ... n]) | all");

  // read user input from terminal
  std::string const entry = cli.ReadLine("Change log settings>");
  if (entry.length() == 0U)
    return;

  // split and process user entry
  auto splittedEntry = gpcc::string::Split(entry, ' ', true);

  Action action    = Action::notSpecified;
  LogLevel level   = LogLevel::DebugOrAbove;
  bool logLevelSet = false;

  for (auto & e: splittedEntry)
  {
    if (action == Action::notSpecified)
    {
      // if no action is specified, then the default is "set"
      action = Action::set;

      if (e == "lower")
      {
        action = Action::lower;
        continue;
      }
      if (e == "raise")
      {
        action = Action::raise;
        continue;
      }
      if (e == "set")
      {
        action = Action::set;
        continue;
      }
    }

    if (!logLevelSet)
    {
      logLevelSet = true;

      if (e == "D")
      {
        level = LogLevel::DebugOrAbove;
        continue;
      }
      if (e == "I")
      {
        level = LogLevel::InfoOrAbove;
        continue;
      }
      if (e == "W")
      {
        level = LogLevel::WarningOrAbove;
        continue;
      }
      if (e == "E")
      {
        level = LogLevel::ErrorOrAbove;
        continue;
      }
      if (e == "F")
      {
        level = LogLevel::FatalOrAbove;
        continue;
      }
      if (e == "N")
      {
        level = LogLevel::Nothing;
        continue;
      }

      // still here?
      cli.WriteLine("Error: No log level specified or invalid log level!");
      return;
    }

    if (e == "all")
    {
      for (auto & ls: logSources)
        ModifyLogLevel(cli, pLogFacilityCtrl, ls.first, action, level);
    }
    else
    {
      uint32_t index = gpcc::string::DecimalToU32(e);
      if (index >= logSources.size())
      {
        cli.WriteLine("Error: Invalid log source index");
        return;
      }

      ModifyLogLevel(cli, pLogFacilityCtrl, logSources[index].first, action, level);
    }
  }
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief CLI command handler: Writes the names and the log levels of all log sources registered at a log facility into
 *        a binary file.
 *
 * The file name is expected to be passed as an parameter to the command.\n
 * If a file with the given name is already existing, then it will be overwritten.
 *
 * Usage example:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("storeLogConf", " FILENAME\n"\
 *                                             "Stores the log system configuration into a file referenced by FILENAME.\n"\
 *                                             "FILENAME will be overwritten if it is already existing.",
 *                  std::bind(&gpcc::log::CLI_Cmd_WriteConfigToFile,
 *                            std::placeholders::_1,
 *                            std::placeholders::_2,
 *                            pLogFacility,
 *                            pFileStorage)));
 * ~~~
 *
 * @ref CLI_Cmd_ReadConfigFromFile() is the counterpart to this.
 *
 * @ref WriteLogSrcConfigToFile() is used to store the information.
 *
 * If the content of the file shall be human readable (and thus editable), then consider using
 * @ref CLI_Cmd_WriteConfigToTextFile() instead of this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee, if [EEPROMSectionSystem](@ref gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem) is
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
 * \param restOfLine
 * Any stuff entered behind the command.\n
 * This function expects the file's name as an argument entered on the CLI.
 *
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 *
 * \param pLogFacilityCtrl
 * Pointer to the @ref ILogFacilityCtrl, from which the log sources and log levels shall be retrieved.
 *
 * \param pFileStorage
 * Pointer to the file storage location where the file shall be stored.
 */
void CLI_Cmd_WriteConfigToFile(std::string const & restOfLine,
                               gpcc::cli::CLI & cli,
                               ILogFacilityCtrl* const pLogFacilityCtrl,
                               gpcc::file_systems::IFileStorage* const pFileStorage)
{
  if (restOfLine.find(' ') != std::string::npos)
  {
    cli.WriteLine("Error: Invalid filename");
    return;
  }

  WriteLogSrcConfigToFile(pLogFacilityCtrl->EnumerateLogSources(), *pFileStorage, restOfLine);
  cli.WriteLine("done");
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief CLI command handler: Loads log source names and log levels from a binary file and configures a log facility
 *        with the loaded settings.
 *
 * The file name is expected to be passed as an parameter to the command.\n
 * All missing entries (in the file and in the log facility) are printed to the console.
 *
 * Usage example:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("loadLogConf", " FILENAME\n"\
 *                                             "Loads the log system configuration from a file referenced by FILENAME.",
 *                  std::bind(&gpcc::log::CLI_Cmd_ReadConfigFromFile,
 *                            std::placeholders::_1,
 *                            std::placeholders::_2,
 *                            pLogFacility,
 *                            pFileStorage)));
 * ~~~
 *
 * This is the counterpart to @ref CLI_Cmd_WriteConfigToFile().
 *
 * @ref ReadLogSrcConfigFromFile() is used to load the configuration.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Configuration of log sources may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Configuration of log sources may be incomplete
 *
 * - - -
 *
 * \param restOfLine
 * Any stuff entered behind the command.\n
 * This function expects the file's name as an argument entered on the CLI.
 *
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 *
 * \param pLogFacilityCtrl
 * Pointer to the @ref ILogFacilityCtrl, which shall be configured with the settings loaded from the file.
 *
 * \param pFileStorage
 * Pointer to the file storage location from where the file shall be loaded.
 */
void CLI_Cmd_ReadConfigFromFile(std::string const & restOfLine, gpcc::cli::CLI & cli, ILogFacilityCtrl* const pLogFacilityCtrl, gpcc::file_systems::IFileStorage* const pFileStorage)
{
  auto const newConfig = ReadLogSrcConfigFromFile(*pFileStorage, restOfLine);
  ApplyLogSrcConfig(newConfig, pLogFacilityCtrl, cli);
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief CLI command handler: Writes the names and the log levels of all log sources registered at a log facility into
 *        a human-readable text file.
 *
 * The file name is expected to be passed as an parameter to the command.\n
 * If a file with the given name is already existing, then it will be overwritten.
 *
 * Usage example:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("storeLogConf", " FILENAME\n"\
 *                                             "Stores the log system configuration into a file referenced by FILENAME.\n"\
 *                                             "FILENAME will be overwritten if it is already existing.",
 *                  std::bind(&gpcc::log::CLI_Cmd_WriteConfigToTextFile,
 *                            std::placeholders::_1,
 *                            std::placeholders::_2,
 *                            pLogFacility,
 *                            pFileStorage,
 *                            "Log levels for XYZ-software")));
 * ~~~
 *
 * The format of the created file is human readable and allows humans to edit the file using a text editor. For details
 * please refer to @ref WriteLogSrcConfigToTextFile().
 *
 * If the file shall be stored using a binary format, then @ref CLI_Cmd_WriteConfigToFile() could be used instead of
 * this.
 *
 * @ref CLI_Cmd_ReadConfigFromTextFile() is the counterpart to this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee, if [EEPROMSectionSystem](@ref gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem) is
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
 * \param restOfLine
 * Any stuff entered behind the command.\n
 * This function expects the file's name as an argument entered on the CLI.
 *
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 *
 * \param pLogFacilityCtrl
 * Pointer to the @ref ILogFacilityCtrl, from which the log sources and log levels shall be retrieved.
 *
 * \param pFileStorage
 * Pointer to the file storage location where the file shall be stored.
 *
 * \param pHeadline
 * Pointer to a null-terminated string that shall be used as a headline for the file's content. The headline will be
 * stored as a comment at the beginning of the file. A leading "#" will be automatically prepended.\n
 * See @ref WriteLogSrcConfigToTextFile() for details.\n
 * The life-time of the referenced string must exceed the time span during which this CLI command handler may be
 * invoked by a CLI instance. During this time span, the referenced string must also not be modified.\n
 * __It is good pratice to use a string located in ROM/code memory.__\n
 * This may be `nullptr` or refer to an empty string ("") if no custom headline is required.
 */
void CLI_Cmd_WriteConfigToTextFile(std::string const & restOfLine,
                                   gpcc::cli::CLI & cli,
                                   ILogFacilityCtrl* const pLogFacilityCtrl,
                                   gpcc::file_systems::IFileStorage* const pFileStorage,
                                   char const * pHeadline)
{
  if (restOfLine.find(' ') != std::string::npos)
  {
    cli.WriteLine("Error: Invalid filename");
    return;
  }

  if (pHeadline == nullptr)
    pHeadline = "";

  WriteLogSrcConfigToTextFile(pLogFacilityCtrl->EnumerateLogSources(), *pFileStorage, restOfLine, pHeadline);
  cli.WriteLine("done");
}

/**
 * \ingroup GPCC_LOG_CLI
 * \brief CLI command handler: Loads log source names and log levels from a human-readable text file and configures a
 *        log facility with the loaded settings.
 *
 * The file name is expected to be passed as an parameter to the command.\n
 * All missing entries (in the file and in the log facility) are printed to the console.
 *
 * Usage example:
 * ~~~{.cpp}
 * pCLI->AddCommand(gpcc::cli::Command::Create("loadLogConf", " FILENAME\n"\
 *                                             "Loads the log system configuration from a file referenced by FILENAME.",
 *                  std::bind(&gpcc::log::CLI_Cmd_ReadConfigFromFile,
 *                            std::placeholders::_1,
 *                            std::placeholders::_2,
 *                            pLogFacility,
 *                            pFileStorage)));
 * ~~~
 *
 * This is the counterpart to @ref CLI_Cmd_WriteConfigToTextFile().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Configuration of log sources may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Configuration of log sources may be incomplete
 *
 * - - -
 *
 * \param restOfLine
 * Any stuff entered behind the command.\n
 * This function expects the file's name as an argument entered on the CLI.
 *
 * \param cli
 * @ref gpcc::cli::CLI instance, in whose context this function is invoked.
 *
 * \param pLogFacilityCtrl
 * Pointer to the @ref ILogFacilityCtrl, which shall be configured with the settings loaded from the file.
 *
 * \param pFileStorage
 * Pointer to the file storage location from where the file shall be loaded.
 */
void CLI_Cmd_ReadConfigFromTextFile(std::string const & restOfLine, gpcc::cli::CLI & cli, ILogFacilityCtrl* const pLogFacilityCtrl, gpcc::file_systems::IFileStorage* const pFileStorage)
{
  auto const newConfig = ReadLogSrcConfigFromTextFile(*pFileStorage, restOfLine);
  ApplyLogSrcConfig(newConfig, pLogFacilityCtrl, cli);
}

} // namespace log
} // namespace gpcc
