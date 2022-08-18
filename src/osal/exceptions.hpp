/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef EXCEPTIONS_HPP_201807011348
#define EXCEPTIONS_HPP_201807011348

#include <stdexcept>
#include <string>

namespace gpcc  {
namespace osal  {

/**
 * \ingroup GPCC_OSAL_EXCEPTIONS
 * \brief Exception thrown if a timeout occurs.
 */
class TimeoutError : public std::runtime_error
{
  public:
    inline TimeoutError(char const * const what) : std::runtime_error(what) {};
    inline TimeoutError(std::string const & what) : std::runtime_error(what) {};
    virtual ~TimeoutError(void) noexcept = default;
};

} // namespace osal
} // namespace gpcc

#endif // EXCEPTIONS_HPP_201807011348
