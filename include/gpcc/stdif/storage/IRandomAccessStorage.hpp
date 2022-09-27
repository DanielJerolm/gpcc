/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IRANDOMACCESSSTORAGE_HPP_202209072134
#define IRANDOMACCESSSTORAGE_HPP_202209072134

#include <cstddef>
#include <cstdint>

namespace gpcc  {
namespace stdif {

/**
 * \ingroup GPCC_STDIF_STORAGE
 * \brief Interface for random accessible storage (EEPROM devices, NVRAMs, plain RAM, ...).
 *
 * # Applications
 * This interface can be realized by derived classes that implement a driver for storage devices like EEPROMs, flash
 * devices, and any other kind of NVRAM devices. Derived classes may also offer access to just plain RAM in some kind
 * of hardware peripheral, or to a file or just a block of memory allocated from the heap.
 *
 * # Concurrent access
 * This interface is thread-safe. Read- and write-accesses are synchronous. Concurrent read- and write-accesses to the
 * same memory addresses and/or pages are properly serialized.
 *
 * The underlying implementation is allowed to execute accesses to different memory locations in parallel, if all of
 * the following is true:
 * - the accesses involve different pages or memory ranges, so that they won't interfere with each other
 * - the underlying storage device supports parallelization
 *
 * If multiple read- and/or write-accesses are issued simultaneously, then the order in which they complete is
 * undefined. If a specific order is required, then the user of this interface has to issue the accesses one-by-one.
 *
 * # Guarantees
 * Write accesses have physically completed before @ref Write() or @ref WriteAndCheck() return.
 *
 * If page boundaries are met by the user of this interface, then derived classes __may__ provide a guarantee that in
 * case of a power-fail during a write access, the storage is in a valid and defined state (either the write access has
 * completed before the power fail, or it was not started). Please refer to the documentation of the derived class.
 *
 * # Page write
 * This interface allows to access the underlying storage randomly. It accepts page-aligned read- and write-accesses
 * and unaligned random accesses. The user of this interface is therefore not required to adhere to page boundaries.
 *
 * Use @ref GetPageSize() to figure out the page size and if the underlying device is organized in pages or not.
 *
 * However, meeting page boundaries may have the following advantages:
 * - Potentially increased performance on some types of underlying storage.
 *
 * Not meeting page boundaries may have the following disadvantages:
 * - Potentially decreased performance on some types of underlying storage, especially for write accesses.
 * - In case of a power-fail during a write access, the affected pages may be corrupted, even though only a few bytes
 *   should have been written.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IRandomAccessStorage
{
  public:
    virtual size_t GetSize(void) const = 0;
    virtual size_t GetPageSize(void) const = 0;

    virtual void Read(uint32_t address, size_t n, void* pBuffer) const = 0;
    virtual void Write(uint32_t address, size_t n, void const * pBuffer) = 0;
    virtual bool WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer) = 0;

  protected:
    IRandomAccessStorage(void) = default;
    IRandomAccessStorage(IRandomAccessStorage const &) = default;
    IRandomAccessStorage(IRandomAccessStorage &&) = default;
    virtual ~IRandomAccessStorage(void) = default;

    IRandomAccessStorage& operator=(IRandomAccessStorage const &) = default;
    IRandomAccessStorage& operator=(IRandomAccessStorage &&) = default;
};

/**
 * \fn size_t IRandomAccessStorage::GetSize() const
 * \brief Retrieves the size of the storage.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Size of the storage in byte.
 */

/**
 * \fn size_t IRandomAccessStorage::GetPageSize() const
 * \brief Retrieves if the storage is organized in pages and the page size.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Page size of the storage in byte.\n
 * Zero, if the storage is not organized in pages (e.g. plain RAM).
 */

/**
 * \fn void IRandomAccessStorage::Read(uint32_t address, size_t n, void* pBuffer) const
 * \brief Reads data from the storage.
 *
 * The read operation is automatically splitted into multiple read operations in order to satisfy page boundary
 * requirements if necessary. The caller does not need to care about page boundaries. This method can be used to
 * read blocks of random size from random addresses inside the storage.
 *
 * However, depending on the specific derived class adhering to page boundaries may gain performance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Undefined/incomplete data may be written to `pBuffer`.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Undefined/incomplete data may be written to `pBuffer`.
 *
 * - - -
 *
 * \param address
 * Byte address inside the storage where to start reading.
 *
 * \param n
 * Number of bytes that shall be read.
 *
 * \param pBuffer
 * The read data is written into the referenced buffer.\n
 * _The buffer's size must be equal to or larger than `n`._
 */

/**
 * \fn void IRandomAccessStorage::Write(uint32_t address, size_t n, void const * pBuffer)
 * \brief Writes data into the storage.
 *
 * The write operation is automatically splitted into multiple write operations in order to satisfy page boundary
 * requirements if necessary. The caller does not need to care about page boundaries. This method can be used to
 * write blocks of random size to random addresses inside the storage. Each (splitted) write operation will use
 * page-write mode if multiple bytes shall be written and if page-write is supported by the underlying storage device.
 *
 * However, depending on the specific derived class it may be advantageous to adhere to page boundaries:
 * - increased performance
 * - prevent dropping special guarantees offered by some derived classes regarding power-fail during write-accesses
 * - minimize wearing of storage cells (depends on derived class and type of storage)
 *
 * For details, please refer to the documentation of [this interface](@ref gpcc::stdif::IRandomAccessStorage).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The storage affected by the write access may contain undefined data.
 * - In case of a write access not aligned to page boundaries, all pages affected by the write access may contain
 *   undefined data.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The storage affected by the write access may contain undefined data.
 * - In case of a write access not aligned to page boundaries, all pages affected by the write access may contain
 *   undefined data.
 *
 * - - -
 *
 * \param address
 * Byte address inside the storage where to start writing.
 *
 * \param n
 * Number of bytes that shall be written.
 *
 * \param pBuffer
 * Buffer containing the data that shall be written.
 */

/**
 * \fn bool IRandomAccessStorage::WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer)
 * \brief Writes data into the storage, reads the written data back, and compares the data.
 *
 * The write operation is automatically splitted into multiple write operations in order to satisfy page boundary
 * requirements if necessary. The caller does not need to care about page boundaries. This method can be used to
 * write blocks of random size to random addresses inside the storage. Each (splitted) write operation will use
 * page-write mode if multiple bytes shall be written and if page-write is supported by the underlying storage device.
 *
 * However, depending on the specific derived class it may be advantageous to adhere to page boundaries:
 * - increased performance
 * - prevent dropping special guarantees offered by some derived classes regarding power-fail during write-accesses
 * - minimize wearing of storage cells (depends on derived class and type of storage)
 *
 * For details, please refer to the documentation of [this interface](@ref gpcc::stdif::IRandomAccessStorage).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The storage affected by the write access may contain undefined data.
 * - In case of a write access not aligned to page boundaries, all pages affected by the write access may contain
 *   undefined data.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The storage affected by the write access may contain undefined data.
 * - In case of a write access not aligned to page boundaries, all pages affected by the write access may contain
 *   undefined data.
 *
 * - - -
 *
 * \param address
 * Byte address inside the storage where to start writing.
 *
 * \param n
 * Number of bytes that shall be written.
 *
 * \param pBuffer
 * Buffer containing the data that shall be written.
 *
 * \param pAuxBuffer
 * Auxiliary buffer that can be used by this method to read back the written data.\n
 * _The buffer's size must be equal to or larger than parameter "n"._\n
 * If this is `nullptr`, then this method will allocate the required memory on the heap and release it afterwards.
 *
 * \retval true  Data read back matched.
 * \retval false Data read back did not match.
 */

} // namespace stdif
} // namespace gpcc

#endif // IRANDOMACCESSSTORAGE_HPP_202209072134
