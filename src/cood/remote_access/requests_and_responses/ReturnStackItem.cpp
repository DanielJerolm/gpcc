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

#include "ReturnStackItem.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"

namespace gpcc {
namespace cood {

size_t const ReturnStackItem::binarySize;

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _id
 * ID of the unit that generated this @ref ReturnStackItem instance.\n
 * This is intended to verify the origin of a @ref ReturnStackItem when popping them from a remote access
 * response object.
 *
 * \param _info
 * Routing info.\n
 * The meaning of the bits depends on the class that created the @ref ReturnStackItem object.
 */
ReturnStackItem::ReturnStackItem(uint32_t const _id, uint32_t const _info) noexcept
: id(_id)
, info(_info)
{
}

/**
 * \brief Constructor. Creates a @ref ReturnStackItem object from data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref ReturnStackItem object.
 *
 * This is the counterpart of @ref ToBinary().
 *
 * - - -
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from 'sr' and 'sr' is not recovered.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from 'sr' and 'sr' is not recovered.
 *
 * - - -
 *
 * \param sr
 * Stream from which the data shall be read.
 */
ReturnStackItem::ReturnStackItem(gpcc::Stream::IStreamReader & sr)
: id(sr.Read_uint32())
, info(sr.Read_uint32())
{
}

/**
 * \brief Writes a binary representation of the object into a stream, which can be deserialized via
 *        @ref ReturnStackItem(gpcc::Stream::IStreamReader & sr).
 *
 * The counterpart of this is @ref ReturnStackItem(gpcc::Stream::IStreamReader & sr).
 *
 * @ref binarySize number of bytes will be written.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of undefined data may have been written to `sw`. `sw` is not recovered.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of undefined data may have been written to `sw`. `sw` is not recovered.
 *
 * - - -
 *
 * \param sw
 * The binary data is written into the referenced stream.
 */
void ReturnStackItem::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  sw.Write_uint32(id);
  sw.Write_uint32(info);
}

} // namespace cood
} // namespace gpcc
