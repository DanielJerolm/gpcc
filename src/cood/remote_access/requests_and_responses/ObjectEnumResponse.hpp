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

#ifndef OBJECTENUMRESPONSE_HPP_202103042132
#define OBJECTENUMRESPONSE_HPP_202103042132

#include "ResponseBase.hpp"
#include "gpcc/src/cood/sdo_abort_codes.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref ObjectEnumResponse (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectEnumResponsePassKey final
{
  private:
    friend class ResponseBase;

    ObjectEnumResponsePassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request response:\n
 *        Response for object enumeration request.
 *
 * # Usage (for clients)
 * ## Extract result of enum request
 * 1. Use @ref GetResult() to check for any error.
 * 2. Use @ref IsComplete() to check if defragmentation is required. See chapter "Fragmentation" below for details.
 * 3. If required: Perform fragmented transfer.
 * 4. Use @ref GetIndices() to retrieve a reference to the container with the indices of the enumerated objects.
 *
 * ## Fragmentation
 * This type of response supports fragmentation:\n
 * The client shall issue a @ref ObjectEnumRequest which queries the desired range of indices. When the response is
 * received, the client shall use @ref GetResult() to query if the response is OK. If it is OK, then the client shall
 * use @ref IsComplete() to figure out if the response contains all indices, or not. If the response is incomplete,
 * then @ref IsComplete() will also tell at which index the enumeration shall be continued. The client shall issue
 * a second @ref ObjectEnumRequest which starts enumeration at that index and which extends to the desired last index
 * where enumeration shall stop.
 *
 * After reception of the response of the second @ref ObjectEnumRequest, @ref GetResult() shall be used to examine
 * the status of the second response. If the status is OK, then @ref AddFragment() shall be used to merge the
 * __second__ response into the __first__ response. After the merge, the client shall use @ref IsComplete() on the
 * __first__ response to determine if the __first__ response is complete now and -if it is not- at which index
 * enumeration shall continue.
 *
 * The client shall repeat issuing requests and merging responses into the __very first__ response until
 * @ref IsComplete() invoked on the __very first__ response indicates that all objects in the desired range have been
 * enumerated. The defragmented response can then be read from the __very first__ response.
 *
 * # Usage (for server)
 * ## Happy path
 * 1. Query maximum number of indices that could be attached to the reponse via @ref CalcMaxNbOfIndices().
 * 2. Create a @ref ObjectEnumResponse instance via @ref ObjectEnumResponse(SDOAbortCode const _result) with error
 *    status (e.g. @ref SDOAbortCode::GeneralError).
 * 3. Attach list of indices via @ref SetData(). At the same time the error status will be cleared to
 *    @ref SDOAbortCode::OK.
 *
 * ## Error scenario
 * Steps one and two are equal to the happy path scenario.
 *
 * 1. Query maximum number of indices that could be attached to the reponse via @ref CalcMaxNbOfIndices().
 * 2. Create a @ref ObjectEnumResponse instance via @ref ObjectEnumResponse(SDOAbortCode const _result) with error
 *    status (e.g. @ref SDOAbortCode::GeneralError).
 * 3. If required, then the error status may be updated to a different error status via @ref SetError().
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ObjectEnumResponse final : public ResponseBase
{
  public:
    ObjectEnumResponse(void) = delete;

    explicit ObjectEnumResponse(SDOAbortCode const _result);

    ObjectEnumResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectEnumResponsePassKey);

    ObjectEnumResponse(ObjectEnumResponse const &) = default;
    ObjectEnumResponse(ObjectEnumResponse &&) = default;

    ~ObjectEnumResponse(void) = default;

    static size_t CalcMaxNbOfIndices(size_t const maxResponseSize, size_t const returnStackSize) noexcept;

    ObjectEnumResponse& operator=(ObjectEnumResponse const &) = delete;
    ObjectEnumResponse& operator=(ObjectEnumResponse &&) = delete;

    // <-- ResponseBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::Stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> ResponseBase

    // for server
    void SetError(SDOAbortCode const _result);
    void SetData(std::vector<uint16_t> && _indices, bool const _complete);

    // for originator of enum request
    SDOAbortCode GetResult(void) const noexcept;
    bool IsComplete(uint16_t * const pNextIndex) const;
    void AddFragment(ObjectEnumResponse const & fragment);
    std::vector<uint16_t> const & GetIndices(void) const;

  private:
    /// Binary size of a serialized @ref ObjectEnumResponse object with @ref SDOAbortCode::OK (excl. base class
    /// @ref ResponseBase and without any data).
    static size_t const objectEnumResponseBinarySize = 7U;

    /// Maximum number of indices that can be encapsulated in this response object.
    static size_t const maxNbOfIndices = 65536UL;


    /// Result of the enum operation.
    SDOAbortCode result;

    /// Flag indicating if enumeration is complete.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK */
    bool complete;

    /// Indices of enumerated objects.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK */
    std::vector<uint16_t> indices;
};

} // namespace cood
} // namespace gpcc

#endif // OBJECTENUMRESPONSE_HPP_202103042132
