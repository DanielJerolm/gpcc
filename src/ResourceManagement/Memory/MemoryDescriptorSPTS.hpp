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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTORSPTS_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTORSPTS_HPP_

#include "MemoryDescriptor.hpp"
#include <memory>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

class HeapManagerSPTS;

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * \brief Key for constructor of class @ref MemoryDescriptorSPTS (passkey pattern).
 */
class MemoryDescriptorSPTSKey
{
  friend class HeapManagerSPTS;
  MemoryDescriptorSPTSKey(void) = default;
};

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * \brief A memory descriptor referencing a chunk of memory managed by class @ref HeapManagerSPTS.
 *
 * The referenced memory can be physical, virtual, or even fictious.
 *
 * # Usage outside of GPCC (user code)
 * Instances of this class are retrieved from an @ref HeapManagerSPTS upon allocation of memory.
 * @ref MemoryDescriptorSPTS instances act as an handle to the allocated memory that must be kept by the owner(s)
 * of the allocated memory. Smart pointers (std::shared_ptr<MemoryDescriptorSPTS>) are used to reference to
 * instances of this class. The smart pointers allow to share an @ref MemoryDescriptorSPTS safely between
 * multiple objects/owners.
 *
 * The referenced memory is released to the @ref HeapManagerSPTS when the associated @ref MemoryDescriptorSPTS
 * instance is released. This happens automatically after the last smart pointer referencing to the
 * @ref MemoryDescriptorSPTS instance has gone.
 *
 * The address and size of the referenced memory can be retrieved via @ref GetStartAddress() and @ref GetSize().
 *
 * Each @ref MemoryDescriptorSPTS instance keeps an std::shared_ptr to the @ref HeapManagerSPTS instance
 * from which it had been retrieved. This approach keeps the @ref HeapManagerSPTS existing until all
 * @ref MemoryDescriptorSPTS instances allocated from it have been released.
 *
 * # Usage inside GPCC
 * This is an RAII wrapper for class @ref MemoryDescriptor, which is internally used by class @ref HeapManagerSPTS.
 */
class MemoryDescriptorSPTS
{
    friend class HeapManagerSPTS;

  public:
    MemoryDescriptorSPTS(void) = delete;
    MemoryDescriptorSPTS(std::shared_ptr<HeapManagerSPTS> const & _spHM, MemoryDescriptor* const _pMD, MemoryDescriptorSPTSKey);
    MemoryDescriptorSPTS(const MemoryDescriptorSPTS&) = delete;
    MemoryDescriptorSPTS(MemoryDescriptorSPTS&&) = delete;
    ~MemoryDescriptorSPTS(void);

    MemoryDescriptorSPTS& operator=(const MemoryDescriptorSPTS&) = delete;
    MemoryDescriptorSPTS& operator=(MemoryDescriptorSPTS&&) = delete;

    inline uint32_t GetStartAddress(void) const noexcept { return pMD->GetStartAddress(); }
    inline size_t GetSize(void) const noexcept { return pMD->GetSize(); }

  private:
    /// Pointer to the @ref HeapManagerSPTS instance where the referenced memory has been allocated at.
    std::shared_ptr<HeapManagerSPTS> spHM;

    /// Pointer to a @ref MemoryDescriptor instance referencing the memory.
    MemoryDescriptor* const pMD;
};

/**
 * \fn uint32_t MemoryDescriptorSPTS::GetStartAddress(void) const
 * \brief Retrieves the start address of the referenced piece of memory.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * Start address of the referenced piece of memory.\n
 * The start address is valid until the @ref MemoryDescriptorSPTS instance is released.
 */
/**
 * \fn size_t MemoryDescriptorSPTS::GetSize(void) const
 * \brief Retrieves the size of the referenced piece of memory.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * Size of the referenced piece of memory.\n
 * The value is valid until the @ref MemoryDescriptorSPTS instance is released.
 */

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTORSPTS_HPP_
