/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#include "RAMBlock.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include <stdexcept>
#include <cstring>

namespace gpcc      {
namespace container {

/**
 * \brief Constructor. All bytes of the RAMBlock's storage will be initialized with zero.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param size
 * Desired size (in byte) for the storage.
 */
RAMBlock::RAMBlock(size_t const size)
: IRandomAccessStorage()
, apiMutex()
, storage(size)
, dirty(false)
{
}

/**
 * \brief Constructor. All bytes of the RAMBlock's storage will be initialized with a give value.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param size
 * Desired size (in byte) for the storage.
 * \param v
 * Value used to initialize each byte of the RAMBlock's storage.
 */
RAMBlock::RAMBlock(size_t const size, uint8_t const v)
: IRandomAccessStorage()
, apiMutex()
, storage(size, v)
, dirty(false)
{
}

/**
 * \brief Constructor. The RAMBlock's storage will be initialized with data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) interface.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Data may have been read from `sr` and the state of the stream reader will not be recovered.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Data may have been read from `sr` and the state of the stream reader will not be recovered.
 *
 * - - -
 *
 * \param size
 * Desired size (in byte) for the RAMBlock's storage.
 * \param sr
 * `size` bytes will be read from this and used to initialize the RAMBlock's storage.
 */
RAMBlock::RAMBlock(size_t const size, gpcc::Stream::IStreamReader& sr)
: IRandomAccessStorage()
, apiMutex()
, storage(size)
, dirty(false)
{
  sr.Read_uint8(storage.data(), size);
}

/**
 * \brief Constructor. The RAMBlock's storage will be initialized with data copied from a `std::vector<uint8_t>`.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param data
 * Unmodifiable reference to an ´std::vector<uint8_t>` used to initialize the new @ref RAMBlock instance.\n
 * The size of the RAMBlock's storage will be initialized with the size of the given vector.\n
 * The content of the RAMBlock's storage will be initialized with a copy of the vector's content.
 */
RAMBlock::RAMBlock(std::vector<uint8_t> const & data)
: IRandomAccessStorage()
, apiMutex()
, storage(data)
, dirty(false)
{
}

/**
 * \brief Constructor. The RAMBlock's storage will be initialized with data moved from a `std::vector<uint8_t>` into the
 *        new @ref RAMBlock instance.
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
 * \param data
 * The content of the referenced ´std::vector<uint8_t>` will be moved into the storage of the new @ref RAMBlock instance.\n
 * The RAMBlock's storage will have the same size as the referenced std::vector.\n
 * `data` will be left in a valid, but undefined state.
 */
RAMBlock::RAMBlock(std::vector<uint8_t> && data)
: IRandomAccessStorage()
, apiMutex()
, storage(std::move(data))
, dirty(false)
{
}

/**
 * \brief Copy constructor. Creates a copy of an existing @ref RAMBlock instance.
 *
 * Note:\n
 * The dirty flag will also be copied!
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * The new @ref RAMBlock instance will be a deep copy of the existing @ref RAMBlock instance referenced by this.\n
 * The referenced @ref RAMBlock instance will be properly locked during the copy procedure to provide thread-safe behavior.
 */
RAMBlock::RAMBlock(RAMBlock const & other)
: IRandomAccessStorage()
, apiMutex()
, storage()
, dirty(false)
{
  gpcc::osal::MutexLocker otherApiMutexLocker(other.apiMutex);
  storage = other.storage;
  dirty   = other.dirty;
}

/**
 * \brief Move constructor. Creates a new @ref RAMBlock instance from an existing one using move-semantics.
 *
 * Note:\n
 * The dirty flag will be copied!
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
 * \param other
 * The new @ref RAMBlock instance will be move-constructed from the existing @ref RAMBlock instance referenced by this.\n
 * The referenced @ref RAMBlock instance will be properly locked during the move procedure to provide thread-safe behavior.
 */
RAMBlock::RAMBlock(RAMBlock && other)
: IRandomAccessStorage()
, apiMutex()
, storage()
, dirty()
{
  gpcc::osal::MutexLocker otherApiMutexLocker(other.apiMutex);
  storage = std::move(other.storage);
  dirty   = other.dirty;
}

/**
 * \brief Copy assignment operator. Copy assigns the content (size & data) of another @ref RAMBlock instance to this instance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.\n
 *                          This type of error cannot occur, if the size of this @ref RAMBlock instance is equal to or larger
 *                          than the size of the other RAMBlock instance (`rhv`) before the copy assignment operation.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * @ref RAMBlock instance, whose content shall be copy-assigned to this @ref RAMBlock instance.\n
 * After copy assignment,
 * - this RAMBlock will have the same size as `rhv`.
 * - this RAMBlock will contain the same data as `rhv`.
 *
 * Note:\n
 * The dirty flag will also be copied!
 *
 * \return
 * Reference to self.
 */
RAMBlock& RAMBlock::operator=(RAMBlock const & rhv)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  gpcc::osal::MutexLocker otherApiMutexLocker(rhv.apiMutex);

  storage = rhv.storage;
  dirty   = rhv.dirty;

  return *this;
}

/**
 * \brief Move assignment operator. Move assigns the content (size & data) of another @ref RAMBlock instance to this instance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * This involves no heap-allocation and thus will never throw `std::bad_alloc`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * @ref RAMBlock instance, whose content shall be move assigned to this @ref RAMBlock instance.\n
 * After move assignment,
 * - this RAMBlock will have the same size as `rhv`.
 * - this RAMBlock will contain the same data as `rhv`.
 * - `rhv` will be left in a valid but undefined state.
 *
 * Note:\n
 * The dirty flag will be copied!
 *
 * \return
 * Reference to self.
 */
RAMBlock& RAMBlock::operator=(RAMBlock && rhv)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  gpcc::osal::MutexLocker otherApiMutexLocker(rhv.apiMutex);

  storage = std::move(rhv.storage);
  dirty   = rhv.dirty;

  return *this;
}

/**
 * \brief Copy assigns the content (size & data) of a `std::vector<uint8_t>` to the RAMBlock.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.\n
 *                          This type of error cannot occur, if the size of this @ref RAMBlock instance is equal to or larger
 *                          than the size of the vector referenced by `rhv`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Vector whose content shall be copy assigned to this @ref RAMBlock instance.\n
 * After copy assignment,
 * - this RAMBlock will have the same size as `rhv`.
 * - this RAMBlock will contain the same data as `rhv`.
 *
 * Note:\n
 * The dirty flag will be cleared.
 *
 * \return
 * Reference to self.
 */
RAMBlock& RAMBlock::operator=(std::vector<uint8_t> const & rhv)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  storage = rhv;
  dirty = false;

  return *this;
}

/**
 * \brief Move assigns the content (size & data) of a `std::vector<uint8_t>` to the RAMBlock.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * This involves no heap-allocation and thus will never throw `std::bad_alloc`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Vector whose content shall be move assigned to this @ref RAMBlock instance.\n
 * After move-assignment,
 * - this RAMBlock will have the same size as `rhv`.
 * - this RAMBlock will contain the same data as `rhv`.
 * - `rhv` will be left in a valid but undefined state.
 *
 * Note:\n
 * The dirty flag will be cleared.
 *
 * \return
 * Reference to self.
 */
RAMBlock& RAMBlock::operator=(std::vector<uint8_t> && rhv)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  storage = std::move(rhv);
  dirty = false;

  return *this;
}

/**
 * \brief Retrieves the dirty-state of the RAMBlock instance.
 *
 * The RAMBlock's dirty flag will be set on any write to the RAMBlock through the
 * [IRandomAccessStorage](@ref gpcc::StdIf::IRandomAccessStorage) interface.
 *
 * The dirty-flag can be cleared using any of the following methods:
 * - @ref ClearDirtyFlag()
 * - @ref GetDataAndClearDirtyFlag()
 * - @ref WriteToStreamAndClearDirtyFlag()
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * State of the dirty-flag.
 */
bool RAMBlock::IsDirty(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return dirty;
}

/**
 * \brief Sets the dirty-flag.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void RAMBlock::SetDirtyFlag(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  dirty = true;
}

/**
 * \brief Clears the dirty-flag.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void RAMBlock::ClearDirtyFlag(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  dirty = false;
}

/**
 * \brief Retrieves a copy of the RAMBlock's storage and clears the RAMBlock's dirty flag.
 *
 * Both operations are carried out atomically.
 *
 * \post   The RAMBlock's dirty flag will be cleared.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * A copy of the RAMBlock's storage.
 */
std::vector<uint8_t> RAMBlock::GetDataAndClearDirtyFlag(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  std::vector<uint8_t>copyOfStorage(storage);
  dirty = false;
  return copyOfStorage;
}

/**
 * \brief Writes the content of the RAMBlock's storage into a [IStreamWriter](@ref gpcc::Stream::IStreamWriter) and
 *        clears the RAMBlock's dirty flag.
 *
 * Both operations are carried out atomically.
 *
 * \post   The RAMBlock's dirty flag will be cleared if writing to the IStreamWriter has succeeded without any error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Data may have been written to `sw`. The state of `sw` will not be recovered.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Data may have been written to `sw`. The state of `sw` will not be recovered.
 *
 * - - -
 *
 * \param sw
 * The content of the RAMBlock's storage will be written into this.\n
 * The storage behind the [IStreamWriter](@ref gpcc::Stream::IStreamWriter) must have a capacity equal to larger than
 * the size of the RAMBlock's storage.
 */
void RAMBlock::WriteToStreamAndClearDirtyFlag(gpcc::Stream::IStreamWriter& sw)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  sw.Write_uint8(storage.data(), storage.size());
  dirty = false;
}

// <-- gpcc::StdIf::IRandomAccessStorage
/// \copydoc gpcc::StdIf::IRandomAccessStorage::GetSize
size_t RAMBlock::GetSize(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return storage.size();
}

/// \copydoc gpcc::StdIf::IRandomAccessStorage::GetPageSize
size_t RAMBlock::GetPageSize(void) const
{
  return 0;
}

/// \copydoc gpcc::StdIf::IRandomAccessStorage::Read
void RAMBlock::Read(uint32_t address, size_t n, void* pBuffer) const
{
  if (pBuffer == nullptr)
     throw std::invalid_argument("RAMBlock::Read: !pBuffer");

  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  CheckBounds(address, n);
  if (n != 0)
    memcpy(pBuffer, storage.data() + address, n);
}

/// \copydoc gpcc::StdIf::IRandomAccessStorage::Write
void RAMBlock::Write(uint32_t address, size_t n, void const * pBuffer)
{
  if (pBuffer == nullptr)
    throw std::invalid_argument("RAMBlock::Write: !pBuffer");

  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  CheckBounds(address, n);
  if (n != 0)
  {
    memcpy(storage.data() + address, pBuffer, n);
    dirty = true;
  }
}

/// \copydoc gpcc::StdIf::IRandomAccessStorage::WriteAndCheck
bool RAMBlock::WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer)
{
  (void)pAuxBuffer;
  Write(address, n, pBuffer);
  return true;
}
// --> gpcc::StdIf::IRandomAccessStorage

/**
 * \brief Checks if a memory range specified by a given address and size is completely inside the memory range
 *        of the RAMBlock's storage.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref apiMutex must be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::logic_error   Specified memory range is out of bounds of the RAMBlock's storage.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param address
 * Start address.
 * \param n
 * Size in byte.
 */
void RAMBlock::CheckBounds(uint32_t const address, size_t const n) const
{
  if ((n > storage.size()) || (address > storage.size() - n) || (address >= storage.size()))
    throw std::logic_error("RAMBlock::CheckBounds: Address and/or number of bytes exceeds size of storage");
}

} // namespace container
} // namespace gpcc
