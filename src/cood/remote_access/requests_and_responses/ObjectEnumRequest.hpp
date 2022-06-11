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

#ifndef OBJECTENUMREQUEST_HPP_202103022210

#include "RequestBase.hpp"
#include "gpcc/src/cood/Object.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref ObjectEnumRequest (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectEnumRequestPassKey final
{
  private:
    friend class RequestBase;

    ObjectEnumRequestPassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request:\n
 *        Enumerate objects.
 *
 * # Purpose
 * This request enumerates object dictionary objects. Objects can be enumerated from all over the object dictionary or
 * from within a given range of indices only. Further a filter can be applied to enumerate only objects with certain
 * attributes set.
 *
 * The response (@ref ObjectEnumResponse) will contain the following information:
 * - Index of each enumerated object, __sorted by index value__ in ascending order.
 * - Information if all objects have been enumerated. If not, then the response also contains the index where
 *   enumeration shall continue and a fragmented transfer must be executed (see next chapter).
 *
 * # Fragmentation
 * If the capacity of the response is not sufficient to include the indices of all objects in the response, then the
 * response will be incomplete. In this case a second request shall be issued which continues the enumeration at the
 * index indicated by the first response. The response of the second request shall then be merged into the response of
 * the first. There might even be the need for a third or even n-th enumeration request.
 *
 * For details about fragmentation please refer to class @ref ObjectEnumResponse.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ObjectEnumRequest final : public RequestBase
{
  public:
    ObjectEnumRequest(void) = delete;
    ObjectEnumRequest(uint16_t                   const _startIndex,
                      uint16_t                   const _lastIndex,
                      gpcc::cood::Object::attr_t const _attrFilter,
                      size_t                     const _maxResponseSize);

    ObjectEnumRequest(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectEnumRequestPassKey);

    ObjectEnumRequest(ObjectEnumRequest const &) = default;
    ObjectEnumRequest(ObjectEnumRequest &&) = default;

    ~ObjectEnumRequest(void) = default;

    ObjectEnumRequest& operator=(ObjectEnumRequest const &) = delete;
    ObjectEnumRequest& operator=(ObjectEnumRequest &&) = delete;

    // <-- RequestBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::Stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> RequestBase

    uint16_t GetStartIndex(void) const noexcept;
    uint16_t GetLastIndex(void) const noexcept;
    gpcc::cood::Object::attr_t GetAttributeFilter(void) const noexcept;

  private:
    /// Binary size of a serialized @ref ObjectEnumRequest object (excl. base class @ref RequestBase).
    static size_t const objectEnumRequestBinarySize = 6U;


    /// Index where enumeration shall start.
    /** Objects located at indices less than this will not be enumerated. */
    uint16_t const startIndex;

    /// Index where enumeration shall stop.
    /** Objects located at indices larger than this will not be enumerated. */
    uint16_t const lastIndex;

    /// Attribute-filter for enumeration.
    /** Only objects with at least one matching attribute bit will be enumerated. */
    gpcc::cood::Object::attr_t attrFilter;
};

/**
 * \brief Retrieves the index where enumeration shall start.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Index where enumeration shall start.\n
 * Objects located at indices less than this will not be enumerated.
 */
inline uint16_t ObjectEnumRequest::GetStartIndex(void) const noexcept
{
  return startIndex;
}

/**
 * \brief Retrieves the index where enumeration shall stop.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Index where enumeration shall stop.\n
 * Objects located at indices larger than this will not be enumerated.
 */
inline uint16_t ObjectEnumRequest::GetLastIndex(void) const noexcept
{
  return lastIndex;
}

/**
 * \brief Retrieves the attribute filter that shall be applied when enumerating objects.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Attribute filter of object enumeration.\n
 * Only objects that have at least one matching attribute bit will be enumerated.
 */
inline gpcc::cood::Object::attr_t ObjectEnumRequest::GetAttributeFilter(void) const noexcept
{
  return attrFilter;
}

} // namespace cood
} // namespace gpcc

#endif // OBJECTENUMREQUEST_HPP_202103022210
