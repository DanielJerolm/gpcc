/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef BACKEND_CLI_HPP_201701052126
#define BACKEND_CLI_HPP_201701052126

#include <gpcc/log/backends/Backend.hpp>

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
