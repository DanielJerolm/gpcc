/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef OBJECTINFOREQUEST_HPP_202102131446

#include "RequestBase.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref ObjectInfoRequest (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectInfoRequestPassKey final
{
  private:
    friend class RequestBase;

    ObjectInfoRequestPassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request:\n
 *        Query object's meta data.
 *
 * This request queries the object's meta data:
 * - Object code
 * - Object's data type
 * - Maximum number of subindices
 * - Object's name (optional)
 *
 * The query also includes the meta data of a range of subindices:
 * - empty-status
 * - Data type
 * - Attributes
 * - Maximum size
 * - Name (optional)
 * - Application specific meta data (optional)
 *
 * The response (@ref ObjectInfoResponse) will always contain information about the object plus information about as
 * many subindices as possible from the specified range of subindices. Information about the complete specified range
 * will not be provided if:\n
 * a) The range exceeds the maximum number of subindices of the object.\n
 * b) The payload of the response is completely consumed.
 *
 * In case of b) the queried information is incomplete and another request should be issued which continues the query at
 * the next subindex that did not fit into the response. Later the response of the second request can be added to
 * the response of the first request. See @ref ObjectInfoResponse for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ObjectInfoRequest final : public RequestBase
{
  public:
    ObjectInfoRequest(void) = delete;
    ObjectInfoRequest(uint16_t const _index,
                      uint8_t  const _firstSubIndex,
                      uint8_t  const _lastSubIndex,
                      bool     const _inclusiveNames,
                      bool     const _inclusiveAppSpecificMetaData,
                      size_t   const _maxResponseSize);

    ObjectInfoRequest(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectInfoRequestPassKey);

    ObjectInfoRequest(ObjectInfoRequest const &) = default;
    ObjectInfoRequest(ObjectInfoRequest &&) = default;

    ~ObjectInfoRequest(void) = default;

    ObjectInfoRequest& operator=(ObjectInfoRequest const &) = delete;
    ObjectInfoRequest& operator=(ObjectInfoRequest &&) = delete;

    // <-- RequestBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::Stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> RequestBase

    uint16_t GetIndex(void) const noexcept;
    uint8_t GetFirstSubIndex(void) const noexcept;
    uint8_t GetLastSubIndex(void) const noexcept;
    bool IsInclusiveNames(void) const noexcept;
    bool IsInclusiveAppSpecificMetaData(void) const noexcept;

  private:
    /// Binary size of a serialized @ref ObjectInfoRequest object (excl. base class @ref RequestBase).
    static size_t const objectInfoRequestBinarySize = 5U;


    /// Index of the object whose meta data shall be queried.
    uint16_t const index;

    /// First subindex whose meta data shall be queried.
    uint8_t const firstSubIndex;

    /// Last subindex whose meta data shall be queried.
    uint8_t const lastSubIndex;

    /// Flag indicating if object's and subindices' names shall be included in the response.
    bool const inclusiveNames;

    /// Flag indicating if application specific meta data of the subindices shall be included in the response.
    bool const inclusiveAppSpecificMetaData;
};

/**
 * \brief Retrieves the index of the object whose meta data shall be queried.
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
 * Index of the object whose meta data shall be queried.
 */
inline uint16_t ObjectInfoRequest::GetIndex(void) const noexcept
{
  return index;
}

/**
 * \brief Retrieves the number of the first subindex whose meta data shall be queried.
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
 * Number of the first subindex whose meta data shall be queried.
 */
inline uint8_t ObjectInfoRequest::GetFirstSubIndex(void) const noexcept
{
  return firstSubIndex;
}

/**
 * \brief Retrieves the number of the last subindex whose meta data shall be queried.
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
 * Number of the last subindex whose meta data shall be queried.
 */
inline uint8_t ObjectInfoRequest::GetLastSubIndex(void) const noexcept
{
  return lastSubIndex;
}

/**
 * \brief Retrieves if the names of the object and subindices shall be included in the response.
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
 * \retval true   The names of the object and the subindices shall be included in the response.
 * \retval false  No object/subindex names shall be included in the response.
 */
inline bool ObjectInfoRequest::IsInclusiveNames(void) const noexcept
{
  return inclusiveNames;
}

/**
 * \brief Retrieves if the application specific meta data of the subindices shall be included in the response.
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
 * \retval true   The application specific meta data (if any) of the subindices shall be included in the response.
 * \retval false  No application specific meta data shall be included in the response.
 */
inline bool ObjectInfoRequest::IsInclusiveAppSpecificMetaData(void) const noexcept
{
  return inclusiveAppSpecificMetaData;
}

} // namespace cood
} // namespace gpcc

#endif // OBJECTINFOREQUEST_HPP_202102131446
