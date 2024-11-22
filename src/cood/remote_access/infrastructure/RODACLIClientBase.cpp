/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021, 2024 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/infrastructure/RODACLIClientBase.hpp>
#include <gpcc/cli/exceptions.hpp>
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cood/cli/string_conversion.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequestResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp>
#include <gpcc/osal/definitions.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include <gpcc/stream/MemStreamReader.hpp>
#include <gpcc/stream/MemStreamWriter.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <gpcc/string/tools.hpp>
#include "src/cood/cli/internal/CAReadArgsParser.hpp"
#include "src/cood/cli/internal/CAWriteArgsParser.hpp"
#include "src/cood/cli/internal/EnumerateArgsParser.hpp"
#include "src/cood/cli/internal/InfoArgsParser.hpp"
#include "src/cood/cli/internal/ReadArgsParser.hpp"
#include "src/cood/cli/internal/WriteArgsParser.hpp"
#include <stdexcept>
#include <cstring>

namespace gpcc {
namespace cood {

uint16_t constexpr RODACLIClientBase::rxTimeout_ms;

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _cli
 * CLI where the CLI command will be registered by the derived class.
 *
 * \param _attributeStringMaxLength
 * Maximum length of any string that could be returned by @ref AttributesToStringHook().
 */
RODACLIClientBase::RODACLIClientBase(gpcc::cli::CLI & _cli, uint8_t const _attributeStringMaxLength)
: IRemoteObjectDictionaryAccessNotifiable()
, cli(_cli)
, attributeStringMaxLength(_attributeStringMaxLength)
, ownerID(0U)
, connectMutex()
, internalMutex()
, state(States::notRegistered)
, pRODA(nullptr)
, maxRequestSize(0U)
, maxResponseSize(0U)
, sessionCnt(0U)
, spReceivedResponse()
, receiveOverflow(false)
, respReceivedConVar()
, stateChangeConVar()
{
  auto const uipThis = reinterpret_cast<uintptr_t>(this);
  #if UINTPTR_MAX == 0xFFFFFFFFUL
    ownerID = static_cast<uint32_t>(uipThis);
  #elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFULL
    ownerID = (static_cast<uint32_t>(uipThis)) ^ (static_cast<uint32_t>(uipThis >> 32U));
  #else
    #error "Cannot determine pointer size."
  #endif
}

/**
 * \brief Destructor.
 *
 * \pre   The client shall not be connected to any RODA interface.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
RODACLIClientBase::~RODACLIClientBase(void)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::notRegistered)
    gpcc::osal::Panic("RODACLIClientBase::~RODACLIClientBase: Still connected to RODA interface!");
}

/**
 * \brief Connects the client to a given RODA interface.
 *
 * @ref Disconnect() is the counterpart of this method.
 *
 * \pre   The client is not connected to any RODA interface yet.
 *
 * \post  The client is connected to the RODA interface.
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
 * \param rodaItf
 * RODA interface the client shall be connected to.
 */
void RODACLIClientBase::Connect(IRemoteObjectDictionaryAccess & rodaItf)
{
  gpcc::osal::MutexLocker connectMutexLocker(connectMutex);

  {
    gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

    if (state != States::notRegistered)
      throw std::logic_error("RODACLIClientBase::Connect: Already connected.");

    stateChangeConVar.Signal();
    state = States::notReady;
    pRODA = &rodaItf;
  }

  ON_SCOPE_EXIT(undo)
  {
    gpcc::osal::MutexLocker internalMutexLocker(internalMutex);
    pRODA = nullptr;
    Reset(States::notRegistered);
  };

  pRODA->Register(this);

  ON_SCOPE_EXIT_DISMISS(undo);
}

/**
 * \brief Disconnects the client from the RODA interface.
 *
 * This is the counterpart of @ref Connect().
 *
 * \pre   The client is registered at a RODA interface.
 *
 * \post  The client is not registered at any RODA interface.
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
 * No cancellation point included.
 */
void RODACLIClientBase::Disconnect(void)
{
  gpcc::osal::MutexLocker connectMutexLocker(connectMutex);

  {
    gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

    if (state == States::notRegistered)
      throw std::logic_error("RODACLIClientBase::Disconnect: Already disconnected");
  }

  try
  {
    pRODA->Unregister();

    gpcc::osal::MutexLocker internalMutexLocker(internalMutex);
    pRODA = nullptr;
    Reset(States::notRegistered);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Retrieves a pointer to the RODA interface this client is currently connected to.
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
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Pointer to the RODA interface this client is currently connected to.\n
 * nullptr = none.
 */
IRemoteObjectDictionaryAccess* RODACLIClientBase::GetCurrentlyConnectedRODAItf(void)
{
  gpcc::osal::MutexLocker connectMutexLocker(connectMutex);
  return pRODA;
}

/**
 * \brief Waits (with timeout) until the RODA interface this client is connected to becomes ready.
 *
 * \pre   This class instance is connected to a RODA interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be invoked by one thread only. If more than one thread is blocked in this, then not all threads
 * may be woken up when the RODA interface becomes ready.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param timeout_ms
 * Timeout in ms.
 *
 * \retval true   RODA interface is ready.
 * \retval false  Timeout. RODA interface did not become ready.
 */
bool RODACLIClientBase::WaitForRODAItfReady(uint16_t const timeout_ms)
{
  if (timeout_ms == 0U)
    throw std::invalid_argument("RODACLIClientBase::WaitForRODAItfReady: 'timeout_ms' invalid");

  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state == States::ready)
  {
    return true;
  }
  else if (state == States::notRegistered)
  {
    throw std::logic_error("RODACLIClientBase::WaitForRODAItfReady: Not connected");
  }
  else
  {
    auto const timeout = gpcc::time::TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID)
                       + gpcc::time::TimeSpan::ms(timeout_ms);

    while (state != States::ready)
    {
      if (stateChangeConVar.TimeLimitedWait(internalMutex, timeout))
      {
        // (timeout)
        return (state == States::ready);
      }
    }

    return true;
  }
}

/**
 * \brief CLI command handler for enumeration of objects.
 *
 * This is intended to be invoked by a CLI handler implemented by a subclass.
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
void RODACLIClientBase::CLI_Enumerate(std::string const & restOfLine)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    throw std::runtime_error("RODA interface not ready or not connected");

  // ============================================
  // Analyse args
  // ============================================
  internal::EnumerateArgsParser args(restOfLine);

  // ============================================
  // Enumerate and print to CLI
  // ============================================
  std::unique_ptr<ObjectEnumResponse> spEnumResponse;
  bool firstLoopCycle = true;
  uint16_t startIndex = args.GetFirstIndex();
  do
  {
    cli.TestTermination();

    spEnumResponse.reset();
    spEnumResponse = Enumerate(startIndex, args.GetLastIndex(), 1U, 0xFFFFU);

    auto const & indices = spEnumResponse->GetIndices();
    if (indices.empty())
    {
      if (firstLoopCycle)
        cli.WriteLine("No objects");

      return;
    }

    for (auto const index : indices)
    {
      cli.TestTermination();

      // collect some more data...
      auto const spInfoResponse = GetInfoSingleSI(index, 0U, true, false);
      auto const objCode  = spInfoResponse->GetObjectCode();
      auto const dataType = spInfoResponse->GetObjectDataType();
      auto const objName  = spInfoResponse->GetObjectName();

      // ...then print to CLI
      using gpcc::string::StringComposer;
      StringComposer sc;
      sc << gpcc::string::ToHex(index, 4U) << ' '
          << StringComposer::AlignLeft << StringComposer::Width(Object::largestObjectCodeNameLength) << Object::ObjectCodeToString(objCode) << ' '
          << StringComposer::AlignLeft << StringComposer::Width(15) << DataTypeToString(dataType) << " \"" << objName << '"';

      cli.WriteLine(sc.Get());
    }

    firstLoopCycle = false;
  }
  while (!spEnumResponse->IsComplete(&startIndex));
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
void RODACLIClientBase::CLI_Info(std::string const & restOfLine)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    throw std::runtime_error("RODA interface not ready or not connected");

  // ============================================
  // Analyse args
  // ============================================
  internal::InfoArgsParser args(restOfLine);

  // ============================================
  // Query
  // ============================================
  auto spInfo = GetInfo(args.GetIndex(), true, args.GetInclASM());

  // ============================================
  // Print to CLI
  // ============================================
  using gpcc::string::StringComposer;
  StringComposer sc;

  // -- print info about object --
  sc << "Object " << gpcc::string::ToHex(args.GetIndex(), 4U) << ": " << Object::ObjectCodeToString(spInfo->GetObjectCode())
      << " (" << DataTypeToString(spInfo->GetObjectDataType()) << ") \"" << spInfo->GetObjectName() << '"';
  cli.WriteLine(sc.Get());

  // small tool: Appends info about a subindex to 'sc'
  auto appendSubIndexInfoToOSS = [&](uint_fast8_t const si)
  {
    size_t  const s     = spInfo->GetSubIdxMaxSize(si);
    size_t  const bytes = s / 8U;
    uint8_t const bits  = s % 8U;
    sc << StringComposer::AlignLeft << StringComposer::Width(15U)
        << DataTypeToString(spInfo->GetSubIdxDataType(si)) << ' '
        << StringComposer::Width(attributeStringMaxLength)
        << AttributesToStringHook(spInfo->GetSubIdxAttributes(si)) << ' '
        << StringComposer::AlignRight << StringComposer::Width(5U)
        << bytes << '.' << static_cast<uint32_t>(bits) << " Byte(s) \"" << spInfo->GetSubIdxName(si) << '"';
  };

  // small tool: Appends human readable textual representation of the app-specific meta data of a subindex to 'sc'
  auto appendAppSpecMetaDataToOSS = [&](uint_fast8_t const si)
  {
    size_t const s = spInfo->GetAppSpecificMetaDataSize(si);
    if (s == 0U)
    {
      sc << "No app-specific meta data.";
    }
    else
    {
      auto const data = spInfo->GetAppSpecificMetaData(si);
      sc << AppSpecificMetaDataToStringHook(data);
    }
  };

  // -- print info about subinces --
  // get maximum number of subindices
  auto maxNbOfSIs = spInfo->GetMaxNbOfSubindices();

  // Get number of digits required to print 'maxNbOfSIs'. This will be used for proper alignment of rows.
  auto digitsForSubindices = DigitsInSubindex(maxNbOfSIs - 1U);

  // compress the output for ARRAY objects if possible
  if ((!args.GetInclASM()) && (spInfo->GetObjectCode() == Object::ObjectCode::Array))
  {
    sc.Clear();
    sc << "  Subindex    " << StringComposer::AlignRight << StringComposer::Width(digitsForSubindices + 2U) << "0: ";
    appendSubIndexInfoToOSS(0U);
    cli.WriteLine(sc.Get());

    if (maxNbOfSIs > 1U)
    {
      sc.Clear();
      sc << "  Subindex 1.." << static_cast<uint32_t>(maxNbOfSIs - 1U) << ": ";
      appendSubIndexInfoToOSS(1U);
      cli.WriteLine(sc.Get());
    }
  }
  else
  {
    for (uint_fast16_t i = 0U; i < maxNbOfSIs; i++)
    {
      sc.Clear();
      sc << "  Subindex " << StringComposer::Width(digitsForSubindices) << static_cast<uint32_t>(i) << ": ";
      if (spInfo->IsSubIndexEmpty(i))
      {
        sc << "empty";
      }
      else
      {
        appendSubIndexInfoToOSS(i);
      }

      if (args.GetInclASM())
      {
        sc << gpcc::osal::endl << "             ";
        for (uint_fast8_t j = 0U; j < digitsForSubindices; j++)
          sc << ' ';
        appendAppSpecMetaDataToOSS(i);
      }

      cli.WriteLine(sc.Get());
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
void RODACLIClientBase::CLI_Read(std::string const & restOfLine)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    throw std::runtime_error("RODA interface not ready or not connected");

  // ============================================
  // Analyse args
  // ============================================
  internal::ReadArgsParser args(restOfLine);

  // ============================================
  // Read data and meta data
  // ============================================
  auto spReadResponse = Read(args.GetIndex(), args.GetSubIndex(), false);
  auto spSIInfo = GetInfoSingleSI(args.GetIndex(), args.GetSubIndex(), false, false);

  // ============================================
  // Print to CLI
  // ============================================
  gpcc::stream::MemStreamReader msr(spReadResponse->GetData().data(),
                                    spReadResponse->GetData().size(),
                                    gpcc::stream::IStreamReader::Endian::Little);

  auto str = CANopenEncodedDataToString(msr, spReadResponse->GetDataSize(), spSIInfo->GetSubIdxDataType(args.GetSubIndex()));
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
void RODACLIClientBase::CLI_Write(std::string const & restOfLine)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    throw std::runtime_error("RODA interface not ready or not connected");

  // ============================================
  // Analyze args (first part: index and subindex)
  // ============================================
  internal::WriteArgsParser args(restOfLine);

  // ============================================
  // Query info about subindex
  // ============================================
  auto const spInfo = GetInfoSingleSI(args.GetIndex(), args.GetSubIndex(), false, false);
  DataType const dataType = spInfo->GetSubIdxDataType(args.GetSubIndex());
  size_t const subIdxMaxSize = spInfo->GetSubIdxMaxSize(args.GetSubIndex());

  // ============================================
  // Analyze args (second part: data)
  // ============================================
  args.ExtractData(dataType, subIdxMaxSize, gpcc::stream::IStreamWriter::Endian::Little);

  // ============================================
  // write to object
  // ============================================
  Write(args.GetIndex(), args.GetSubIndex(), false, std::move(args.GetData()));
  cli.WriteLine("OK");
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
void RODACLIClientBase::CLI_CARead(std::string const & restOfLine)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    throw std::runtime_error("RODA interface not ready or not connected");

  // ============================================
  // Analyse args
  // ============================================
  internal::CAReadArgsParser args(restOfLine);

  // ============================================
  // Read data and meta data
  // ============================================
  auto spSIInfo = GetInfo(args.GetIndex(), args.GetVerbose(), false);
  auto spReadResponse = Read(args.GetIndex(), 0U, true);

  // ============================================
  // Print to CLI
  // ============================================
  gpcc::stream::MemStreamReader msr(spReadResponse->GetData().data(),
                                    spReadResponse->GetData().size(),
                                    gpcc::stream::IStreamReader::Endian::Little);

  // extract value of SI0
  uint8_t const si0 = msr.Read_uint8();
  auto const digitsForSubIdx = DigitsInSubindex(si0);

  if (args.GetVerbose())
  {
    // determine padding for data type column and name column
    size_t paddingDataType = 0U;
    size_t paddingName = 0U;

    for (uint_fast16_t subIdx = 0U; subIdx <= si0; ++subIdx)
    {
      size_t s = strlen(DataTypeToString(spSIInfo->GetSubIdxDataType(subIdx)));
      if (paddingDataType < s)
        paddingDataType = s;

      s = spSIInfo->GetSubIdxName(subIdx).length();
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
      using gpcc::string::StringComposer;
      auto const dataType = spSIInfo->GetSubIdxDataType(subIdx);

      StringComposer sc;
      sc << "SI " << StringComposer::AlignLeft << StringComposer::Width(digitsForSubIdx) << subIdx << ' '
          << StringComposer::Width(paddingDataType) << DataTypeToString(dataType) << ' '
          << StringComposer::Width(paddingName) << spSIInfo->GetSubIdxName(subIdx) << " : ";

      if (subIdx == 0U)
        sc << static_cast<unsigned int>(si0);
      else
        sc << CANopenEncodedDataToString(msr, spSIInfo->GetSubIdxMaxSize(subIdx), dataType);

      cli.WriteLine(sc.Get());
    }
  }
  else
  {
    for (uint_fast16_t subIdx = 0U; subIdx <= si0; ++subIdx)
    {
      using gpcc::string::StringComposer;
      StringComposer sc;
      sc << "SI " << StringComposer::AlignLeft << StringComposer::Width(digitsForSubIdx) << subIdx << ": ";

      if (subIdx == 0U)
        sc << static_cast<unsigned int>(si0);
      else
        sc << CANopenEncodedDataToString(msr, spSIInfo->GetSubIdxMaxSize(subIdx), spSIInfo->GetSubIdxDataType(subIdx));

      cli.WriteLine(sc.Get());
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
void RODACLIClientBase::CLI_CAWrite(std::string const & restOfLine)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    throw std::runtime_error("RODA interface not ready or not connected");

  // ============================================
  // Analyse args
  // ============================================
  internal::CAWriteArgsParser args(restOfLine);

  // ============================================
  // Read meta data
  // ============================================
  auto spInfo = GetInfo(args.GetIndex(), true, false);

  // ============================================
  // Object supported?
  // ============================================
  if (   (spInfo->GetObjectCode() != Object::ObjectCode::Array)
      && (spInfo->GetObjectCode() != Object::ObjectCode::Record))
  {
    cli.WriteLine("Object type not supported.");
    return;
  }

  // ============================================
  // Determine current value of SI0
  // ============================================
  auto spResponse = Read(args.GetIndex(), 0U, false);
  uint8_t const currSI0 = spResponse->GetData().at(0U);

  // ============================================
  // Ask user to enter value for SI0
  // ============================================
  uint8_t newSI0 = currSI0;

  // Is SI0 writeable?
  if ((spInfo->GetSubIdxAttributes(0U) & Object::attr_ACCESS_WR) != 0U)
  {
    if (spInfo->GetObjectCode() != Object::ObjectCode::Array)
    {
      cli.WriteLine("SI0 is writeable. This is only supported for ARRAY objects.");
      return;
    }

    cli.WriteLine("Current value of SI0: " + std::to_string(currSI0));
    newSI0 = gpcc::string::DecimalToU8(cli.ReadLine("New value for SI0: "));

    if (newSI0 >= spInfo->GetMaxNbOfSubindices())
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
    auto const dataType = spInfo->GetSubIdxDataType(subIdx);

    // align to byte boundary, if data type is byte-based
    if (!IsDataTypeBitBased(dataType))
      sizeInBit += (8U - (sizeInBit % 8U)) % 8U;

    sizeInBit += spInfo->GetSubIdxMaxSize(subIdx);
  }

  // allocate memory
  size_t sizeInByte = (sizeInBit + 7U) / 8U;
  std::vector<uint8_t> data(sizeInByte);
  gpcc::stream::MemStreamWriter msw(data.data(), sizeInByte, gpcc::stream::MemStreamWriter::Endian::Little);

  // ============================================
  // Fill buffer with write data entered by the user
  // ============================================
  // write SI0
  msw.Write_uint8(newSI0);

  // write the data for the other subindices
  for (uint_fast16_t subIdx = 1U; subIdx <= newSI0; ++subIdx)
  {
    // determine data type and size
    auto const dataType = spInfo->GetSubIdxDataType(subIdx);
    size_t const siSize = spInfo->GetSubIdxMaxSize(subIdx);

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
    auto const attributes = spInfo->GetSubIdxAttributes(subIdx);
    if ((attributes & Object::attr_ACCESS_WR) == 0U)
    {
      cli.WriteLine("Skipping SI " + std::to_string(subIdx) + " (pure read-only))");
      msw.FillBits(siSize, false);
      continue;
    }

    // ask user to enter value
    gpcc::string::StringComposer sc;
    sc << "Enter value for SI " << subIdx << ", "
        << DataTypeToString(dataType) << ", "
        << AttributesToStringHook(spInfo->GetSubIdxAttributes(subIdx)) << ", "
        << bytes << '.' << static_cast<uint32_t>(bits) << " Byte(s), \"" << spInfo->GetSubIdxName(subIdx) << '"';
    cli.WriteLine(sc.Get());

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
    Write(args.GetIndex(), 0U, true, std::move(data));
    cli.WriteLine("OK");
  }
  else
  {
    cli.WriteLine("Aborted. No data written.");
  }
}

// Doc: See RODACLIClientBase.hpp
std::string RODACLIClientBase::AppSpecificMetaDataToStringHook(std::vector<uint8_t> const & data)
{
  using gpcc::string::StringComposer;
  StringComposer s;
  s << data.size() << " byte(s) of ASM";

  if (data.size() != 0U)
  {
    s << ':';

    uint_fast8_t maxPrintedBytes = 16U;

    s << StringComposer::AlignRightPadZero << StringComposer::BaseHex << StringComposer::Uppercase;
    auto it = data.begin();
    while (it != data.end())
    {
      if (maxPrintedBytes-- == 0U)
      {
        s << "...";
        break;
      }

      s << ' ' << StringComposer::Width(2) << static_cast<unsigned int>(*it);
      ++it;
    }
  }

  return s.Get();
}

/**
 * \brief Retrieves the number of digits a subindex number is comprised of.
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
 *
 * \return
 * Number of digits the subindex value is comprised of.
 */
uint_fast8_t RODACLIClientBase::DigitsInSubindex(uint8_t const si) noexcept
{
  if (si > 99U)
    return 3U;

  if (si > 9U)
    return 2U;

  return 1U;
}

// <-- IRemoteObjectDictionaryAccessNotifiable

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady
void RODACLIClientBase::OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::notReady)
    gpcc::osal::Panic("RODACLIClientBase::OnReady: 'state' invalid");

  if (maxRequestSize < (RequestBase::minimumUsefulRequestSize + ReturnStackItem::binarySize))
    this->maxRequestSize = 0U;
  else
    this->maxRequestSize = maxRequestSize - ReturnStackItem::binarySize;

  if (maxResponseSize < (ResponseBase::minimumUsefulResponseSize + ReturnStackItem::binarySize))
    this->maxResponseSize = 0U;
  else
    this->maxResponseSize = maxResponseSize - ReturnStackItem::binarySize;

  state = States::ready;
  stateChangeConVar.Signal();
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnDisconnected
void RODACLIClientBase::OnDisconnected(void) noexcept
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::ready)
    gpcc::osal::Panic("RODACLIClientBase::OnDisconnected: 'state' invalid");

  Reset(States::notReady);
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed
void RODACLIClientBase::OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  // sanity check
  if (state != States::ready)
    gpcc::osal::Panic("RODACLIClientBase::OnDisconnected: 'state' invalid");

  // Extract return stack item and check:
  // - Are we the originator?
  // - Does the response belong to the current session?
  // In case of any mismatch we just discard the response.
  if (spResponse->IsReturnStackEmpty())
    return;

  auto rsi = spResponse->PopReturnStack();
  if (rsi.GetID() != ownerID)
    return;

  if (rsi.GetInfo() != sessionCnt)
    return;

  // check for overflow and fetch response or set receiveOverflow
  if (!spReceivedResponse)
    spReceivedResponse = std::move(spResponse);
  else
    receiveOverflow = true;

  respReceivedConVar.Signal();
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext
void RODACLIClientBase::LoanExecutionContext(void) noexcept
{
  // empty by intention
}

// --> IRemoteObjectDictionaryAccessNotifiable

/**
 * \brief Resets class members according to requirements when the RODA interface becomes not-ready or disconnected.
 *
 * @ref pRODA is not modified by this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newState
 * New value for @ref state.
 */
void RODACLIClientBase::Reset(States const newState) noexcept
{
  if (state != newState)
    stateChangeConVar.Signal();

  state = newState;
  maxRequestSize = 0U;
  maxResponseSize = 0U;
  sessionCnt = 0U;
  spReceivedResponse.reset();
  receiveOverflow = false;
}

/**
 * \brief Blocks the calling thread until a response is received or a timeout occurrs.
 *
 * Messages are received via @ref RODACLIClientBase::OnRequestProcessed() of the private provided RODAN interface.
 * @ref OnRequestProcessed() will check @ref sessionCnt and @ref ownerID to the return stack item attached to the
 * response.
 *
 * In case of an error, the caller may want to increase @ref sessionCnt.
 *
 * \post  @ref receiveOverflow is cleared.
 *
 * \post  @ref spReceivedResponse is cleared.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   Timeout while waiting for response or receive overrun.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Received response message.
 */
std::unique_ptr<ResponseBase> RODACLIClientBase::WaitAndFetchResponse(uint32_t const timeout_ms)
{
  if (!spReceivedResponse)
  {
    auto const timeout = gpcc::time::TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID)
                       + gpcc::time::TimeSpan::ms(timeout_ms);

    while (!spReceivedResponse)
    {
      if (respReceivedConVar.TimeLimitedWait(internalMutex, timeout))
      {
        // timeout
        if (!spReceivedResponse)
          throw std::runtime_error("RODACLIClientBase::WaitAndFetchResponse: Timeout waiting for response from remote access server");
        else
          break;
      }
    }
  }

  if (receiveOverflow)
  {
    spReceivedResponse.reset();
    receiveOverflow = false;
    throw std::runtime_error("RODACLIClientBase::WaitAndFetchResponse: Receive overflow");
  }

  return std::move(spReceivedResponse);
}

/**
 * \brief Transmits a requests and waits for the response (with timeout).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - If the error occurred _after_ successful transmission of the request, then @ref sessionCnt will be incremented.
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * \throws std::runtime_error                Timeout while waiting for response or receive overrun.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - If thread cancellation occurrs _after_ successful transmission of the request, then @ref sessionCnt will be
 *   incremented.
 *
 * - - -
 *
 * \param spReq
 * Request that shall be transmitted. Ownership moves to this method in any case. The object will be consumed even in
 * case of an excpetion.\n
 * nullptr is not allowed.
 *
 * \return
 * Received response.
 */
std::unique_ptr<ResponseBase> RODACLIClientBase::TxAndRx(std::unique_ptr<RequestBase> spReq)
{
  if (!spReq)
    throw std::invalid_argument("RODACLIClientBase::TxAndRx: !spReq");

  ReturnStackItem rsi(ownerID, sessionCnt);
  spReq->Push(rsi);

  pRODA->Send(spReq);

  ON_SCOPE_EXIT(incSessionCount)
  {
    ++sessionCnt;
  };

  auto resp = WaitAndFetchResponse(rxTimeout_ms);

  ON_SCOPE_EXIT_DISMISS(incSessionCount);

  return resp;
}

/**
 * \brief Enumerates the objects in the remote object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * - - -
 *
 * \param firstIndex
 * Index where enumeration shall start.\n
 * Objects located at indices less than this will not be enumerated.
 *
 * \param lastIndex
 * Index where enumeration shall stop.\n
 * Objects located at indices larger than this will not be enumerated.
 *
 * \param maxFragments
 * Each response object received from the remote access server contains @ref maxResponseSize bytes of data (both payload
 * and overhead). It may happen, that the indices of all objects up to `lastIndex` do not fit into the response object.
 * It that case, this method will send another request to continue enumeration. This is called a _fragmented transfer_.\n
 * \n
 * This parameter limits the number of fragments. If the number of transmitted requests and received responses reaches
 * the value of this parameter, then the enumeration will stop regardless if all objects up to 'lastIndex' have been
 * enumerated or not.\n
 * A potentially incomplete @ref ObjectEnumResponse object will be returned. The status of the returned object
 * (complete/incomplete and where to continue enumeration) can be queried via @ref ObjectEnumResponse::IsComplete().
 * The enumeration can then be seamlessly continued with a second call to this.\n
 * Zero is not allowed.
 *
 * \param attrFilter
 * Attribute-filter for enumeration.\n
 * Only objects with at least one matching attribute bit will be enumerated.\n
 * Zero is not allowed. 0xFFFFU will enumerate all objects.
 *
 * \return
 * @ref ObjectEnumResponse object containing the indices of the enumerated objects.\n
 * The status code of the returned object is always @ref SDOAbortCode::OK, otherwise this will throw.\n
 * The enumeration may be incomplete, use @ref ObjectEnumResponse::IsComplete() to check and also refer to
 * parameter `maxFragments`.
 */
std::unique_ptr<ObjectEnumResponse> RODACLIClientBase::Enumerate(uint16_t const firstIndex,
                                                                 uint16_t const lastIndex,
                                                                 uint16_t maxFragments,
                                                                 Object::attr_t const attrFilter)
{
  if (maxFragments == 0U)
    throw std::invalid_argument("RODACLIClientBase::Enumerate: 'maxFragments' is zero");

  // create initial enum request
  auto spRequest = std::make_unique<ObjectEnumRequest>(firstIndex, lastIndex, attrFilter, maxResponseSize);

  // this will take the first response and potential fragments will be added to it
  std::unique_ptr<ObjectEnumResponse> spFirstResponse;

  while (true)
  {
    // transmit the request
    auto spResponse = TxAndRx(std::move(spRequest));

    // check response type
    if (spResponse->GetType() != ResponseBase::ResponseTypes::objectEnumResponse)
      throw std::runtime_error("RODACLIClientBase::Enumerate: Received unexpected response type.");

    auto & response = dynamic_cast<ObjectEnumResponse&>(*spResponse);

    // examine the result
    if (response.GetResult() != SDOAbortCode::OK)
    {
      throw std::runtime_error(std::string("RODACLIClientBase::Enumerate: Enum request failed (" ) +
                               SDOAbortCodeToDescrString(response.GetResult()) + ")");
    }

    // first response?
    if (!spFirstResponse)
    {
      // first response: move to spFirstResponse
      spFirstResponse.reset(&response);
      spResponse.release();
    }
    else
    {
      // not first: append fragment to *spFirstResponse
      spFirstResponse->AddFragment(std::move(response));
    }

    // Done? If yes, break the loop
    uint16_t nextIndex = 0U;
    if (spFirstResponse->IsComplete(&nextIndex))
      break;

    // maximum number of fragments reached?
    if (--maxFragments == 0U)
      break;

    // create next request which continues the query
    spRequest = std::make_unique<ObjectEnumRequest>(nextIndex, lastIndex, attrFilter, maxResponseSize);
  }

  return spFirstResponse;
}

/**
 * \brief Retrieves information about an object from the remote object dictionary and all its subindices.
 *
 * If a fragmented transfer takes place, then the CLI will be polled for CTRL+C to abort the transfer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * - - -
 *
 * \param index
 * Index of the object.
 *
 * \param inclNames
 * Controls if the names of the object and the subindes shall be queried (true) or not (false).\n
 * Including the names may increase the amounth of transferred data significantly.
 *
 * \param inclASM
 * Controls if application specific meta data of the subindices shall be queried (true) or not (false).\n
 * Including application specific meta data may increase the amounth of transferred data significantly.
 *
 * \return
 * @ref ObjectInfoResponse object containing the requested information.\n
 * The status code of the returned object is always @ref SDOAbortCode::OK, otherwise this will throw.
 */
std::unique_ptr<ObjectInfoResponse> RODACLIClientBase::GetInfo(uint16_t const index,
                                                               bool const inclNames,
                                                               bool const inclASM)
{
  // create initial info request
  auto spRequest = std::make_unique<ObjectInfoRequest>(index, 0U, 255U, inclNames, inclASM, maxResponseSize);

  // this will take the first response and potential fragments will be added to it
  std::unique_ptr<ObjectInfoResponse> spFirstResponse;

  while (true)
  {
    cli.TestTermination();

    // transmit the request
    auto spResponse = TxAndRx(std::move(spRequest));

    // check response type
    if (spResponse->GetType() != ResponseBase::ResponseTypes::objectInfoResponse)
      throw std::runtime_error("RODACLIClientBase::GetInfo: Received unexpected response type.");

    auto & response = dynamic_cast<ObjectInfoResponse&>(*spResponse);

    // examine the result
    if (response.GetResult() != SDOAbortCode::OK)
    {
      throw std::runtime_error(std::string("RODACLIClientBase::GetInfo: Info request failed (" ) +
                               SDOAbortCodeToDescrString(response.GetResult()) + ")");
    }

    // first response?
    if (!spFirstResponse)
    {
      // first response: move to spFirstResponse
      spFirstResponse.reset(&response);
      spResponse.release();
    }
    else
    {
      // not first: append fragment to *spFirstResponse
      spFirstResponse->AddFragment(std::move(response));
    }

    // Done? If yes, break the loop
    uint8_t nextSubIndex = 0U;
    if (spFirstResponse->IsComplete(&nextSubIndex))
      break;

    // create next request which continues the query
    spRequest = std::make_unique<ObjectInfoRequest>(index, nextSubIndex, 255U, inclNames, inclASM, maxResponseSize);
  }

  return spFirstResponse;
}

/**
 * \brief Retrieves information about a single subindex of an object contained in the remote object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * - - -
 *
 * \param index
 * Index of the object.
 *
 * \param subIndex
 * Number of the subindex.
 *
 * \param inclNames
 * Controls if the names of the object and of the subindex shall be queried (true) or not (false).\n
 * Including the names may increase the amounth of transferred data significantly.
 *
 * \param inclASM
 * Controls if application specific meta data of the subindex shall be queried (true) or not (false).\n
 * Including application specific meta data may increase the amounth of transferred data significantly.
 *
 * \return
 * @ref ObjectInfoResponse object containing the requested information.\n
 * The status code of the returned object is always @ref SDOAbortCode::OK, otherwise this will throw.
 */
std::unique_ptr<ObjectInfoResponse> RODACLIClientBase::GetInfoSingleSI(uint16_t const index,
                                                                       uint8_t const subIndex,
                                                                       bool const inclNames,
                                                                       bool const inclASM)
{
  // create info request
  auto spRequest = std::make_unique<ObjectInfoRequest>(index, subIndex, subIndex, inclNames, inclASM, maxResponseSize);

  // transmit the request
  auto spResponse = TxAndRx(std::move(spRequest));

  // check response type
  if (spResponse->GetType() != ResponseBase::ResponseTypes::objectInfoResponse)
    throw std::runtime_error("RODACLIClientBase::GetInfoSingleSI: Received unexpected response type.");

  auto & response = dynamic_cast<ObjectInfoResponse&>(*spResponse);

  // examine the result
  if (response.GetResult() != SDOAbortCode::OK)
  {
    throw std::runtime_error(std::string("RODACLIClientBase::GetInfoSingleSI: Info request failed (" ) +
                             SDOAbortCodeToDescrString(response.GetResult()) + ")");
  }

  // create unique_ptr to ObjectInfoResponse
  std::unique_ptr<ObjectInfoResponse> spSpecificResponse(&response);
  spResponse.release();

  return spSpecificResponse;
}

/**
 * \brief Reads a subindex or a complete object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * - - -
 *
 * \param index
 * Index of the object.
 *
 * \param subindex
 * Number of the subindex that shall be read, or number of the subindex where complete access shall start.
 *
 * \param ca
 * Controls if the complete object shall be read using complete access:\n
 * true = complete access; `subIndex` shall be 0 or 1; SI0 is read using 8bit.\n
 * false = single subindex access
 *
 * \return
 * @ref ReadRequestResponse object containing the read data.\n
 * The status code of the returned object is always @ref SDOAbortCode::OK, otherwise this will throw.
 */
std::unique_ptr<ReadRequestResponse> RODACLIClientBase::Read(uint16_t const index, uint8_t const subindex, bool const ca)
{
  // create read request
  auto spRequest = std::make_unique<ReadRequest>(ca ? ReadRequest::AccessType::completeAccess_SI0_8bit : ReadRequest::AccessType::singleSubindex,
                                                 index, subindex,
                                                 Object::attr_ACCESS_RD,
                                                 maxResponseSize);

  // transmit the request
  auto spResponse = TxAndRx(std::move(spRequest));

  // check response type
  if (spResponse->GetType() != ResponseBase::ResponseTypes::readRequestResponse)
    throw std::runtime_error("RODACLIClientBase::Read: Received unexpected response type.");

  auto & response = dynamic_cast<ReadRequestResponse&>(*spResponse);

  // examine the result
  if (response.GetResult() != SDOAbortCode::OK)
  {
    throw std::runtime_error(std::string("RODACLIClientBase::Read: Read request failed (" ) +
                             SDOAbortCodeToDescrString(response.GetResult()) + ")");
  }

  // create unique_ptr to ReadRequestResponse
  std::unique_ptr<ReadRequestResponse> spSpecificResponse(&response);
  spResponse.release();

  return spSpecificResponse;
}

/**
 * \brief Writes to a subindex or to a complete object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The caller shall have @ref internalMutex acquired.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of 'data' may have been moved somewhere
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * \throws std::runtime_error                Write request response contains a bad SDO abort code.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of 'data' may have been moved somewhere
 * - @ref sessionCnt will be incremented if a request has been transmitted and its response is outstanding
 *
 * - - -
 *
 * \param index
 * Index of the object.
 *
 * \param subindex
 * Number of the subindex that shall be written or number of the subindex where complete access shall start.
 *
 * \param ca
 * Controls if the complete object shall be written using complete access:\n
 * true = complete access; `subIndex` shall be 0 or 1; SI0 is written using 8bit.\n
 * false = single subindex access
 *
 * \param data
 * Universal reference to a vector containing the data that shall be written.\n
 * The content of the vector will be moved into the request object.\n
 * If bit based data shall be written, then bits shall be filled from LSB to MSB. Unused upper bits will be ignored
 * and should be zero. The written CAN object knows the exact size of the subindex.
 */
void RODACLIClientBase::Write(uint16_t const index, uint8_t const subindex, bool const ca, std::vector<uint8_t> && data)
{
  // create write request
  auto spRequest = std::make_unique<WriteRequest>(ca ? WriteRequest::AccessType::completeAccess_SI0_8bit : WriteRequest::AccessType::singleSubindex,
                                                  index, subindex,
                                                  Object::attr_ACCESS_WR,
                                                  std::move(data),
                                                  maxResponseSize);
  // transmit the request
  auto spResponse = TxAndRx(std::move(spRequest));

  // check response type
  if (spResponse->GetType() != ResponseBase::ResponseTypes::writeRequestResponse)
    throw std::runtime_error("RODACLIClientBase::Write: Received unexpected response type.");

  auto & response = dynamic_cast<WriteRequestResponse&>(*spResponse);

  // examine the result
  if (response.GetResult() != SDOAbortCode::OK)
  {
    throw std::runtime_error(std::string("RODACLIClientBase::Write: Write request failed (" ) +
                             SDOAbortCodeToDescrString(response.GetResult()) + ")");
  }
}

} // namespace cood
} // namespace gpcc
