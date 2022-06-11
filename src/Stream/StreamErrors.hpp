/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018 Daniel Jerolm

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

#ifndef SRC_GPCC_STREAM_STREAMERRORS_HPP_
#define SRC_GPCC_STREAM_STREAMERRORS_HPP_

#include <stdexcept>
#include <string>

namespace gpcc
{
namespace Stream
{

/**
 * @ingroup GPCC_STREAM
 * \brief Exception thrown by streams in case of an low-level IO error.
 *
 * There are likely nested exceptions providing details about the error.
 */
class IOError : public std::runtime_error
{
  public:
    inline IOError(char const * const pDescr): std::runtime_error(pDescr) {};
    inline IOError(std::string const & descr): std::runtime_error(descr) {};

    virtual ~IOError(void) noexcept = default;
};

/**
 * @ingroup GPCC_STREAM
 * \brief Exception thrown by streams if a write to a full stream occurs.
 */
class FullError : public std::runtime_error
{
  public:
    inline FullError(void) : std::runtime_error("") {};
    inline const char* what() const noexcept override { return "Attempt to write to a full stream."; }

    virtual ~FullError(void) noexcept = default;
};

/**
 * @ingroup GPCC_STREAM
 * \brief Exception thrown by streams if a read from an empty stream occurs.
 */
class EmptyError : public std::runtime_error
{
  public:
    inline EmptyError(void) : std::runtime_error("") {};
    inline const char* what() const noexcept override { return "Attempt to read from empty stream."; }

    virtual ~EmptyError(void) noexcept = default;
};

/**
 * @ingroup GPCC_STREAM
 * \brief Exception thrown by streams if a closed stream is accessed.
 */
class ClosedError : public std::logic_error
{
  public:
    inline ClosedError(void) : std::logic_error("") {};
    inline const char* what() const noexcept override { return "Attempt to access a closed stream."; }

    virtual ~ClosedError(void) noexcept = default;
};

/**
 * @ingroup GPCC_STREAM
 * \brief Exception thrown by streams if a stream which is in the error state is accessed.
 */
class ErrorStateError : public std::logic_error
{
  public:
    inline ErrorStateError(void) : std::logic_error("") {};
    inline const char* what() const noexcept override { return "Attempt to access a stream in error state."; }

    virtual ~ErrorStateError(void) noexcept = default;
};

/**
 * @ingroup GPCC_STREAM
 * \brief Exception thrown by [IStreamReader::EnsureAllDataConsumed(...)](@ref gpcc::Stream::IStreamReader::EnsureAllDataConsumed)
 *        if the remaining number of bits (or bytes) in the stream available for reading does not match the expectation passed to
 *        EnsureAllDataConsumed(...).
 */
class RemainingBitsError : public std::runtime_error
{
  public:
    inline RemainingBitsError(void) : std::runtime_error("Stream does not contain the expected number of remaining bits.") {};

    virtual ~RemainingBitsError(void) noexcept = default;
};

} // namespace Stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_STREAMERRORS_HPP_ */
