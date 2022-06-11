/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#ifndef SRC_GPCC_STDIF_IRANDOMACCESSSTORAGE_HPP_
#define SRC_GPCC_STDIF_IRANDOMACCESSSTORAGE_HPP_

#include <cstdint>
#include <cstddef>

namespace gpcc
{
namespace StdIf
{

/**
 * \brief Interface for random accessible storage (EEPROM devices, NVRAMs, plain RAM, ...).
 *
 * _Implicit capabilities (protected): default-construction, copy-construction, copy-assignment, move-construction, move-assignment_
 *
 * This interface can be implemented by derived classes providing access to storage like RAM, EEPROM and similar
 * devices.
 *
 * All accesses are thread-safe. Concurrent read- and write-accesses to the same memory addresses are properly
 * serialized. Write accesses are physically completed before @ref Write() or @ref WriteAndCheck() return.
 * @ref Write() and @ref WriteAndCheck() always use page-write if multiple bytes shall be written and if the
 * underlying storage device supports page-write.
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
 * \fn virtual size_t IRandomAccessStorage::GetSize() = 0
 * \brief Retrieves the size of the storage.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Size of the storage in byte.
 */

/**
 * \fn virtual size_t IRandomAccessStorage::GetPageSize() = 0
 * \brief Retrieves the page size of the storage.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Page size of the storage in byte.\n
 * For storage that is not organized in pages (i.e. plain RAM), zero is returned.
 */

/**
 * \fn virtual void IRandomAccessStorage::Read(uint32_t address, size_t n, void* pBuffer) const = 0
 * \brief Reads data from the storage.
 *
 * The read operation is automatically splitted into multiple read operations in order to satisfy page boundary
 * requirements if necessary. The caller does not need to care about page boundaries. This method can be used to
 * read blocks of random size from random addresses inside the storage.
 *
 * However, depending on the particular derived class reading not across page boundaries may gain
 * performance.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * The memory referenced by `pBuffer` may be left with undefined content.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but the memory referenced by `pBuffer` may be left with undefined content.
 *
 * ---
 *
 * \param address
 * Byte address inside the storage where to start reading.
 * \param n
 * Number of bytes that shall be read.
 * \param pBuffer
 * The read data is written into the referenced buffer.\n
 * _The buffer's size must be equal to or larger than parameter "n"._
 */

/**
 * \fn virtual void IRandomAccessStorage::Write(uint32_t address, size_t n, void const * pBuffer) = 0
 * \brief Writes data into the storage.
 *
 * The write operation is automatically splitted into multiple write operations in order to satisfy page boundary
 * requirements if necessary. The caller does not need to care about page boundaries. This method can be used to
 * write blocks of random size to random addresses inside the storage. Each (splitted) write operation will use
 * page-write mode if multiple bytes shall be written and if page-write is supported by the underlying storage device.
 *
 * However, depending on the particular derived class it may be useful to not write across page boundaries,
 * though it is supported by this method. Reasons for doing so:
 * - gain maximum performance
 * - prevent dropping special guarantees offered by some derived classes (e.g. behavior in case of reset or
 *   power-loss during write)
 * - minimize wearing of storage cells (depends on derived class and type of storage)
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the write access is likely not completed
 * - the storage affected by the write access may contain undefined data
 * - storage not affected by the write access but located in the same page may contain undefined data
 * The exact behavior depends on the derived class.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the write access is likely not completed
 * - the storage affected by the write access may contain undefined data
 * - storage not affected by the write access but located in the same page may contain undefined data
 * The exact behavior depends on the derived class.
 *
 * ---
 *
 * \param address
 * Byte address inside the storage where to start writing.
 * \param n
 * Number of bytes that shall be written.
 * \param pBuffer
 * Buffer containing the data that shall be written.
 */

/**
 * \fn virtual bool IRandomAccessStorage::WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer) = 0
 * \brief Writes data into the storage, reads the written data back, and compares the data.
 *
 * The write operation is automatically splitted into multiple write operations in order to satisfy page boundary
 * requirements if necessary. The caller does not need to care about page boundaries. This method can be used to
 * write blocks of random size to random addresses inside the storage. Each (splitted) write operation will use
 * page-write mode if multiple bytes shall be written and if page-write is supported by the underlying storage device.
 *
 * However, depending on the particular derived class it may be useful to not write across page boundaries,
 * though it is supported by this method. Reasons for doing so:
 * - gain maximum performance
 * - prevent dropping special guarantees offered by some derived classes (e.g. behavior in case of reset or
 *   power-loss during write)
 * - minimize wearing of storage cells (depends on derived class and type of storage)
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the write access is likely not completed
 * - the storage affected by the write access may contain undefined data
 * - storage not affected by the write access but located in the same page may contain undefined data
 * The exact behavior depends on the derived class.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the write access is likely not completed
 * - the storage affected by the write access may contain undefined data
 * - storage not affected by the write access but located in the same page may contain undefined data
 * The exact behavior depends on the derived class.
 *
 * ---
 *
 * \param address
 * Byte address inside the storage where to start writing.
 * \param n
 * Number of bytes that shall be written.
 * \param pBuffer
 * Buffer containing the data that shall be written.
 * \param pAuxBuffer
 * Auxiliary buffer that can be used by this method to read back the written data.\n
 * _The buffer's size must be equal to or larger than parameter "n"._\n
 * If this is `nullptr`, then this method will allocate the required memory on the heap and release it
 * afterwards.
 * \return
 * true  = data read back matched
 * false = data read back did not match
 */

} // namespace StdIf
} // namespace gpcc

#endif // SRC_GPCC_STDIF_IRANDOMACCESSSTORAGE_HPP_
