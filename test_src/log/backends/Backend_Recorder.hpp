/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2022 Daniel Jerolm

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
