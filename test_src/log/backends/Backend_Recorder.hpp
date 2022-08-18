/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef BACKEND_RECORDER_HPP_201702152052
#define BACKEND_RECORDER_HPP_201702152052

#include "gpcc/src/log/backends/Backend.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include <vector>
#include <cstddef>

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG_BACKENDS
 * \brief Log facility back-end for recording log messages in a unit test environment.
 *
 * This backend is intended to be used in unit-tests. Log messages are recorded and can be printed to stdout and/or into
 * a text file.
 *
 * Use @ref PrintToStdout() to print all recorded messages to stdout.\n
 * Use @ref WriteToFile() to write all recorded messages into a text file.\n
 * Use @ref Clear() to clear all records.
 *
 * - - -
 *
 *  __Thread safety:__\n
 * Thread-safe.
 */
class Backend_Recorder final: public Backend
{
  public:
    Backend_Recorder(void);
    Backend_Recorder(size_t const capacity);
    Backend_Recorder(Backend_Recorder const &) = delete;
    Backend_Recorder(Backend_Recorder &&) = delete;
    ~Backend_Recorder(void) = default;

    Backend_Recorder& operator=(Backend_Recorder const &) = delete;
    Backend_Recorder& operator=(Backend_Recorder &&) = delete;

    std::string const & operator[] (size_t const index) const;

    void Clear(void);
    void PrintToStdout(bool const clear);
    void WriteToFile(bool const clear, std::string const & pathAndName);
    size_t GetNbOfRecords(void) const;

    // --> Backend
    void Process(std::string const & msg, LogType const type) override;
    // <--

  private:
    /// Mutex used to make the API thread-safe.
    gpcc::osal::Mutex mutable mutex;

    /// Recorded messages.
    /** @ref mutex is required. */
    std::vector<std::string> records;
};

} // namespace log
} // namespace gpcc

#endif // BACKEND_RECORDER_HPP_201702152052
