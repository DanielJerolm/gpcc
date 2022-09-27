/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef EXCEPTIONS_HPP_201808232142
#define EXCEPTIONS_HPP_201808232142

#include <stdexcept>
#include <string>

namespace gpcc                {
namespace resource_management {
namespace objects             {

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_OBJECTS_EXCEPTIONS
 * \brief Exception thrown if a resource shall be unlocked which is not locked.
 *
 * Potential reasons:
 * - The resource has never been locked
 * - The resource has already been unlocked
 * - The type of lock does not match. Example: The resource is write-locked, but there is an attempt to release a read-lock
 */
class NotLockedError : public std::logic_error
{
  public:
    inline NotLockedError(void) : std::logic_error("Resource not locked, or type of lock does not match") {};
    virtual ~NotLockedError(void) noexcept = default;
};

} // namespace objects
} // namespace resource_management
} // namespace gpcc

#endif // EXCEPTIONS_HPP_201808232142
