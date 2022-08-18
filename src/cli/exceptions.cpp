/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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
