/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
