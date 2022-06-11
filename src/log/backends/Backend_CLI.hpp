/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019 Daniel Jerolm

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

#ifndef BACKEND_CLI_HPP_201701052126
#define BACKEND_CLI_HPP_201701052126

#include "Backend.hpp"

namespace gpcc {

namespace cli
{
  class CLI;
}

namespace log {

/**
 * \ingroup GPCC_LOG_BACKENDS
 * \brief Log facility back-end which prints log messages to a @ref gpcc::cli::CLI instance.
 *
 * - - -
 *
 *  __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class Backend_CLI final: public Backend
{
  public:
    explicit Backend_CLI(gpcc::cli::CLI & _cli);
    Backend_CLI(Backend_CLI const &) = delete;
    Backend_CLI(Backend_CLI &&) = delete;
    ~Backend_CLI(void) = default;

    Backend_CLI& operator=(Backend_CLI const &) = delete;
    Backend_CLI& operator=(Backend_CLI &&) = delete;

    // --> Backend
    void Process(std::string const & msg, LogType const type) override;
    // <--

  private:
    /// CLI instance to which log messages will be printed.
    gpcc::cli::CLI & cli;
};

} // namespace log
} // namespace gpcc

#endif // BACKEND_CLI_HPP_201701052126
