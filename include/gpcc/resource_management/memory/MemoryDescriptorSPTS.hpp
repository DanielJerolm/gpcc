/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTORSPTS_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTORSPTS_HPP_

#include "MemoryDescriptor.hpp"
#include <memory>

namespace gpcc
{
namespace resource_management
{
namespace memory
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

} // namespace memory
} // namespace resource_management
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTORSPTS_HPP_
