/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#include "CLIAdapterBase.hpp"
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/Command.hpp>
#include <gpcc/cli/exceptions.hpp>
#include "string_conversion.hpp"
#include "gpcc/src/cood/cli/internal/CAReadArgsParser.hpp"
#include "gpcc/src/cood/cli/internal/CAWriteArgsParser.hpp"
#include "gpcc/src/cood/cli/internal/EnumerateArgsParser.hpp"
#include "gpcc/src/cood/cli/internal/InfoArgsParser.hpp"
#include "gpcc/src/cood/cli/internal/ReadArgsParser.hpp"
#include "gpcc/src/cood/cli/internal/WriteArgsParser.hpp"
#include "gpcc/src/cood/IObjectAccess.hpp"
#include "gpcc/src/cood/ObjectPtr.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
 *
 * After construction, the sub-class has to invoke @ref RegisterCLICommand(). \n
 * The recommended place for the call is the end of the subclass' constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _od
 * @ref IObjectAccess interface of the object dictionary that shall be accessed by the CLI command.
 *
 * \param _cli
 * Reference to the CLI instance where the CLI command shall be registered.
 *
 * \param _cmdName
 * Desired name for the CLI command.\n
 * Sub-commands will be realized via arguments passed to the command.\n
 * The string must meet the requirements of [cli::Command::Create()](@ref gpcc::cli::Command::Create) or
 * @ref RegisterCLICommand() will fail later.
 *
 * \param _attributeStringMaxLength
 * Maximum length of any string that could be returned by @ref AttributesToStringHook(). Zero is not allowed.
 */
CLIAdapterBase::CLIAdapterBase(IObjectAccess & _od,
                               gpcc::cli::CLI & _cli,
                               std::string const & _cmdName,
                               uint8_t const _attributeStringMaxLength)
: od(_od)
, cli(_cli)
, cmdName(_cmdName)
, attributeStringMaxLength(_attributeStringMaxLength)
{
  if (attributeStringMaxLength == 0U)
    throw std::invalid_argument("CLIAdapterBase::CLIAdapterBase: '_attributeStringMaxLength' invalid");
}

/**
 * \brief Registers the CLI command at the CLI.
 *
 * This shall be invoked by the sub-class. The recommended place for the call is the end of the sub-class' constructor.
 *
 * \pre   The CLI command is not registered at the CLI.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void CLIAdapterBase::RegisterCLICommand(void)
{
  using gpcc::cli::Command;

  ON_SCOPE_EXIT(undoCMDRegistration) { UnregisterCLICommand(); };

  cli.AddCommand(Command::Create(cmdName.c_str(), " subcmd [args...]\n"
                                 "Accesses the local object dictionary. The type of access is specified by <subcmd>:\n"
                                 "- enum [0xFROM-0xTO]\n"
                                 "  Enumerates objects contained in the object dictionary.\n"
                                 "  Options:\n"
                                 "    FROM   Index where enumeration shall start. Default: 0x0000\n"
                                 "    TO     Index where enumeration shall end. Default: 0xFFFF\n"
                                 "    FROM <= TO must be valid.\n"
                                 "\n"
                                 "- info 0xINDEX [asm]\n"
                                 "  Prints the meta data of an object and its subindices.\n"
                                 "  Options:\n"
                                 "    asm   Includes application-specific meta data in the output.\n"
                                 "\n"
                                 "- read 0xINDEX:Subindex\n"
                                 "  Reads the data of a subindex and prints it to CLI.\n"
                                 "  <subindex> shall be provided in decimal format.\n"
                                 "\n"
                                 "- write 0xINDEX:Subindex DATA\n"
                                 "  Writes <DATA> to a subindex. <Subindex> shall be provided in decimal format.\n"
                                 "\n"
                                 "  The format of <DATA> must meet the data type of the subindex:\n"
                                 "  For BOOLEAN: TRUE, FALSE, true, false\n"
                                 "  For REAL32/64: [+|-]digits[.][digits][(e|E)[+|-]digits]\n"
                                 "  For VISIBLE_STRING: \"Text...\"\n"
                                 "  For OCTET_STRING: 5B A3 ... (8bit hex values, separated by spaces)\n"
                                 "  For UNICODE_STRING: 5B33 A6CF (16bit hex values, separated by spaces)\n"
                                 "  For BIT1..BIT8: 0, 1, 3, 0x3, 0b11 (unused upper bits must be zero)\n"
                                 "\n"
                                 "- caread 0xINDEX [v]\n"
                                 "  Reads the whole object via complete access and prints the value of each\n"
                                 "  subindex to CLI.\n"
                                 "  Options:\n"
                                 "  v   Verbose output. Prints the data type and name of each subindex in addition\n"
                                 "      to the data.\n"
                                 "- cawrite 0xINDEX\n"
                                 "  Writes the whole object via complete access.\n"
                                 "  The data that shall be written is entered using an interactive dialog.",
                                 std::bind(&CLIAdapterBase::CLI_CommandHandler, this,
                                           std::placeholders::_1, std::placeholders::_2)));

  ON_SCOPE_EXIT_DISMISS(undoCMDRegistration);
}

/**
 * \brief Unregisters the CLI command from the CLI.
 *
 * This is the counterpart to @ref RegisterCLICommand().
 *
 * This shall be invoked by the sub-class. The recommended place for the call is the begin of the sub-class' destructor.
 *
 * It is not harmful to invoke this, if the CLI command is not registered at the CLI.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 */
void CLIAdapterBase::UnregisterCLICommand(void) noexcept
{
  try
  {
    cli.RemoveCommand(cmdName.c_str());
  }
  catch (...)
  {
    PANIC();
  }
}

// Doc: See CLIAdapterBase.hpp
std::string CLIAdapterBase::AppSpecificMetaDataToStringHook(std::vector<uint8_t> const & data)
{
  std::ostringstream oss;
  oss << data.size() << " byte(s) of ASM";

  if (data.size() != 0U)
  {
    oss << ':';

    uint_fast8_t maxPrintedBytes = 16U;

    auto it = data.begin();
    while (it != data.end())
    {
      if (maxPrintedBytes-- == 0U)
      {
        oss << "...";
        break;
      }

      oss << ' ' << gpcc::string::ToHexNoPrefix(*it, 2U);
      ++it;
    }
  }

  return oss.str();
}

/**
 * \brief CLI command handler.
 *
 * This is executed when the user enters the CLI command registered by this class.
 *
 * This will pick up the first argument and delegate the call to a specialized CLI handler.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 *
 * \param cli
 * [CLI](@ref gpcc::cli::CLI) instance invoking this method.
 */
void CLIAdapterBase::CLI_CommandHandler(std::string const & restOfLine, gpcc::cli::CLI & cli)
{
  (void)cli;

  if (restOfLine.empty())
    throw gpcc::cli::UserEnteredInvalidArgsError();

  // split restOfLine into "command" (one word) and potential further arguments (args)
  auto pos = restOfLine.find(' ');
  auto const command = restOfLine.substr(0U, pos);
  if (pos == std::string::npos)
    pos = restOfLine.size() - 1U;
  auto const args = restOfLine.substr(pos + 1U);

  if (command == "enum")
    CLI_Enumerate(args);
  else if (command == "info")
    CLI_Info(args);
  else if (command == "read")
    CLI_Read(args);
  else if (command == "write")
    CLI_Write(args);
  else if (command == "caread")
    CLI_CARead(args);
  else if (command == "cawrite")
    CLI_CAWrite(args);
  else
    throw gpcc::cli::UserEnteredInvalidArgsError("Invalid/unknown sub command!");
}

/**
 * \brief CLI command handler for enumeration of objects.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 */
void CLIAdapterBase::CLI_Enumerate(std::string const & restOfLine)
{
  // ============================================
  // Analyse args
  // ============================================
  internal::EnumerateArgsParser args(restOfLine);

  uint16_t const startIdx = args.GetFirstIndex();
  uint16_t const endIdx   = args.GetLastIndex();

  // ============================================
  // Enumerate and print to CLI
  // ============================================

  // query first object
  auto objPtr = od.GetNextNearestObject(startIdx);
  if ((!objPtr) || (objPtr->GetIndex() > endIdx))
  {
    cli.WriteLine("No objects");
    return;
  }

  do
  {
    cli.TestTermination();

    uint16_t const index = objPtr->GetIndex();
    if (index > endIdx)
      break;

    // collect some more data...
    auto const objCode  = objPtr->GetObjectCode();
    auto const dataType = objPtr->GetObjectDataType();
    auto const objName  = objPtr->GetObjectName();

    // ...then print to CLI
    std::ostringstream oss;
    oss << gpcc::string::ToHex(index, 4U) << ' ';
    oss << std::left << std::setw(Object::largestObjectCodeNameLength) << std::setfill(' ') << Object::ObjectCodeToString(objCode) << ' '
        << std::left << std::setw(15) << std::setfill(' ') << DataTypeToString(dataType) << '"' << objName << '"';

    cli.WriteLine(oss.str());

    ++objPtr;
  }
  while (objPtr);
}

/**
 * \brief CLI command handler for querying information about one object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 * - Not all subindices may have been enumerated.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 * - Not all subindices may have been enumerated.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 */
void CLIAdapterBase::CLI_Info(std::string const & restOfLine)
{
  // ============================================
  // Analyse args
  // ============================================
  internal::InfoArgsParser args(restOfLine);

  uint16_t const idx = args.GetIndex();
  bool const inclASM = args.GetInclASM();

  // ============================================
  // Query object
  // ============================================
  auto objPtr = od.GetObject(idx);
  if (!objPtr)
  {
    cli.WriteLine("Error: No object with given index");
    return;
  }

  // ============================================
  // Print to CLI
  // ============================================
  std::ostringstream oss;

  // -- print info about object --
  oss << "Object " << gpcc::string::ToHex(idx, 4U) << ": " << Object::ObjectCodeToString(objPtr->GetObjectCode())
      << " (" << DataTypeToString(objPtr->GetObjectDataType()) << ") \"" << objPtr->GetObjectName() << '"';
  cli.WriteLine(oss.str());

  // small tool: Appends info about a subindex to 'oss'
  auto appendSubIndexInfoToOSS = [&](uint_fast8_t const si)
  {
    size_t  const s     = objPtr->GetSubIdxMaxSize(si);
    size_t  const bytes = s / 8U;
    uint8_t const bits  = s % 8U;
    oss << std::left << std::setw(15U) << std::setfill(' ') << DataTypeToString(objPtr->GetSubIdxDataType(si)) << ' '
        << std::left << std::setw(attributeStringMaxLength) << std::setfill(' ') << AttributesToStringHook(objPtr->GetSubIdxAttributes(si)) << ' '
        << std::right << std::setw(5U) << std::setfill(' ') << bytes << '.' << static_cast<uint32_t>(bits) << " Byte(s) \"" << objPtr->GetSubIdxName(si) << '"';
  };

  // small tool: Appends human readable textual representation of the app-specific meta data of a subindex to 'oss'
  auto appendAppSpecMetaDataToOSS = [&](uint_fast8_t const si)
  {
    size_t const s = objPtr->GetAppSpecificMetaDataSize(si);
    if (s == 0U)
    {
      oss << "No app-specific meta data.";
    }
    else
    {
      auto const data = objPtr->GetAppSpecificMetaData(si);
      oss << AppSpecificMetaDataToStringHook(data);
    }
  };

  // -- print info about subinces --
  // get maximum number of subindices
  auto maxNbOfSIs = objPtr->GetMaxNbOfSubindices();

  // Get number of digits required to print 'maxNbOfSIs'. This will be used for proper alignment of rows.
  auto digitsForSubindices = DigitsInSubindex(maxNbOfSIs - 1U);

  // compress the output for ARRAY objects if possible
  if ((!inclASM) && (objPtr->GetObjectCode() == Object::ObjectCode::Array))
  {
    oss.str("");
    oss << "  Subindex    ";
    for (uint_fast8_t i = 1U; i < digitsForSubindices; i++)
      oss << ' ';
    oss << "0: ";
    appendSubIndexInfoToOSS(0U);
    cli.WriteLine(oss.str());

    if (maxNbOfSIs > 1U)
    {
      oss.str("");
      oss << "  Subindex 1.." << static_cast<uint32_t>(maxNbOfSIs - 1U) << ": ";
      appendSubIndexInfoToOSS(1U);
      cli.WriteLine(oss.str());
    }
  }
  else
  {
    for (uint_fast16_t i = 0U; i < maxNbOfSIs; i++)
    {
      oss.str("");
      oss << "  Subindex " << std::setw(digitsForSubindices) << std::setfill(' ') << static_cast<uint32_t>(i) << ": ";
      if (objPtr->IsSubIndexEmpty(i))
      {
        oss << "empty";
      }
      else
      {
        appendSubIndexInfoToOSS(i);
      }

      if (inclASM)
      {
        oss << std::endl << "             ";
        for (uint_fast8_t j = 0U; j < digitsForSubindices; j++)
          oss << ' ';
        appendAppSpecMetaDataToOSS(i);
      }

      cli.WriteLine(oss.str());
    }
  }
}

/**
 * \brief CLI command handler for reading a single subindex.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 */
void CLIAdapterBase::CLI_Read(std::string const & restOfLine)
{
  // ============================================
  // Analyse args
  // ============================================
  internal::ReadArgsParser args(restOfLine);

  uint16_t const index  = args.GetIndex();
  uint8_t  const subIdx = args.GetSubIndex();

  // ============================================
  // Get object
  // ============================================
  auto objPtr = od.GetObject(index);
  if (!objPtr)
  {
    cli.WriteLine("Error: No object with given index");
    return;
  }

  // ============================================
  // Read the object
  // ============================================
  // prepare pointer to a buffer allocated for the read data
  size_t sizeInBit;
  size_t sizeInByte;
  uint8_t* pData = nullptr;
  ON_SCOPE_EXIT() { delete [] pData; };

  // this will contain the result of the read-access
  SDOAbortCode result;

  // read the object
  {
    // determine access permissions
    auto const permissions = BeginAccessHook() & Object::attr_ACCESS_RD;
    ON_SCOPE_EXIT() { EndAccessHook(); };

    // lock object's data
    auto locker = objPtr->LockData();

    // check if the subindex is existing
    if (subIdx >= objPtr->GetNbOfSubIndices())
    {
      cli.WriteLine("Error: Subindex does not exist");
      return;
    }

    // check if subindex is empty
    if (objPtr->IsSubIndexEmpty(subIdx))
    {
      cli.WriteLine("Error: Subindex is empty");
      return;
    }

    // determine size and allocate memory
    sizeInBit = objPtr->GetSubIdxActualSize(subIdx);
    sizeInByte = (sizeInBit + 7U) / 8U;
    pData = new uint8_t[sizeInByte];

    // do the actual read
    gpcc::Stream::MemStreamWriter msw(pData, sizeInByte, gpcc::Stream::IStreamWriter::nativeEndian);
    result = objPtr->Read(subIdx, permissions, msw);
    msw.Close();
  }

  // check for errors
  if (result != SDOAbortCode::OK)
  {
    cli.WriteLine(std::string("Read access failed: ") + SDOAbortCodeToDescrString(result));
    return;
  }

  // ============================================
  // Print to CLI
  // ============================================
  gpcc::Stream::MemStreamReader msr(pData, sizeInByte, gpcc::Stream::IStreamReader::nativeEndian);
  auto str = CANopenEncodedDataToString(msr, sizeInBit, objPtr->GetSubIdxDataType(subIdx));
  msr.Close();

  cli.WriteLine(str);
}

/**
 * \brief CLI command handler for writing a single subindex.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 */
void CLIAdapterBase::CLI_Write(std::string const & restOfLine)
{
  // ============================================
  // Analyze args (first part: index and subindex)
  // ============================================
  internal::WriteArgsParser args(restOfLine);

  uint16_t const index  = args.GetIndex();
  uint8_t  const subIdx = args.GetSubIndex();

  // ============================================
  // Get object
  // ============================================
  auto objPtr = od.GetObject(index);
  if (!objPtr)
  {
    cli.WriteLine("Error: No object with given index");
    return;
  }

  // ============================================
  // Query info about subindex
  // ============================================
  DataType const dataType = objPtr->GetSubIdxDataType(subIdx);
  size_t const subIdxMaxSize = objPtr->GetSubIdxMaxSize(subIdx);

  // ============================================
  // Analyze args (second part: data)
  // ============================================
  args.ExtractData(dataType, subIdxMaxSize, gpcc::Stream::IStreamWriter::nativeEndian);

  // ============================================
  // write to object
  // ============================================

  // this will contain the result of the write-access
  SDOAbortCode result;

  // write to the object
  {
    // determine access permissions
    auto const permissions = BeginAccessHook() & Object::attr_ACCESS_WR;
    ON_SCOPE_EXIT() { EndAccessHook(); };

    // lock object's data
    auto locker = objPtr->LockData();

    // check if the subindex is existing
    if (subIdx >= objPtr->GetNbOfSubIndices())
    {
      cli.WriteLine("Error: Subindex exceeds number of subindices");
      return;
    }

    // check if subindex is empty
    if (objPtr->IsSubIndexEmpty(subIdx))
    {
      cli.WriteLine("Error: Subindex is empty");
      return;
    }

    // do the actual write
    auto const & data = args.GetData();
    gpcc::Stream::MemStreamReader msr(data.data(), data.size(), gpcc::Stream::IStreamReader::nativeEndian);
    result = objPtr->Write(subIdx, permissions, msr);
    msr.Close();
  }

  // check for errors
  if (result != SDOAbortCode::OK)
  {
    cli.WriteLine(std::string("Write access failed: ") + SDOAbortCodeToDescrString(result));
  }
  else
  {
    cli.WriteLine("OK");
  }
}

/**
 * \brief CLI command handler for reading an object via complete access.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 */
void CLIAdapterBase::CLI_CARead(std::string const & restOfLine)
{
  // ============================================
  // Analyse args
  // ============================================
  internal::CAReadArgsParser args(restOfLine);

  // ============================================
  // Get object
  // ============================================
  auto objPtr = od.GetObject(args.GetIndex());
  if (!objPtr)
  {
    cli.WriteLine("Error: No object with given index");
    return;
  }

  // ============================================
  // Read the object
  // ============================================
  // prepare pointer to a buffer allocated for the read data
  size_t sizeInBit;
  size_t sizeInByte;
  uint8_t* pData = nullptr;
  ON_SCOPE_EXIT() { delete [] pData; };

  // read the object
  {
    // determine access permissions
    auto const permissions = BeginAccessHook() & Object::attr_ACCESS_RD;
    ON_SCOPE_EXIT() { EndAccessHook(); };

    // lock object's data
    auto locker = objPtr->LockData();

    // determine size and allocate memory
    sizeInBit = objPtr->GetObjectStreamSize(false);
    sizeInByte = (sizeInBit + 7U) / 8U;
    pData = new uint8_t[sizeInByte];

    // do the actual read
    gpcc::Stream::MemStreamWriter msw(pData, sizeInByte, gpcc::Stream::IStreamWriter::nativeEndian);
    auto const result = objPtr->CompleteRead(true, false, permissions, msw);
    msw.Close();

    if (result != SDOAbortCode::OK)
    {
      cli.WriteLine(std::string("Read access failed: ") + SDOAbortCodeToDescrString(result));
      return;
    }
  }

  // ============================================
  // Print to CLI
  // ============================================
  gpcc::Stream::MemStreamReader msr(pData, sizeInByte, gpcc::Stream::IStreamReader::nativeEndian);

  // extract value of SI0
  uint8_t const si0 = msr.Read_uint8();

  if (args.GetVerbose())
  {
    // determine padding for data type column and name column
    size_t paddingDataType = 0U;
    size_t paddingName = 0U;

    for (uint_fast16_t subIdx = 0U; subIdx <= si0; ++subIdx)
    {
      size_t s = strlen(DataTypeToString(objPtr->GetSubIdxDataType(subIdx)));
      if (paddingDataType < s)
        paddingDataType = s;

      s = objPtr->GetSubIdxName(subIdx).length();
      if (paddingName < s)
      {
        if (s > 120U)
        {
          cli.WriteLine("Encountered very large subindex name. Retry command without option 'v'.");
          return;
        }

        paddingName = s;
      }
    }

    // print each SI to CLI
    for (uint_fast16_t subIdx = 0U; subIdx <= si0; ++subIdx)
    {
      auto const dataType = objPtr->GetSubIdxDataType(subIdx);

      std::ostringstream oss;
      oss << "SI " << std::left << std::setw(3) << std::setfill(' ') << subIdx << " ("
          << std::left << std::setw(paddingDataType) << std::setfill('.') << DataTypeToString(dataType) << ".."
          << std::left << std::setw(paddingName) << std::setfill('.') << objPtr->GetSubIdxName(subIdx) << "..: ";

      if (subIdx == 0U)
        oss << static_cast<unsigned int>(si0);
      else
        oss << CANopenEncodedDataToString(msr, objPtr->GetSubIdxMaxSize(subIdx), dataType);

      cli.WriteLine(oss.str());
    }
  }
  else
  {
    for (uint_fast16_t subIdx = 0U; subIdx <= si0; ++subIdx)
    {
      std::ostringstream oss;
      oss << "SI " << subIdx << ": ";

      if (subIdx == 0U)
        oss << static_cast<unsigned int>(si0);
      else
        oss << CANopenEncodedDataToString(msr, objPtr->GetSubIdxMaxSize(subIdx), objPtr->GetSubIdxDataType(subIdx));

      cli.WriteLine(oss.str());
    }
  }

  msr.Close();
}

/**
 * \brief CLI command handler for writing to an object via complete access.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 */
void CLIAdapterBase::CLI_CAWrite(std::string const & restOfLine)
{
  // ============================================
  // Analyse args
  // ============================================
  internal::CAWriteArgsParser args(restOfLine);

  // ============================================
  // Get object
  // ============================================
  auto objPtr = od.GetObject(args.GetIndex());
  if (!objPtr)
  {
    cli.WriteLine("Error: No object with given index");
    return;
  }

  // ============================================
  // Object supported?
  // ============================================
  if (   (objPtr->GetObjectCode() != Object::ObjectCode::Array)
      && (objPtr->GetObjectCode() != Object::ObjectCode::Record))
  {
    cli.WriteLine("Object type not supported.");
    return;
  }

  // ============================================
  // Determine current value of SI0
  // ============================================
  uint8_t currSI0;

  {
    // determine access permissions
    auto const permissions = BeginAccessHook() & Object::attr_ACCESS_RD;
    ON_SCOPE_EXIT() { EndAccessHook(); };

    // lock object's data
    auto locker = objPtr->LockData();

    // do the actual read
    gpcc::Stream::MemStreamWriter msw(&currSI0, sizeof(currSI0), gpcc::Stream::IStreamWriter::nativeEndian);
    auto const result = objPtr->Read(0U, permissions, msw);
    msw.Close();

    if (result != SDOAbortCode::OK)
    {
      cli.WriteLine(std::string("Reading SI0 failed: ") + SDOAbortCodeToDescrString(result));
      return;
    }
  }

  // ============================================
  // Ask user to enter value for SI0
  // ============================================
  uint8_t newSI0 = currSI0;

  // Is SI0 writeable?
  if ((objPtr->GetSubIdxAttributes(0U) & Object::attr_ACCESS_WR) != 0U)
  {
    if (objPtr->GetObjectCode() != Object::ObjectCode::Array)
    {
      cli.WriteLine("SI0 is writeable. This is only supported for ARRAY objects.");
      return;
    }

    cli.WriteLine("Current value of SI0: " + std::to_string(currSI0));
    newSI0 = gpcc::string::DecimalToU8(cli.ReadLine("New value for SI0: "));

    if (newSI0 >= objPtr->GetMaxNbOfSubindices())
    {
      cli.WriteLine("Value for SI0 exceeds maximum number of subindices the object can have.");
      return;
    }
  }

  // ============================================
  // Prepare buffer for the data that shall be written
  // ============================================
  // determine size of whole object in bit
  size_t sizeInBit = 0U;
  for (uint_fast16_t subIdx = 0U; subIdx <= newSI0; ++subIdx)
  {
    auto const dataType = objPtr->GetSubIdxDataType(subIdx);

    // align to byte boundary, if data type is byte-based
    if (!IsDataTypeBitBased(dataType))
      sizeInBit += (8U - (sizeInBit % 8U)) % 8U;

    sizeInBit += objPtr->GetSubIdxMaxSize(subIdx);
  }

  // allocate memory
  size_t sizeInByte = (sizeInBit + 7U) / 8U;
  std::vector<uint8_t> data(sizeInByte);
  gpcc::Stream::MemStreamWriter msw(data.data(), sizeInByte, gpcc::Stream::MemStreamWriter::nativeEndian);

  // ============================================
  // Fill buffer with write data entered by the user
  // ============================================
  // write SI0
  msw.Write_uint8(newSI0);

  // write the data for the other subindices
  for (uint_fast16_t subIdx = 1U; subIdx <= newSI0; ++subIdx)
  {
    // determine data type and size
    auto const dataType = objPtr->GetSubIdxDataType(subIdx);
    size_t const siSize = objPtr->GetSubIdxMaxSize(subIdx);

    // skip empty subindices
    if (siSize == 0U)
    {
      cli.WriteLine("Skipping SI " + std::to_string(subIdx) + " (zero size))");
      continue;
    }

    // gap?
    if (dataType == DataType::null)
    {
      cli.WriteLine("Skipping SI " + std::to_string(subIdx) + " (gap)");
      msw.FillBits(siSize, false);
      continue;
    }

    // calculate subindex's size in whole bytes plus 0..7 bits
    size_t  const bytes = siSize / 8U;
    uint8_t const bits  = siSize % 8U;

    // Determine attributes and skip pure-ro subindices.
    // Data must be present for pure-ro subindices, so we add some zeros.
    auto const attributes = objPtr->GetSubIdxAttributes(subIdx);
    if ((attributes & Object::attr_ACCESS_WR) == 0U)
    {
      cli.WriteLine("Skipping SI " + std::to_string(subIdx) + " (pure read-only))");
      msw.FillBits(siSize, false);
      continue;
    }

    // ask user to enter value
    std::ostringstream oss;
    oss << "Enter value for SI " << subIdx << ", "
        << DataTypeToString(dataType) << ", "
        << AttributesToStringHook(objPtr->GetSubIdxAttributes(subIdx)) << ", "
        << bytes << '.' << static_cast<uint32_t>(bits) << " Byte(s), \"" << objPtr->GetSubIdxName(subIdx) << '"';
    cli.WriteLine(oss.str());

    auto val = cli.ReadLine("Value: ");

    switch (dataType)
    {
      case DataType::visible_string:
      {
        // check if leading and trailing '"' are present...
        if ((val.size() < 2U) || (val.front() != '"') || (val.back() != '"'))
          throw gpcc::cli::UserEnteredInvalidArgsError("DATA of type visible_string requires double-quotes.");

        // ...and remove them
        val = val.substr(1U, val.size() - 2U);

        try
        {
          StringToCANOpenEncodedData(val, siSize, DataType::visible_string, msw);
        }
        catch (std::bad_alloc const &) { throw; }
        catch (std::exception const &)
        {
          std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
        }

        break;
      }

      case DataType::octet_string:
      {
        auto const values = gpcc::string::Split(val, ' ', true);
        if (values.size() != bytes)
          throw gpcc::cli::UserEnteredInvalidArgsError("Size of DATA does not match size of subindex.");

        try
        {
          for (auto const & value: values)
            StringToCANOpenEncodedData(value, siSize, DataType::octet_string, msw);
        }
        catch (std::bad_alloc const &) { throw; }
        catch (std::exception const &)
        {
          std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
        }

        break;
      }

      case DataType::unicode_string:
      {
        auto const values = gpcc::string::Split(val, ' ', true);
        if (values.size() * 2U != bytes)
          throw gpcc::cli::UserEnteredInvalidArgsError("Size of DATA does not match size of subindex.");

        try
        {
          for (auto const & value: values)
            StringToCANOpenEncodedData(value, siSize, DataType::unicode_string, msw);
        }
        catch (std::bad_alloc const &) { throw; }
        catch (std::exception const &)
        {
          std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
        }

        break;
      }

      default:
      {
        if (val.empty())
          throw gpcc::cli::UserEnteredInvalidArgsError("No DATA entered.");

        try
        {
          StringToCANOpenEncodedData(val, siSize, dataType, msw);
        }
        catch (std::bad_alloc const &) { throw; }
        catch (std::exception const &)
        {
          std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
        }

        break;
      }
    } // switch (dataType)
  }

  if (msw.RemainingCapacity() > 1U)
    throw std::logic_error("Length of constructed binary is invalid!");

  if (msw.GetNbOfCachedBits() != (sizeInBit % 8U))
    throw std::logic_error("Length of constructed binary is invalid!");

  msw.Close();

  // ============================================
  // write
  // ============================================
  cli.WriteLine("All data entered.");
  if (cli.ReadLine("Write now? (y/n/Ctrl+C): ") == "y")
  {
    // determine access permissions
    auto const permissions = BeginAccessHook() & Object::attr_ACCESS_WR;
    ON_SCOPE_EXIT() { EndAccessHook(); };

    // lock object's data
    auto locker = objPtr->LockData();

    // determine expected ernob
    gpcc::Stream::MemStreamReader::RemainingNbOfBits ernob;
    switch (sizeInBit % 8U)
    {
      case 0U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero; break;
      case 1U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::seven; break;
      case 2U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::six; break;
      case 3U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::five; break;
      case 4U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::four; break;
      case 5U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::three; break;
      case 6U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::two; break;
      case 7U: ernob = gpcc::Stream::MemStreamReader::RemainingNbOfBits::one; break;
    }

    // do the actual write
    gpcc::Stream::MemStreamReader msr(data.data(), sizeInByte, gpcc::Stream::IStreamReader::nativeEndian);
    auto const result = objPtr->CompleteWrite(true, false, permissions, msr, ernob);
    msr.Close();

    if (result != SDOAbortCode::OK)
    {
      cli.WriteLine(std::string("Writing object failed: ") + SDOAbortCodeToDescrString(result));
      return;
    }

    cli.WriteLine("OK");
  }
  else
  {
    cli.WriteLine("Aborted. No data written.");
  }
}

/**
 * \brief Retrieves the number of digits a subindex number is comprised of.
 *
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param si
 * Subindex value.
 * \return
 * Number of digits the subindex value is comprised of.
 */
uint_fast8_t CLIAdapterBase::DigitsInSubindex(uint8_t const si) noexcept
{
  if (si > 99U)
    return 3U;

  if (si > 9U)
    return 2U;

  return 1U;
}

} // namespace cood
} // namespace gpcc
