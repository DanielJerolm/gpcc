/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "CAReadArgsParser.hpp"
#include "gpcc/src/cli/exceptions.hpp"
#include "gpcc/src/cood/cli/string_conversion.hpp"
#include "gpcc/src/string/tools.hpp"
#include <stdexcept>
#include <exception>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \brief Constructor.
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
 * See documentation details of class @ref CAReadArgsParser for expected format and syntax.
 */
CAReadArgsParser::CAReadArgsParser(std::string const & args)
: index(0U)
, verbose(false)
{
  try
  {
    auto splitted = gpcc::string::Split(args, ' ', false);

    if ((splitted.size() != 1U) && (splitted.size() != 2U))
      throw gpcc::cli::UserEnteredInvalidArgsError();

    index = StringToObjIndex(splitted[0]);

    if (splitted.size() == 2U)
    {
      if (splitted[1] == "v")
        verbose = true;
      else
        throw gpcc::cli::UserEnteredInvalidArgsError();
    }
  }
  catch (gpcc::cli::UserEnteredInvalidArgsError const &) { throw; }
  catch (std::bad_alloc const &) { throw; }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::cli::UserEnteredInvalidArgsError());
  }
}

} // namespace internal
} // namespace cood
} // namespace gpcc
