/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018 Daniel Jerolm

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

#ifndef EXCEPTIONS_HPP_201808232142
#define EXCEPTIONS_HPP_201808232142

#include <stdexcept>
#include <string>

namespace gpcc               {
namespace ResourceManagement {
namespace Objects            {

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

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc

#endif // EXCEPTIONS_HPP_201808232142
