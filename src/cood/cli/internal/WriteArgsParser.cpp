/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "WriteArgsParser.hpp"
#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/cood/cli/string_conversion.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include <gpcc/string/tools.hpp>
#include <stdexcept>
#include <exception>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \brief Constructor.
 *
 * After object construction, the index and subindex from the user's arguments are available.
 * Use @ref ExtractData() to extract the data.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws UserEnteredInvalidArgsError   Invalid arguments ([details](@ref gpcc::cli::UserEnteredInvalidArgsError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param args
 * Arguments passed to the CLI command.\n
 * See documentation details of class @ref WriteArgsParser for expected format and syntax.
 */
WriteArgsParser::WriteArgsParser(std::string const & args)
: index(0U)
, subIndex(0U)
, dataStr()
, sizeInBit(0U)
, data()
{
  try
  {
    // find the border between first parameter (object's index and subindex) and the other parameter(s) (data to be written)
    auto const border = args.find(' ');
    if (border == std::string::npos)
      throw gpcc::cli::UserEnteredInvalidArgsError();

    // extract index and subindex from args
    StringToObjIndexAndSubindex(args.substr(0U, border), index, subIndex);

    // extract data from arguments
    dataStr = args.substr(border + 1U);
  }
  catch (gpcc::cli::UserEnteredInvalidArgsError const &) { throw; }
  catch (std::bad_alloc const &) { throw; }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
  }
}

/**
 * \brief Extracts the data from the args.
 *
 * Any previously extracted data from a previous call to this will be lost.
 *
 * \post    @ref GetDataSize() and @ref GetData() will return the extracted data.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * \throws UserEnteredInvalidArgsError   Invalid arguments passed to CTOR ([details](@ref gpcc::cli::UserEnteredInvalidArgsError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param dataType
 * Data type of the data.
 *
 * \param subIndexMaxSize
 * Maximum size of the subindex in bit.\n
 * This is don't care for the following data types: visible_string, octet_string, unicode_string
 *
 * \param endian
 * Endian for serializing the data in CANopen format.
 */
void WriteArgsParser::ExtractData(DataType const dataType,
                                  size_t const subIndexMaxSize,
                                  gpcc::Stream::IStreamWriter::Endian const endian)
{
  // Local buffer for the data. In the end, it will be moved to class' buffer in case of success.
  size_t _sizeInBit = 0U;
  std::vector<uint8_t> _data;

  switch (dataType)
  {
    case DataType::visible_string:
    {
      // check if leading and trailing '"' are present...
      if ((dataStr.size() < 2U) || (dataStr.front() != '"') || (dataStr.back() != '"'))
        throw gpcc::cli::UserEnteredInvalidArgsError("User entry for DATA is not a valid visible_string.");

      // ...and remove them
      auto const strippedDataStr = dataStr.substr(1U, dataStr.size() - 2U);

      // Determine size of CANopen encoded data.
      // Note that an empty string will result in size 1 (null-terminator)
      auto sizeInByte = strippedDataStr.size();
      if (sizeInByte == 0U)
        sizeInByte = 1U;
      _sizeInBit = sizeInByte * 8U;

      // allocate memory and convert user's input to data
      _data.resize(sizeInByte);
      gpcc::Stream::MemStreamWriter msw(_data.data(), sizeInByte, endian);
      try
      {
        StringToCANOpenEncodedData(strippedDataStr, _sizeInBit, DataType::visible_string, msw);
      }
      catch (std::bad_alloc const &) { throw; }
      catch (std::exception const &)
      {
        std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
      }

      msw.Close();

      break;
    }

    case DataType::octet_string:
    {
      auto const values = gpcc::string::Split(dataStr, ' ', true);
      if (values.empty())
        throw gpcc::cli::UserEnteredInvalidArgsError("User entry for DATA is not a valid octet_string.");

      // determine size of CANopen encoded data
      auto const sizeInByte = values.size();
      _sizeInBit = sizeInByte * 8U;

      // allocate memory and convert user's input to data
      _data.resize(sizeInByte);
      gpcc::Stream::MemStreamWriter msw(_data.data(), sizeInByte, endian);
      try
      {
        for (auto const & value: values)
          StringToCANOpenEncodedData(value, _sizeInBit, DataType::octet_string, msw);
      }
      catch (std::bad_alloc const &) { throw; }
      catch (std::exception const &)
      {
        std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
      }

      msw.Close();

      break;
    }

    case DataType::unicode_string:
    {
      auto const values = gpcc::string::Split(dataStr, ' ', true);
      if (values.empty())
        throw gpcc::cli::UserEnteredInvalidArgsError("User entry for DATA is not a valid unicode_string.");

      // determine size of CANopen encoded data
      auto sizeInByte = values.size() * 2U;
      _sizeInBit  = sizeInByte * 8U;

      // allocate memory and convert user's input to data
      _data.resize(sizeInByte);
      gpcc::Stream::MemStreamWriter msw(_data.data(), sizeInByte, endian);
      try
      {
        for (auto const & value: values)
          StringToCANOpenEncodedData(value, _sizeInBit, DataType::unicode_string, msw);
      }
      catch (std::bad_alloc const &) { throw; }
      catch (std::exception const &)
      {
        std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
      }

      msw.Close();

      break;
    }

    default:
    {
      if (dataStr.empty())
        throw gpcc::cli::UserEnteredInvalidArgsError("User entry for DATA is invalid.");

      // determine size of CANopen encoded data
      _sizeInBit = subIndexMaxSize;
      size_t sizeInByte = (_sizeInBit + 7U) / 8U;

      // allocate memory and convert user's input to data
      _data.resize(sizeInByte);
      gpcc::Stream::MemStreamWriter msw(_data.data(), sizeInByte, endian);
      try
      {
        StringToCANOpenEncodedData(dataStr, _sizeInBit, dataType, msw);
      }
      catch (std::bad_alloc const &) { throw; }
      catch (std::exception const &)
      {
        std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
      }

      msw.Close();
      break;
    }
  } // switch (dataType)

  data = std::move(_data);
  sizeInBit = _sizeInBit;
}

} // namespace internal
} // namespace cood
} // namespace gpcc
