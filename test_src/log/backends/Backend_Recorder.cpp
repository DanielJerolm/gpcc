/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
