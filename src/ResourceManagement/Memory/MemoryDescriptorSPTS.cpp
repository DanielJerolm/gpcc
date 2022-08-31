/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "MemoryDescriptorSPTS.hpp"
#include "HeapManagerSPTS.hpp"
#include <gpcc/osal/Panic.hpp>
#include <stdexcept>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

MemoryDescriptorSPTS::MemoryDescriptorSPTS(std::shared_ptr<HeapManagerSPTS> const & _spHM, MemoryDescriptor* const _pMD, MemoryDescriptorSPTSKey)
: spHM(_spHM)
, pMD(_pMD)
/**
 * \brief Constructor. @ref MemoryDescriptorSPTSKey passkey is required.
 *
 * Note:\n
 * This constructor makes use of the passkey pattern to ensure that only @ref HeapManagerSPTS instances
 * can invoke it. The passkey pattern allows the constructor to be public (enables use of std::make_shared),
 * but restricts its use to friends of class @ref MemoryDescriptorSPTSKey.
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _spHM
 * Pointer to the @ref HeapManagerSPTS instance where the memory is allocated.\n
 * _nullptr is not allowed._
 * \param _pMD
 * Pointer to the @ref MemoryDescriptor instance that shall be encapsulated by this.\n
 * _nullptr is not allowed._\n
 * _The memory must have been allocated from the heap manager referenced by parameter _spHM._
 */
{
  if ((!_spHM) || (_pMD == nullptr))
    throw std::invalid_argument("MemoryDescriptorSPTS::MemoryDescriptorSPTS");
}

MemoryDescriptorSPTS::~MemoryDescriptorSPTS(void)
/**
 * \brief Destructor. Releases the referenced memory.
 *
 * __Thread safety:__\n
 * Do not access object after invocation of destructor.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations may only fail due to serious errors that will result in program termination via Panic(...).
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  spHM->Release(pMD);
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc
