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

#ifndef WRITEREQUEST_HPP_202006251837
#define WRITEREQUEST_HPP_202006251837

#include "RequestBase.hpp"
#include "gpcc/src/cood/Object.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref WriteRequest (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class WriteRequestPassKey final
{
  private:
    friend class RequestBase;

    WriteRequestPassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request:\n
 *        Write request (single subindex and complete access).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class WriteRequest final : public RequestBase
{
  public:
    /// Access type.
    enum class AccessType : uint8_t
    {
      singleSubindex,           ///<Single subindex write.
      completeAccess_SI0_8bit,  ///<Complete access write, SI0 written as uint8
      completeAccess_SI0_16bit  ///<Complete access write, SI0 written as uint16
    };


    WriteRequest(void) = delete;

    WriteRequest(AccessType           const _accessType,
                 uint16_t             const _index,
                 uint8_t              const _subindex,
                 Object::attr_t       const _permissions,
                 std::vector<uint8_t> &&    _data,
                 size_t               const _maxResponseSize);

    WriteRequest(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, WriteRequestPassKey);

    WriteRequest(WriteRequest const &) = default;
    WriteRequest(WriteRequest &&) noexcept = default;

    ~WriteRequest(void) = default;

    static size_t CalcMaxDataPayload(size_t const maxRequestSize, bool const withRSI) noexcept;

    WriteRequest& operator=(WriteRequest const &) = delete;
    WriteRequest& operator=(WriteRequest &&) = delete;

    // <-- RequestBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::Stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> RequestBase

    AccessType GetAccessType(void) const noexcept;
    uint16_t GetIndex(void) const noexcept;
    uint8_t GetSubIndex(void) const noexcept;
    Object::attr_t GetPermissions(void) const noexcept;
    std::vector<uint8_t> const & GetData(void) const;

  private:
    /// Binary size of a serialized @ref WriteRequest object (excl. base class @ref RequestBase).
    static size_t const writeRequestBinarySize = 8U;


    /// Access type.
    AccessType const accessType;

    /// Index of the object that shall be written.
    uint16_t const index;

    /// Subindex that shall be written.
    uint8_t const subindex;

    /// Permissions provided by the originator of the write request.
    /** This is any combination of attr_ACCESS_X values from class @ref Object. */
    Object::attr_t const permissions;

    /// Data that shall be written.
    std::vector<uint8_t> data;


    static AccessType U8_to_AccessType(uint8_t const u8);
};

/**
 * \brief Retrieves the access type.
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
 * Access type.
 */
inline WriteRequest::AccessType WriteRequest::GetAccessType(void) const noexcept
{
  return accessType;
}

/**
 * \brief Retrieves the index of the object that shall be written.
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
 * Index of the object that shall be written.
 */
inline uint16_t WriteRequest::GetIndex(void) const noexcept
{
  return index;
}

/**
 * \brief Retrieves the subindex that shall be written.
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
 * Subindex that shall be written.
 */
inline uint8_t WriteRequest::GetSubIndex(void) const noexcept
{
  return subindex;
}

/**
 * \brief Retrieves the permissions provided by the originator of the request.
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
 * Permissions for the write access provided by the originator of the request.
 */
inline Object::attr_t WriteRequest::GetPermissions(void) const noexcept
{
  return permissions;
}

} // namespace cood
} // namespace gpcc

#endif // WRITEREQUEST_HPP_202006251837
