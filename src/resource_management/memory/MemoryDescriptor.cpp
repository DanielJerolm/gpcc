/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/resource_management/memory/MemoryDescriptor.hpp>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

MemoryDescriptor::MemoryDescriptor(uint32_t const _startAddress, size_t const _size, bool const _free) noexcept
: startAddress(_startAddress)
, size(_size)
, free(_free)
, pPrevInMem(nullptr)
, pNextInMem(nullptr)
, pPrevInList(nullptr)
, pNextInList(nullptr)
/**
 * \brief Constructor.
 *
 * All double-linked list pointers (`pPrevInMem`, `pNextInMem`, `pPrevInList`, and `pNextInList`)
 * are initialized with nullptr. The other attributes are initialized with the parameters passed to this method.
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
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
 * \param _startAddress Initial value for @ref startAddress.
 * \param _size Initial value for @ref size.
 * \param _free Initial value for @ref free.
 */
{
}

void MemoryDescriptor::RemoveFromMemList(void) noexcept
/**
 * \brief Removes the memory descriptor from the double linked list made up by @ref pPrevInMem and @ref pNextInMem.
 *
 * @ref pPrevInMem and @ref pNextInMem are `nullptr` afterwards.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (pPrevInMem != nullptr)
    pPrevInMem->pNextInMem = pNextInMem;

  if (pNextInMem != nullptr)
    pNextInMem->pPrevInMem = pPrevInMem;

  pPrevInMem = nullptr;
  pNextInMem = nullptr;
}
void MemoryDescriptor::InsertIntoMemListBehindThis(MemoryDescriptor* const pNewDescr) noexcept
/**
 * \brief Inserts an @ref MemoryDescriptor instance behind this into the double linked list made
 * up by @ref pPrevInMem and @ref pNextInMem.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
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
 * \param pNewDescr
 * @ref MemoryDescriptor instance that shall be inserted into the double linked list made
 * up by @ref pPrevInMem and @ref pNextInMem. The new descriptor is inserted _behind_ this.
 */
{
  if (pNextInMem != nullptr)
    pNextInMem->pPrevInMem = pNewDescr;
  pNewDescr->pNextInMem = pNextInMem;

  pNewDescr->pPrevInMem = this;
  pNextInMem = pNewDescr;
}
void MemoryDescriptor::RemoveFromManagementList(void) noexcept
/**
 * \brief Removes the memory descriptor from the double linked list made up by @ref pPrevInList and @ref pNextInList.
 *
 * @ref pPrevInList and @ref pNextInList are `nullptr` afterwards.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (pPrevInList != nullptr)
    pPrevInList->pNextInList = pNextInList;

  if (pNextInList != nullptr)
    pNextInList->pPrevInList = pPrevInList;

  pPrevInList = nullptr;
  pNextInList = nullptr;
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc
