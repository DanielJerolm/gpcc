/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "ReadArgsParser.hpp"
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
 * See documentation details of class @ref ReadArgsParser for expected format and syntax.
 */
ReadArgsParser::ReadArgsParser(std::string const & args)
: index(0U)
, subIndex(0U)
{
  if (args.empty())
    throw gpcc::cli::UserEnteredInvalidArgsError();

  try
  {
    StringToObjIndexAndSubindex(args, index, subIndex);
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
