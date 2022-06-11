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

#include "MemoryDescriptorSPTS.hpp"
#include "HeapManagerSPTS.hpp"
#include "gpcc/src/osal/Panic.hpp"
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
