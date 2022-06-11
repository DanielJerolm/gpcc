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

#include "Backend_Recorder.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace gpcc {
namespace log  {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Backend_Recorder::Backend_Recorder(void)
: Backend()
, mutex()
, records()
{
}

/**
 * \brief Constructor. This also reserves space for records.
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
 * \param capacity
 * Capacity that shall be reserved for records.
 */
Backend_Recorder::Backend_Recorder(size_t const capacity)
: Backend()
, mutex()
, records()
{
  records.reserve(capacity);
}

/**
 * \brief []-Operator: Retrieves a specific record.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the record that shall be retrieved.
 * \return
 * Non-modifiable reference to the record specified by `index`.
 */
std::string const & Backend_Recorder::operator[] (size_t const index) const
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (index >= records.size())
    throw std::out_of_range("Backend_Recorder::operator[]");

  return records[index];
}

/**
 * \brief Discards all records.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 */
void Backend_Recorder::Clear(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  records.clear();
}

/**
 * \brief Prints all records to stdout.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Basic guarantee:
 * - not all records may be printed to stdout
 * - the last printed record may be incomplete
 *
 * - - -
 *
 * \param clear
 * Controls if the records shall be discarded after printing them to stdout:\n
 * true = discard\n
 * false = do not discard
 */
void Backend_Recorder::PrintToStdout(bool const clear)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  for (auto const & e: records)
    std::cout << e << std::endl;

  if (clear)
    records.clear();
}

/**
 * \brief Writes all records into a text file.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Basic guarantee:
 * - Not all records may have been written into the file. The partially written file will not be removed.
 *
 * __Thread-cancellation-safety:__\n
 * Basic guarantee:
 * - Not all records may have been written into the file. The partially written file will not be removed.
 *
 * - - -
 *
 * \param clear
 * Controls if the records shall be discarded after writing them into the file:\n
 * true = discard\n
 * false = do not discard
 *
 * \param pathAndName
 * Path and name of the file.\n
 * If the file is already existing, then it will be overwritten.
 */
void Backend_Recorder::WriteToFile(bool const clear, std::string const & pathAndName)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  {
    std::ofstream f(pathAndName);
    for (auto const & e: records)
    {
      f << e << std::endl;
    }
  }

  if (clear)
    records.clear();
}

/**
 * \brief Retrieves the number of records.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Number of records.
 */
size_t Backend_Recorder::GetNbOfRecords(void) const
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  return records.size();
}

/// \copydoc Backend::Process
void Backend_Recorder::Process(std::string const & msg, LogType const type)
{
  (void)type;

  gpcc::osal::MutexLocker mutexLocker(mutex);
  records.emplace_back(msg);
}

} // namespace log
} // namespace gpcc
