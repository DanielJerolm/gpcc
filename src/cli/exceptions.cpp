/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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

#include "exceptions.hpp"

namespace gpcc     {
namespace cli      {

/**
 * \brief Constructor. Creates an @ref UserEnteredInvalidArgsError exception with no details provided.
 *
 * The @ref CLI will print the what-text of this exception and the what-text of any nested exception.
 *
 * Use this if there is something generally wrong with the arguments and if the user can recognize his error by reading
 * the command's help text.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
UserEnteredInvalidArgsError::UserEnteredInvalidArgsError(void)
: std::runtime_error("User entered invalid arguments.")
, details("")
{
};

/**
 * \brief Constructor. Creates a @ref UserEnteredInvalidArgsError exception with details provided.\n
 *        The provided string with details will be copied into the exception object.
 *
 * The @ref CLI will print the provided details instead of the what-texts of this exception and potential nested
 * exceptions.
 *
 * Use this if there is something concrete wrong with the arguments entered behind the command and if it is hard to
 * figure out the error by reading the command's help text.
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
 * \param _details
 * Detailed description of the error encountered with the arguments passed to the CLI command.\n
 * Example:\n
 * "2nd parameter should be a decimal number."\n
 * The referenced string will be copied.
 */
UserEnteredInvalidArgsError::UserEnteredInvalidArgsError(std::string const & _details)
: std::runtime_error("User entered invalid arguments.")
, details(_details)
{
};

/**
 * \brief Constructor. Creates a @ref UserEnteredInvalidArgsError exception with details provided.\n
 *        The provided string with details will be moved into the exception object.
 *
 * The @ref CLI will print the provided details instead of the what-texts of this exception and potential nested
 * exceptions.
 *
 * Use this if there is something concrete wrong with the arguments entered behind the command and if it is hard to
 * figure out the error by reading the command's help text.
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
 * \param _details
 * Detailed description of the error encountered with the arguments passed to the CLI command.\n
 * Example:\n
 * "2nd parameter should be a decimal number."\n
 * The referenced string will be moved into the new exception object.
 */
UserEnteredInvalidArgsError::UserEnteredInvalidArgsError(std::string && _details)
: std::runtime_error("User entered invalid arguments.")
, details(std::move(_details))
{
};

} // namespace cli
} // namespace gpcc
