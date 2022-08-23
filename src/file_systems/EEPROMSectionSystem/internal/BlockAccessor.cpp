/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "BlockAccessor.hpp"
#include <gpcc/crc/simple_crc.hpp>
#include "gpcc/src/file_systems/EEPROMSectionSystem/Exceptions.hpp"
#include "gpcc/src/Compiler/definitions.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/StdIf/IRandomAccessStorage.hpp"
#include <limits>
#include <stdexcept>
#include <cstddef>
#include <cstring>

namespace gpcc
{
namespace file_systems
{
namespace EEPROMSectionSystem
{
namespace internal
{

BlockAccessor::BlockAccessor(StdIf::IRandomAccessStorage & _storage, uint32_t const _startAddressInStorage, size_t const _sizeInStorage)
: storage(_storage)
, startAddressInStorage(_startAddressInStorage)
, sizeInStorage(_sizeInStorage)
, blockSize(0)
, nBlocks(0)
/**
 * \brief Constructor.
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
 * \param _storage
 * Storage on which the @ref EEPROMSectionSystem is working.
 * \param _startAddressInStorage
 * Start address inside the storage where the data managed by the @ref EEPROMSectionSystem resides.\n
 * Constraints:
 * - This must be aligned to a page boundary of the storage.
 * \param _sizeInStorage
 * Number of bytes granted to @ref EEPROMSectionSystem, starting at `_startAddressInStorage`.\n
 * Constraints:
 * - This must be a whole numbered multiple of the page size of the underlying storage.
 * - The memory range specified by `_startAddressInStorage` and `_sizeInStorage` must not
 *   exceed the end of the storage.
 * - The memory range specified by `_startAddressInStorage` and `_sizeInStorage` must be
 *   accessible using 32-bit addresses.
 * - This must be sufficient for at least (MinimumNbOfBlocks)[@ref EEPROMSectionSystem::MinimumNbOfBlocks] blocks
 *   of the smallest size (MinimumBlockSize)[@ref EEPROMSectionSystem::MinimumBlockSize].
 */
{
  // retrieve storage properties
  size_t const storageSize     = storage.GetSize();
  size_t const storagePageSize = storage.GetPageSize();

  // page-alignment required?
  if (storagePageSize != 0)
  {
    // check if page alignment is met
    if ((storageSize            % storagePageSize != 0) ||
        (startAddressInStorage  % storagePageSize != 0) ||
        (sizeInStorage          % storagePageSize != 0))
      throw std::invalid_argument("BlockAccessor::BlockAccessor: Page alignment not met");
  }

  // _sizeInStorage invalid?
  if (sizeInStorage < MinimumBlockSize * MinimumNbOfBlocks)
    throw std::invalid_argument("BlockAccessor::BlockAccessor: _sizeInStorage too small");

  // Specified memory block out of bounds? Accessible via 32-bit addresses?
  if ((startAddressInStorage > std::numeric_limits<size_t>::max() - sizeInStorage) ||
      (startAddressInStorage + sizeInStorage > std::numeric_limits<uint32_t>::max()) ||
      (startAddressInStorage + sizeInStorage > storageSize))
    throw std::invalid_argument("BlockAccessor::BlockAccessor: Memory out of bounds");
}

size_t BlockAccessor::GetSizeInStorage(void) const
/**
 * \brief Retrieves the size inside the storage.
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
 * Size inside the storage in byte.
 */
{
  return sizeInStorage;
}

size_t BlockAccessor::GetPageSize(void) const
/**
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
 * For storage that is not organized in pages (i.e. plain RAM), this returns zero.
 */
{
  return storage.GetPageSize();
}

void BlockAccessor::SetBlockSize(uint16_t const _blockSize)
/**
 * \brief Sets the size of the blocks established inside the storage.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
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
 * \param _blockSize
 * Desired size (in bytes) of the blocks established inside the storage.\n
 * Constraints:
 * - This must be larger than (MinimumBlockSize)[@ref EEPROMSectionSystem::MinimumBlockSize].
 * - This must not exceed (MaximumBlockSize)[@ref EEPROMSectionSystem::MaximumBlockSize].
 * - This must not exceed the page size of the underlying storage.
 * - This must divide the page size of the underlying storage without any remainder.
 * - The resulting number of blocks must be within (MinimumNbOfBlocks)[@ref EEPROMSectionSystem::MinimumNbOfBlocks]
 *   and (MaximumNbOfBlocks)[@ref EEPROMSectionSystem::MaximumNbOfBlocks].
 */
{
  // retrieve storage properties
  size_t const pageSize = storage.GetPageSize();

  // check _blockSize
  if ((_blockSize < MinimumBlockSize) ||
      (_blockSize > MaximumBlockSize))
    throw std::invalid_argument("BlockAccessor::SetBlockSize: _blockSize");

  // page-alignment required?
  if (pageSize != 0)
  {
    if (_blockSize > pageSize)
      throw std::invalid_argument("BlockAccessor::SetBlockSize: _blockSize exceeds storage page size");

    if (pageSize % _blockSize != 0)
      throw std::invalid_argument("BlockAccessor::SetBlockSize: n * _blockSize does not fit storage page size");
  }

  // calculate resulting number of blocks and check it
  size_t const _nBlocks = sizeInStorage / _blockSize;

  if ((_nBlocks < MinimumNbOfBlocks) ||
      (_nBlocks > MaximumNbOfBlocks))
    throw std::invalid_argument("BlockAccessor::SetBlockSize: Invalid number of blocks");

  // OK, adopt the new settings
  blockSize = _blockSize;
  nBlocks   = static_cast<uint16_t>(_nBlocks);
}
uint16_t BlockAccessor::GetBlockSize(void) const
/**
 * \brief Retrieves the configured size of the blocks that are established inside the storage.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \return
 * Currently configured size of the blocks (in byte) established inside the storage.
 */
{
  if (blockSize == 0U)
    throw std::logic_error("BlockAccessor::GetBlockSize: Block size not configured");

  return blockSize;
}
uint16_t BlockAccessor::GetnBlocks(void) const
/**
 * \brief Retrieves the configured number of blocks established inside the storage.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \return
 * Currently configured number of blocks established inside the storage.
 */
{
  if (nBlocks == 0U)
    throw std::logic_error("BlockAccessor::GetnBlocks: Block size not configured");

  return nBlocks;
}
size_t BlockAccessor::GetMaxSectionNameLength(void) const
/**
 * \brief Retrieves the maximum section name length in characters.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \return
 * The maximum allowed length for a section name in characters, without null-terminator.\n
 * Example: If this returns 5, then the section name may be comprised by up to 5 characters
 * plus the null-terminator.
 */
{
  if (blockSize == 0U)
    throw std::logic_error("BlockAccessor::MaxSectionNameLength: Block size not configured");

  return blockSize - (sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + 1U);
}

uint16_t BlockAccessor::LoadFields_type_sectionNameHash(uint16_t const blockIndex) const
/**
 * \brief Loads the values of the fields "type" and "sectionNameHash" of the common header of an block.
 *
 * _Note: No CRC check and no checks on the loaded value are included._
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \param blockIndex
 * Index of the block.
 * \return
 * Bits 0..7: Value of field "type".\n
 * Bits 15..8: Value of field "sectionNameHash".
 */
{
  // calculate address (includes checks if blockIndex is valid and if block size is configured)
  uint32_t const address = CalcBlockStartAddress(blockIndex) + offsetof(CommonBlockHead_t, type);

  uint16_t value;
  storage.Read(address, sizeof(value), &value);
  SwapEndian(value);
  return value;
}
uint8_t BlockAccessor::LoadField_type(uint16_t const blockIndex) const
/**
 * \brief Loads the value of the field "type" of the common header of an block.
 *
 * _Note: No CRC check and no checks on the loaded value are included._
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \param blockIndex
 * Index of the block.
 * \return
 * Value of field "type".
 */
{
  // calculate address (includes checks if blockIndex is valid and if block size is configured)
  uint32_t const address = CalcBlockStartAddress(blockIndex) + offsetof(CommonBlockHead_t, type);

  uint8_t value;
  storage.Read(address, sizeof(value), &value);
  return value;
}
uint32_t BlockAccessor::LoadField_totalNbOfWrites(uint16_t const blockIndex) const
/**
 * \brief Loads the value of the field "totalNbOfWrites" of the common header of an block.
 *
 * _Note: No CRC check and no checks on the loaded value are included._
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \param blockIndex
 * Index of the block.
 * \return
 * Value of field "totalNbOfWrites".
 */
{
  // calculate address (includes checks if blockIndex is valid and if block size is configured)
  uint32_t const address = CalcBlockStartAddress(blockIndex) + offsetof(CommonBlockHead_t, totalNbOfWrites);

  uint32_t value;
  storage.Read(address, sizeof(value), &value);
  SwapEndian(value);
  return value;
}
uint16_t BlockAccessor::LoadField_nextBlock(uint16_t const blockIndex) const
/**
 * \brief Loads the value of the field "nextBlock" of the common header of an block.
 *
 * _Note: No CRC check and no checks on the loaded value are included._
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \param blockIndex
 * Index of the block.
 * \return
 * Value of field "nextBlock".
 */
{
  // calculate address (includes checks if blockIndex is valid and if block size is configured)
  uint32_t const address = CalcBlockStartAddress(blockIndex) + offsetof(CommonBlockHead_t, nextBlock);

  uint16_t value;
  storage.Read(address, sizeof(value), &value);
  SwapEndian(value);
  return value;
}

void BlockAccessor::LoadBlock(uint16_t const blockIndex, void* const pBuffer, size_t const maxLength) const
/**
 * \brief Loads an block from the storage and takes care of endian, CRC, and basic error checks.
 *
 * The following checks are performed on the loaded block:
 * - common header, type
 * - common header, section name hash
 * - common header, nBytes
 * - common header, nextBlock (range; not referencing to block zero or itself; NOBLOCK allowed/required)
 * - CRC
 * - presence of a null-terminator in section's name (only blocks of type BlockTypes::sectionHead)
 * - sequence number range (only blocks of type BlockTypes::sectionData)
 *
 * The data of the block header inside the storage is little endian. The endian is automatically
 * swapped if this code is running on a big endian machine.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the memory referenced by `pBuffer` may contain undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the memory referenced by `pBuffer` may contain undefined data.
 *
 * ---
 *
 * \param blockIndex
 * Index of the storage block that shall be loaded.
 * \param pBuffer
 * Pointer to a buffer into which the content of the storage block shall be loaded.\n
 * The buffer must have a capacity of at least `maxLength` bytes.\n
 * \param maxLength
 * If the length of the block that shall be loaded (nBytes-field of common header) would exceed this value,
 * then the block is not loaded and an exception is thrown.
 */
{
  // calculate block start address (includes checks if blockIndex is valid and if block size is configured)
  uint32_t const blockStartAddress = CalcBlockStartAddress(blockIndex);

  // load field "nBytes" from common header
  uint16_t nBytes;
  storage.Read(blockStartAddress + offsetof(CommonBlockHead_t, nBytes), sizeof(nBytes), &nBytes);
  SwapEndian(nBytes);

  // check nBytes
  if ((nBytes < sizeof(CommonBlockHead_t) + sizeof(uint16_t)) ||
      (nBytes > blockSize))
    throw InvalidHeaderError("Bad \"nBytes\"", blockIndex);

  if (static_cast<size_t>(nBytes) > maxLength)
    throw InvalidHeaderError("Unexpected \"nBytes\"", blockIndex);

  // load the first "nBytes" byte of the whole block
  storage.Read(blockStartAddress, nBytes, pBuffer);

  // check CRC
  if (!CheckCRC(pBuffer, nBytes - 2U))
    throw CRCError(blockIndex);

  // get pointer to access the header
  CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t*>(pBuffer);

  // check data integrity and swap endian if required
  switch (pHead->type)
  {
    // ------------------------------------------------------
    case static_cast<uint8_t>(BlockTypes::sectionSystemInfo):
    // ------------------------------------------------------
      if (nBytes != sizeof(SectionSystemInfoBlock_t) + sizeof(uint16_t))
        throw InvalidHeaderError("Bad \"nBytes\"", blockIndex);

      SwapEndian(static_cast<SectionSystemInfoBlock_t*>(pBuffer));

      if (pHead->sectionNameHash != 0)
        throw InvalidHeaderError("Bad \"sectionNameHash\"", blockIndex);

      if (pHead->nextBlock != NOBLOCK)
        throw InvalidHeaderError("Bad \"nextBlock\"", blockIndex);
      break;

    // ----------------------------------------------
    case static_cast<uint8_t>(BlockTypes::freeBlock):
    // ----------------------------------------------
      if (nBytes != sizeof(CommonBlockHead_t) + sizeof(uint16_t))
        throw InvalidHeaderError("Bad \"nBytes\"", blockIndex);

      SwapEndian(static_cast<CommonBlockHead_t*>(pBuffer));

      if (pHead->sectionNameHash != 0)
        throw InvalidHeaderError("Bad \"sectionNameHash\"", blockIndex);
      break;

    // ------------------------------------------------
    case static_cast<uint8_t>(BlockTypes::sectionHead):
    // ------------------------------------------------
      // check size, incl. null-terminator and name (1 char)
      if (nBytes < sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + 2U)
        throw InvalidHeaderError("Bad \"nBytes\"", blockIndex);

      // check that null-terminator is present in section's name
      if (static_cast<char const*>(pBuffer)[nBytes - sizeof(uint16_t) - 1U] != 0)
        throw InvalidHeaderError("Missing null-terminator in section name", blockIndex);

      // check that there is only one null-terminator present in section's name
      if (strlen(static_cast<char const*>(pBuffer) + sizeof(SectionHeadBlock_t)) != pHead->nBytes - (sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + 1U))
        throw InvalidHeaderError("Multiple null-terminators in section name", blockIndex);

      SwapEndian(static_cast<SectionHeadBlock_t*>(pBuffer));

      // check section name hash
      if (pHead->sectionNameHash != CalcHash(static_cast<char const*>(pBuffer) + sizeof(SectionHeadBlock_t)))
        throw InvalidHeaderError("Invalid \"sectionNameHash\"", blockIndex);

      if (pHead->nextBlock == NOBLOCK)
        throw InvalidHeaderError("Bad \"nextBlock\"", blockIndex);
      break;

    // ------------------------------------------------
    case static_cast<uint8_t>(BlockTypes::sectionData):
    // ------------------------------------------------
      if (nBytes < sizeof(DataBlock_t) + sizeof(uint16_t))
        throw InvalidHeaderError("Bad \"nBytes\"", blockIndex);

      SwapEndian(static_cast<DataBlock_t*>(pBuffer));

      if (pHead->sectionNameHash != 0)
        throw InvalidHeaderError("Bad \"sectionNameHash\"", blockIndex);

      if (static_cast<DataBlock_t*>(pBuffer)->seqNb > nBlocks - 2U)
        throw InvalidHeaderError("Bad \"seqNb\"", blockIndex);
      break;

    // -----
    default:
    // -----
      throw InvalidHeaderError("Bad \"type\"", blockIndex);
  } // switch (pHead->type)

  // "nBytes" has been loaded twice. The values must be equal.
  if (pHead->nBytes != nBytes)
    throw VolatileStorageError(blockIndex);

  // check nextBlock
  if ((pHead->nextBlock != NOBLOCK) &&
      ((pHead->nextBlock >= nBlocks) ||
       (pHead->nextBlock == 0) ||
       (pHead->nextBlock == blockIndex)))
    throw InvalidHeaderError("Bad \"nextBlock\"", blockIndex);
}
void BlockAccessor::StoreBlock(uint16_t const blockIndex, void* const pBuffer, void* const pAuxBuf, bool const recoverEndian)
/**
 * \brief Stores an block into the storage and takes care for endian, CRC, and basic error checks.
 *
 * Before writing to the storage, the following error checks are performed on the block
 * that shall be written:
 * - common header, type
 * - common header, section name hash (all except for blocks of type BlockTypes::sectionHead)
 * - common header, nBytes
 * - common header, nextBlock (range; not referencing to block zero or itself; NOBLOCK allowed/required)
 * - presence of a null-terminator in section's name (only blocks of type BlockTypes::sectionHead)
 * - sequence number range (only blocks of type BlockTypes::sectionData)
 *
 * The following modifications are done to the data that shall be written into the block:
 * - The field "totalNbOfWrites" is incremented
 * - The endian of the header is swapped if this is running on a big endian machine.\n
 *   This ensures that little endian format is used within the storage.
 * - The CRC field is overwritten with a valid checksum value.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe, if `blockIndex` is different
 * among all simultaneous calls to this method or to @ref LoadBlock() or any of the `LoadField_...()` methods.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the write access to the storage may have not been started
 * - the storage elements affected by the write access may contain undefined data
 * - storage elements not affected by the write access but located in the same page may contain undefined data
 * The exact behavior depends on the type of the underlying storage this class is working on.
 *
 * Regarding `pBuffer` in case of an exception:
 * - field "totalNbOfWrites" is only recovered to the original value if the exception was thrown before
 *   attempting to write to the storage.
 * - field "CRC" may or may not be updated with a valid CRC
 * - the endian of the header is recovered depending on parameter `recoverEndian`.
 * Apart from this there are no other side effects and all other data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the write access to the storage may have not been started
 * - the storage elements affected by the write access may contain undefined data
 * - storage elements not affected by the write access but located in the same page may contain undefined data
 * The exact behavior depends on the type of the underlying storage this class is working on.
 *
 * ---
 *
 * \param blockIndex
 * Index of the storage block that shall be written.
 * \param pBuffer
 * Pointer to a buffer containing the data (incl. header and placeholder for CRC) that shall
 * be written into the storage block.\n
 * Note that the content of the referenced memory is modified by this method:
 * - increment of field "totalNbOfWrites"
 * - update of field "CRC" with a valid checksum
 * - potential endian swap of block header fields
 * If a potential endian swap shall be reverted if this method returns or throws, then parameter
 * `recoverEndian` must be set to true.
 * \param pAuxBuf
 * Pointer to a buffer that shall be used to read-back and compare the written data.\n
 * The buffer must have at least the size of the data that shall be written.\n
 * If this is `nullptr`, then the underlying storage manager will allocate memory from the heap and
 * release the memory afterwards.
 * \param recoverEndian
 * If this is running on a big endian machine, then the endian of the block header will
 * be swapped inside the memory referenced by `pBuffer` before writing it to the storage.\n
 * This parameter controls if the endian inside the memory referenced by `pBuffer` shall be recovered when
 * this method returns or throws:\n
 * true  = recover\n
 * false = do not recover (faster)
 */
{
  // calculate block start address (includes checks if blockIndex is valid and if block size is configured)
  uint32_t const blockStartAddress = CalcBlockStartAddress(blockIndex);

  // get pointer to access the header
  CommonBlockHead_t * const pHead = static_cast<CommonBlockHead_t*>(pBuffer);

  // check nBytes
  if ((pHead->nBytes < sizeof(CommonBlockHead_t) + sizeof(uint16_t)) ||
      (pHead->nBytes > blockSize))
    throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

  // check nextBlock
  if ((pHead->nextBlock != NOBLOCK) &&
      ((pHead->nextBlock >= nBlocks) ||
       (pHead->nextBlock == 0) ||
       (pHead->nextBlock == blockIndex)))
    throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

  // increment number of writes
  uint32_t const prevTotalNbOfWrites = pHead->totalNbOfWrites;

  if (pHead->totalNbOfWrites != std::numeric_limits<uint32_t>::max())
    pHead->totalNbOfWrites++;

  ON_SCOPE_EXIT(recoverTotalNbOfWrites)
  {
    pHead->totalNbOfWrites = prevTotalNbOfWrites;
  };

  // calculate and append CRC
  CalcCRC(pBuffer, pHead->nBytes - sizeof(uint16_t));

  // check data integrity and swap endian if required
  switch (static_cast<BlockTypes>(pHead->type))
  {
    // --------------------------------
    case BlockTypes::sectionSystemInfo:
    // --------------------------------
      if ((pHead->nBytes != sizeof(SectionSystemInfoBlock_t) + sizeof(uint16_t)) ||
          (pHead->sectionNameHash != 0) ||
          (pHead->nextBlock != NOBLOCK))
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      ON_SCOPE_EXIT_DISMISS(recoverTotalNbOfWrites);
      SwapEndian(static_cast<SectionSystemInfoBlock_t*>(pBuffer));
      break;

    // ------------------------
    case BlockTypes::freeBlock:
    // ------------------------
      if ((pHead->nBytes != sizeof(CommonBlockHead_t) + sizeof(uint16_t)) ||
          (pHead->sectionNameHash != 0))
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      ON_SCOPE_EXIT_DISMISS(recoverTotalNbOfWrites);
      SwapEndian(static_cast<CommonBlockHead_t*>(pBuffer));
      break;

    // --------------------------
    case BlockTypes::sectionHead:
    // --------------------------
      // note: size check incl. null-terminator and name (1 char)
      if ((pHead->nBytes < sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + 2U) ||
          (pHead->nextBlock == NOBLOCK))
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      // check that null-terminator is present in section's name
      if (static_cast<char const*>(pBuffer)[pHead->nBytes - (sizeof(uint16_t) + 1U)] != 0)
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      // check that there is only one null-terminator present in section's name
      if (strlen(static_cast<char const*>(pBuffer) + sizeof(SectionHeadBlock_t)) != pHead->nBytes - (sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + 1U))
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      ON_SCOPE_EXIT_DISMISS(recoverTotalNbOfWrites);
      SwapEndian(static_cast<SectionHeadBlock_t*>(pBuffer));
      break;

    // --------------------------
    case BlockTypes::sectionData:
    // --------------------------
      if ((pHead->nBytes < sizeof(DataBlock_t) + sizeof(uint16_t)) ||
          (pHead->sectionNameHash != 0))
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      if (static_cast<DataBlock_t const *>(pBuffer)->seqNb > nBlocks - 2U)
        throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));

      ON_SCOPE_EXIT_DISMISS(recoverTotalNbOfWrites);
      SwapEndian(static_cast<DataBlock_t*>(pBuffer));
      break;

    // -----
    default:
    // -----
      throw std::logic_error(std::string("BlockAccessor::StoreBlock: Bad data, attempt to write block ") + std::to_string(blockIndex));
  } // switch (static_cast<BlockTypes>(pHead->type))

  // recover endian if required
  ON_SCOPE_EXIT(recoverEndian)
  {
    if ((recoverEndian) && (!SwapEndian(pBuffer)))
      PANIC(); // Memory corrupted
  };

  if (!storage.WriteAndCheck(blockStartAddress, pHead->nBytes, pBuffer, pAuxBuf))
    throw VolatileStorageError(blockIndex);
}

uint32_t BlockAccessor::CalcBlockStartAddress(uint16_t const blockIndex) const
/**
 * \brief Calculates the absolute start address of an block inside the storage.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \param blockIndex
 * Index of the block, whose start address shall be calculated.
 * \return
 * Absolute start address of the block referenced by `blockIndex` inside the storage.
 */
{
  if (nBlocks == 0U)
    throw std::logic_error("BlockAccessor::CalcBlockStartAddress: Block size not configured");

  if (blockIndex >= nBlocks)
    throw std::invalid_argument("BlockAccessor::CalcBlockStartAddress: Invalid index");

  return startAddressInStorage + static_cast<uint32_t>(blockIndex) * blockSize;
}

void BlockAccessor::CalcCRC(void* const pData, size_t const n) const noexcept
/**
 * \brief Calculates a checksum over an block of data and appends it to the data.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pData
 * Pointer to the data. The data must be in little endian format.\n
 * The checksum is calculated over n bytes of data and __appended__ to the data.
 * \param n
 * Number of data bytes referenced by `pData`, CRC __not__ included.\n
 * Example:
 * ~~~{.cpp}
 * // storage for 4 bytes of data __PLUS__ 2 bytes for the checksum
 * uint8_t data[6] = { 0x01, 0x02, 0x03, 0x04, 0x00, 0x00};
 *
 * // calculate checksum
 * CalcCRC(data, 4);
 * ~~~
 */
{
  // calculate CRC
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, pData, n, gpcc::crc::crc16_ccitt_table_normal);

  // append CRC to the block of data (little endian)
  char* pCRC = static_cast<char*>(pData) + n;
  *pCRC++ = static_cast<char>(crc & 0xFFU);
  *pCRC   = static_cast<char>(crc >> 8U);
}
bool BlockAccessor::CheckCRC(const void* const pData, size_t const n) const noexcept
/**
 * \brief Checks the checksum appended to a block of data.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pData
 * Pointer to the data. The data must be in little endian format.\n
 * The checksum (also in little endian format) must be located behind the block of data referenced by this.
 * \param n
 * Size of the data block referenced by this in bytes, CRC __not__ included.\n
 * Example:
 * ~~~{.cpp}
 * // storage for 4 bytes of data __PLUS__ 2 bytes of checksum
 * uint8_t data[6] = { 0x01, 0x02, 0x03, 0x04, 0x12, 0x13};
 *
 * // check checksum
 * bool crcOK = CheckCRC(data, 4);
 * ~~~
 * \return
 * Result of the check:\n
 * true  = checksum is OK\n
 * false = checksum is invalid
 */
{
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, pData, n, gpcc::crc::crc16_ccitt_table_normal);

  char const * const pCRC = static_cast<char const *>(pData) + n;

  return ((pCRC[0] == static_cast<char>(crc & 0xFFU)) &&
          (pCRC[1] == static_cast<char>(crc >> 8U)));
}

void BlockAccessor::SwapEndian(uint16_t& u16) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of a referenced variable.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param u16
 * Reference to the variable whose endian shall be swapped.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  u16 = (u16 >> 8U) |
        (u16 << 8U);
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)u16;
#else
  #error "Unsupported endian"
#endif
}
void BlockAccessor::SwapEndian(uint32_t& u32) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of a referenced variable.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param u32
 * Reference to the variable whose endian shall be swapped.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  u32 = (u32 >> 24U) |
        ((u32 >> 8U)  &   0xFF00U) |
        ((u32 << 8U)  & 0xFF0000U) |
        (u32 << 24U);
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)u32;
#else
  #error "Unsupported endian"
#endif
}
void BlockAccessor::SwapEndian(CommonBlockHead_t* const pData) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of the fields inside a @ref CommonBlockHead_t structure.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pData
 * Pointer to the @ref CommonBlockHead_t structure.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  char* const p = static_cast<char*>(static_cast<void*>(pData));
  char t;

  // field nBytes
  t     = p[2];
  p[2]  = p[3];
  p[3]  = t;

  // field totalNbOfWrites
  t     = p[4];
  p[4]  = p[7];
  p[7]  = t;
  t     = p[5];
  p[5]  = p[6];
  p[6]  = t;

  // field nextBlock
  t     = p[8];
  p[8]  = p[9];
  p[9]  = t;
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)pData;
#else
  #error "Unsupported endian"
#endif
}
void BlockAccessor::SwapEndian(SectionSystemInfoBlock_t* const pData) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of the fields inside a @ref SectionSystemInfoBlock_t structure.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pData
 * Pointer to the @ref SectionSystemInfoBlock_t structure.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  SwapEndian(&pData->head);

  char* p = static_cast<char*>(static_cast<void*>(pData)) + sizeof(CommonBlockHead_t);
  char t;

  // field sectionSystemVersion
  t     = p[0];
  p[0]  = p[1];
  p[1]  = t;

  // field eepromBlockSize
  t     = p[2];
  p[2]  = p[3];
  p[3]  = t;

  // field nBlocks
  t     = p[4];
  p[4]  = p[5];
  p[5]  = t;
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)pData;
#else
  #error "Unsupported endian"
#endif
}
void BlockAccessor::SwapEndian(SectionHeadBlock_t* const pData) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of the fields inside a @ref SectionHeadBlock_t structure.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pData
 * Pointer to the @ref SectionHeadBlock_t structure.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  SwapEndian(&pData->head);

  char* const p = static_cast<char*>(static_cast<void*>(pData)) + sizeof(CommonBlockHead_t);
  char t;

  // field version
  t     = p[0];
  p[0]  = p[1];
  p[1]  = t;
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)pData;
#else
  #error "Unsupported endian"
#endif
}
void BlockAccessor::SwapEndian(DataBlock_t* const pData) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of the fields inside a @ref DataBlock_t structure.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pData
 * Pointer to the @ref DataBlock_t structure.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  SwapEndian(&pData->head);

  char* p = static_cast<char*>(static_cast<void*>(pData)) + sizeof(CommonBlockHead_t);
  char t;

  // field "seqNb"
  t    = p[0];
  p[0] = p[1];
  p[1] = t;
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)pData;
#else
  #error "Unsupported endian"
#endif
}
bool BlockAccessor::SwapEndian(void* const pBlock) const noexcept
/**
 * \brief On big endian machines, this swaps the endian of the fields inside the header of an
 * @ref EEPROMSectionSystem block.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param pBlock
 * Pointer to a buffer containing the @ref EEPROMSectionSystem block.\n
 * The buffer must contain a valid @ref EEPROMSectionSystem structure with a valid common header (CommonBlockHead_t).
 * \return
 * true  = OK.\n
 * false = type-field of the common header of the @ref EEPROMSectionSystem block was invalid. No swap done.
 */
{
#if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
  CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t*>(static_cast<void*>(pBlock));

  switch (static_cast<BlockTypes>(pHead->type))
  {
    case BlockTypes::sectionSystemInfo:
      SwapEndian(static_cast<SectionSystemInfoBlock_t*>(static_cast<void*>(pBlock)));
      break;

    case BlockTypes::freeBlock:
      SwapEndian(static_cast<CommonBlockHead_t*>(static_cast<void*>(pBlock)));
      break;

    case BlockTypes::sectionHead:
      SwapEndian(static_cast<SectionHeadBlock_t*>(static_cast<void*>(pBlock)));
      break;

    case BlockTypes::sectionData:
      SwapEndian(static_cast<DataBlock_t*>(static_cast<void*>(pBlock)));
      break;

    default:
      return false;
  }
  return true;
#elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
  (void)pBlock;
  return true;
#else
  #error "Unsupported endian"
#endif
}

} // namespace internal
} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc
