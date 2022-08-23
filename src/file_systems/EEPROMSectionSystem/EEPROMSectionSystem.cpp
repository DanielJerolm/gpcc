/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "EEPROMSectionSystem.hpp"
#include "Exceptions.hpp"
#include <gpcc/container/BitField.hpp>
#include "gpcc/src/file_systems/exceptions.hpp"
#include "internal/SectionReader.hpp"
#include "internal/SectionWriter.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <limits>
#include <stdexcept>
#include <cassert>
#include <cstring>

namespace gpcc
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

#ifndef __DOXYGEN__
size_t   const EEPROMSectionSystem::MinimumBlockSize;
size_t   const EEPROMSectionSystem::MaximumBlockSize;
size_t   const EEPROMSectionSystem::MinimumNbOfBlocks;
size_t   const EEPROMSectionSystem::MaximumNbOfBlocks;
uint16_t const EEPROMSectionSystem::Version;
#endif

using namespace internal;

EEPROMSectionSystem::EEPROMSectionSystem(StdIf::IRandomAccessStorage& _storage, uint32_t const _startAddressInStorage, size_t const _sizeInStorage)
: mutex()
, state(States::not_mounted)
, sectionLockManager()
, storage(_storage, _startAddressInStorage, _sizeInStorage)
, nFreeBlocks(0)
, freeBlockListHeadIdx(NOBLOCK)
, freeBlockListEndIdx(NOBLOCK)
/**
 * \brief Constructor. Creates an @ref EEPROMSectionSystem instance.
 *
 * Before the EEPROM Section System can be used, a Section System must be created or mounted:
 * - invoke @ref Format() to create a new Section System inside the storage.
 * - invoke @ref MountStep1() and @ref MountStep2() to mount an Section System that is
 *   already existing inside the storage.
 *
 * Please also refer to chapter "Mounting" in the detailed documentation of class @ref EEPROMSectionSystem.
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
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _storage
 * Storage on which the @ref EEPROMSectionSystem shall be working.\n
 * See chapter "Storage Requirements" in the detailed documentation of @ref EEPROMSectionSystem for
 * requirements that must be met by the underlying storage.
 * \param _startAddressInStorage
 * Start address inside the storage where the data managed by the @ref EEPROMSectionSystem resides.\n
 * Constraints:
 * - This must be aligned to a page boundary of the storage.
 * \param _sizeInStorage
 * Number of bytes assigned to the @ref EEPROMSectionSystem, starting at `_startAddressInStorage`.\n
 * Constraints:
 * - This must be a whole numbered multiple of the page size of the underlying storage.
 * - The memory range specified by `_startAddressInStorage` and `_sizeInStorage` must not
 *   exceed the end of the storage.
 * - This must be sufficient for at least @ref MinimumNbOfBlocks blocks of the smallest size
 *   @ref MinimumBlockSize.
 */
{
}
EEPROMSectionSystem::~EEPROMSectionSystem(void)
/**
 * \brief Destructor.
 *
 * The section system must be in state @ref States::not_mounted.
 *
 * ---
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
  osal::MutexLocker mutexLocker(mutex);
  if (state != States::not_mounted)
    PANIC();
}

char const * EEPROMSectionSystem::States2String(States const state)
/**
 * \brief Retrieves a null-terminated c-string with the name of an @ref States value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \param state
 * @ref States value.
 * \return
 * Null-terminated c-string with the name of an @ref States value.
 */
{
  switch (state)
  {
    case States::not_mounted: return "not_mounted";
    case States::ro_mount:    return "ro_mount";
    case States::checking:    return "checking";
    case States::mounted:     return "mounted";
    case States::defect:      return "defect";
    default:
      throw std::logic_error("EEPROMSectionSystem::States2String: Invalid state");
  }
}

EEPROMSectionSystem::States EEPROMSectionSystem::GetState(void) const
/**
 * \brief Retrieves the current state of the Section System.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * Current state of the Section System.
 */
{
  osal::MutexLocker mutexLocker(mutex);
  return state;
}
void EEPROMSectionSystem::Format(uint16_t const desiredBlockSize)
/**
 * \brief Creates a new empty Section System inside the storage and mounts it.
 *
 * __Warning:__\n
 * The current content of the storage will be overwritten!\n
 * This also applies to any wear-leveling information!
 *
 * The Section System must be unmounted (@ref States::not_mounted) when this is executed.\n
 * After successful execution, the Section System will be mounted (@ref States::mounted).
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the storage content is undefined.
 * - state will be @ref States::not_mounted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the storage content is undefined.
 * - state will be @ref States::not_mounted.
 *
 * ---
 *
 * \param desiredBlockSize
 * Desired block size (in bytes) for internal organization of the storage.\n
 * Constraints:
 * - This must be within @ref MinimumBlockSize and @ref MaximumBlockSize.
 * - This must not exceed the page size of the underlying storage.
 * - This must divide the page size of the underlying storage without any remainder.
 * - The resulting number of blocks must be within @ref MinimumNbOfBlocks and @ref MaximumNbOfBlocks.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (state != States::not_mounted)
    throw InsufficientStateError("EEPROMSectionSystem::Format", state);

  // reconfigure block-level access (this includes complete check of parameter "desiredBlockSize")
  storage.SetBlockSize(desiredBlockSize);
  uint16_t const blocksize = storage.GetBlockSize();
  uint16_t const nBlocks   = storage.GetnBlocks();

  std::unique_ptr<char[]> spMainBuf(new char[blocksize]);
  std::unique_ptr<char[]> spAuxBuf(new char[blocksize]);

  nFreeBlocks           = 0;
  freeBlockListHeadIdx  = NOBLOCK;
  freeBlockListEndIdx   = NOBLOCK;

  // --------------------------------------
  // LINK UNUSED BLOCKS TOGETHER IN STORAGE
  // --------------------------------------

  // Build a template empty block in memory. The template will be completed in the for-loop below.
  memset(spMainBuf.get(), 0x00, blocksize);
  CommonBlockHead_t* const pFreeBlock = static_cast<CommonBlockHead_t*>(static_cast<void*>(spMainBuf.get()));
  pFreeBlock->type            = static_cast<uint8_t>(BlockTypes::freeBlock);
  pFreeBlock->sectionNameHash = 0;
  pFreeBlock->nBytes          = sizeof(CommonBlockHead_t) + sizeof(uint16_t);
  pFreeBlock->totalNbOfWrites = 1;

  // write empty blocks to storage
  for (uint16_t i = 1U; i < nBlocks; i++)
  {
    // complete the template
    if (i < nBlocks - 1U)
      pFreeBlock->nextBlock = i + 1U;
    else
      pFreeBlock->nextBlock = NOBLOCK;

    storage.StoreBlock(i, spMainBuf.get(), spAuxBuf.get(), true);
  }

  // -------------------------------------------
  // CREATE SECTION SYSTEM INFO BLOCK IN STORAGE
  // -------------------------------------------

  memset(spMainBuf.get(), 0x00, blocksize);
  SectionSystemInfoBlock_t* const pInfoBlock = static_cast<SectionSystemInfoBlock_t*>(static_cast<void*>(spMainBuf.get()));
  pInfoBlock->head.type             = static_cast<uint8_t>(BlockTypes::sectionSystemInfo);
  pInfoBlock->head.sectionNameHash  = 0;
  pInfoBlock->head.nBytes           = sizeof(SectionSystemInfoBlock_t) + sizeof(uint16_t);
  pInfoBlock->head.totalNbOfWrites  = 1;
  pInfoBlock->head.nextBlock        = NOBLOCK;
  pInfoBlock->sectionSystemVersion  = Version;
  pInfoBlock->blockSize             = blocksize;
  pInfoBlock->nBlocks               = nBlocks;

  storage.StoreBlock(0, spMainBuf.get(), spAuxBuf.get(), false);

  // --------
  // FINISHED
  // --------
  nFreeBlocks           = nBlocks - 1U;
  freeBlockListHeadIdx  = 1U;
  freeBlockListEndIdx   = nBlocks - 1U;

  state = States::mounted;
}
void EEPROMSectionSystem::MountStep1(void)
/**
 * \brief Mounts the Section System (first step).
 *
 * This mounts an unmounted Section System. The @ref EEPROMSectionSystem must be in @ref States::not_mounted.
 * If the operation is successful, then the @ref EEPROMSectionSystem will be switched to @ref States::ro_mount
 * and read-access to the Section System will be possible.\n
 * To allow for write-access, @ref MountStep2() must be invoked after this.
 *
 * In case of any error, the @ref EEPROMSectionSystem will remain in @ref States::not_mounted.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.\n
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (state != States::not_mounted)
    throw InsufficientStateError("EEPROMSectionSystem::MountStep1", state);

  // Strategy:
  // First we will setup a block size that is sufficient for loading the Section System Info Block. After loading the
  // info block the block size will be reconfigured to the block size value extracted from the info block.
  // Reconfiguration includes a complete check of the block size value against various constraints incl. the properties
  // of the storage device.

  // query properties of storage
  size_t const storageSize     = storage.GetSizeInStorage();
  size_t const storagePageSize = storage.GetPageSize();

  if (storagePageSize != 0U)
  {
    if (storagePageSize < MinimumBlockSize)
      throw std::logic_error("EEPROMSectionSystem::MountStep1: Page size of storage device is too small.");
  }

  // determine a feasible initial block size by trying block sizes which are a power of 2
  size_t blockSize = 0U;
  for (uint_fast8_t i = 0U; i < 32U; i++)
  {
    size_t const trialBlockSize = 1UL << i;

    if (trialBlockSize < MinimumBlockSize)
      continue;

    if (trialBlockSize > MaximumBlockSize)
      throw std::logic_error("EEPROMSectionSystem::MountStep1: Cannot figure out suitable initial block size");

    // calculate resulting number of blocks
    size_t nBlocks = storageSize / trialBlockSize;

    if (nBlocks > MaximumNbOfBlocks)
      continue;

    if (nBlocks < MinimumNbOfBlocks)
      throw std::logic_error("EEPROMSectionSystem::MountStep1: Cannot figure out suitable initial block size");

    // check against page size if any value is given
    if (storagePageSize != 0U)
    {
      if (trialBlockSize > storagePageSize)
        throw std::logic_error("EEPROMSectionSystem::MountStep1: Cannot figure out suitable initial block size");

      if (storagePageSize % trialBlockSize != 0U)
        continue;
    }

    // suitable!
    blockSize = trialBlockSize;
    break;
  }

  // no suitable value found?
  if (blockSize == 0U)
    throw std::logic_error("EEPROMSectionSystem::MountStep1: Cannot figure out suitable initial block size");

  storage.SetBlockSize(blockSize);

  // Allocate memory for loading the Section System Info Block and setup a pointer for accessing it.
  std::unique_ptr<char[]> spMem(new char[blockSize]);
  SectionSystemInfoBlock_t const * const pSSIB = static_cast<SectionSystemInfoBlock_t*>(static_cast<void*>(spMem.get()));

  // load and check Section System Info Block
  Mount_LoadAndCheckSecSysInfoBlock(spMem.get());

  // update block size if required
  if (blockSize != pSSIB->blockSize)
  {
    // reconfigure block-level access (this includes complete check of block size value)
    blockSize = pSSIB->blockSize;
    storage.SetBlockSize(blockSize);
  }

  // get number of blocks and cross-check with Section System Info Block
   uint16_t const nBlocks = storage.GetnBlocks();
   if (nBlocks != pSSIB->nBlocks)
      throw StorageSizeMismatchError();

   state = States::ro_mount;
}
void EEPROMSectionSystem::MountStep2(void)
/**
 * \brief Mounts the section system (second step).
 *
 * This mounts the Section System for read/write-access. Before calling this, the Section System must
 * have been mounted for at least read-only access (@ref MountStep1()).\n
 * The Section System must be in one of the following states:
 * - @ref States::ro_mount
 * - @ref States::mounted
 * - @ref States::defect
 *
 * There must be no sections open for reading or writing.
 *
 * If the operation succeeds, then the @ref EEPROMSectionSystem will be switched to @ref States::mounted. In case
 * of failure, the @ref EEPROMSectionSystem will enter state @ref States::defect.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * If an exception is thrown while `MountStep2()` repairs the section system, then the storage content could be left
 * in an invalid state that differs from the state before calling this. However, things cannot get worse than they
 * have been when `MountStep2()` was invoked. A subsequent call to `MountStep2()` is able to recover the section
 * system if the underlying storage is physically OK.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but the storage content could be left in an invalid state if the thread is
 * cancelled while `MountStep2()` repairs the section system. A subsequent call to `MountStep2()` is able to
 * recover the section system if the underlying storage is physically OK.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if ((state != States::ro_mount) &&
      (state != States::mounted) &&
      (state != States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::MountStep2", state);

  if (sectionLockManager.AnyLocks())
    throw NotAllSectionsClosedError();

  state = States::checking;

  // on failure, ensure that state will be switched to States::defect
  ON_SCOPE_EXIT(EnterDefectOnError)
  {
    state = States::defect;
  };

  nFreeBlocks           = 0;
  freeBlockListHeadIdx  = NOBLOCK;
  freeBlockListEndIdx   = NOBLOCK;

  // get storage properties
  uint16_t const nBlocks = storage.GetnBlocks();
  uint16_t const blockSize = storage.GetBlockSize();

  // allocate memory for one storage block and for a section name
  std::unique_ptr<char[]> spMem(new char[blockSize]);
  std::unique_ptr<char[]> spSecName(new char[storage.GetMaxSectionNameLength() + 1U]);

  // setup a pointer to access the common head of a block stored in spMem
  CommonBlockHead_t const * const pCommonHead  = static_cast<CommonBlockHead_t*>(static_cast<void*>(spMem.get()));

  // -------------------------------------------------------------------------------------
  // Step 1:
  // Check all blocks and mark them as either "used/unused" or "garbage". Free blocks
  // are added to the list of free blocks.
  //
  // Two fields are used to classify the blocks:
  // BfGarbageBlocks | BfUsedUnusedBlocks | Meaning
  // :-------------: | :----------------: | ----------------------
  // 0               | 0                  | not yet examined
  // 0               | 1                  | used/unused
  // 1               | 0                  | garbage
  // 1               | 1                  | examination in process / do not know yet
  // -------------------------------------------------------------------------------------
  container::BitField BfUsedUnusedBlocks(nBlocks);
  container::BitField BfGarbageBlocks(nBlocks);

  // block 0 is the Section System Info Block, mark it "used"
  BfUsedUnusedBlocks.SetBit(0);

  // loop through all other blocks
  for (uint16_t currIndex = 1U; currIndex < nBlocks; currIndex++)
  {
    // block already examined?
    if ((BfUsedUnusedBlocks.GetBit(currIndex)) || (BfGarbageBlocks.GetBit(currIndex)))
      continue;

    // load type-field
    BlockTypes const typeField = static_cast<BlockTypes>(storage.LoadField_type(currIndex));

    // load the complete block if it is a free block or a section head
    if ((typeField == BlockTypes::freeBlock) || (typeField == BlockTypes::sectionHead))
    {
      try
      {
        storage.LoadBlock(currIndex, spMem.get(), blockSize);
      }
      catch (DataIntegrityError const &)
      {
        // block is invalid, mark it as garbage and continue with next block
        BfGarbageBlocks.SetBit(currIndex);
        continue;
      }

      // Block type must not have changed. If it does, then it is a serious error.
      if (typeField != static_cast<BlockTypes>(pCommonHead->type))
        throw VolatileStorageError(currIndex);
    }

    // examine the block in detail
    switch (typeField)
    {
      case BlockTypes::sectionSystemInfo:
      {
        // There is only one Section System Info Block. The current block is garbage.
        BfGarbageBlocks.SetBit(currIndex);
        break;
      }

      case BlockTypes::freeBlock:
      {
        Mount_ProcessFreeBlock(currIndex, spMem.get(), BfUsedUnusedBlocks, BfGarbageBlocks);
        break;
      }

      case BlockTypes::sectionHead:
      {
        Mount_ProcessSectionHead(currIndex, spMem.get(), spSecName.get(), BfUsedUnusedBlocks, BfGarbageBlocks);
        break;
      }

      case BlockTypes::sectionData:
      {
        // Do not add this block to any list. Maybe it will be added later to a list when
        // a section head is checked. If this does not happen, then the block will finally
        // be collected as garbage.
        break;
      }

      default:
      {
        // Invalid block type value. -> garbage
        BfGarbageBlocks.SetBit(currIndex);
        break;
      }
    } // switch (typeField)
  } // for (uint16_t currIndex = 1U; currIndex < nBlocks; currIndex++)

  // spSecName is no longer needed
  spSecName.reset();

  // ---------------------------------------------------------------------------------------------------
  // Step 2: Make sure that the last block in the chain of free blocks found up to now refers to NOBLOCK
  // ---------------------------------------------------------------------------------------------------
  Mount_CheckLastFreeBlock(spMem.get());

  // ---------------------------
  // Step 3: collect any garbage
  // ---------------------------
  Mount_CollectGarbageBlocks(spMem.get(), BfUsedUnusedBlocks);

  // --------
  // finished
  // --------
  state = States::mounted;
  ON_SCOPE_EXIT_DISMISS(EnterDefectOnError);
}
void EEPROMSectionSystem::Unmount(void)
/**
 * \brief Unmounts the Section System.
 *
 * ---
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
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if ((state != States::ro_mount) &&
      (state != States::mounted) &&
      (state != States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::Unmount", state);

  if (sectionLockManager.AnyLocks())
    throw NotAllSectionsClosedError();

  state = States::not_mounted;
}

std::unique_ptr<Stream::IStreamReader> EEPROMSectionSystem::Open(std::string const & name)
/**
 * \brief Opens a section for reading.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * Be aware of the following exceptions:
 * - @ref NoSuchFileError
 * - any derived from std::exception
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param name
 * Name of the section that shall be opened.
 * \return
 * A std::unique_ptr to an @ref Stream::IStreamReader for reading from the opened section.\n
 * The calling function must finally close the @ref Stream::IStreamReader and release it.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (!CheckSectionName(name))
    throw std::invalid_argument("EEPROMSectionSystem::Open: Invalid name");

  if ((state != States::ro_mount) &&
      (state != States::mounted))
    throw InsufficientStateError("EEPROMSectionSystem::Open", state, States::ro_mount);

  if (!sectionLockManager.GetReadLock(name))
    throw FileAlreadyAccessedError(name);
  ON_SCOPE_EXIT(releaseReadLock)
  {
    sectionLockManager.ReleaseReadLock(name);
  };

  // allocate memory for reading storage blocks
  uint16_t const blockSize = storage.GetBlockSize();
  std::unique_ptr<unsigned char[]> spMem(new unsigned char[blockSize]);

  // load the section head into spMem
  if (state == States::mounted)
  {
    if (FindSectionHead(1, name.c_str(), CalcHash(name.c_str()), spMem.get()) == NOBLOCK)
      throw NoSuchFileError(name);
  }
  else
  {
    // In state ro_mount, MountStep2() has not yet been executed. We therefore have to check
    // for other section heads with the same section name but higher version. If there are no
    // other section heads with same name, then we have to check for section heads with different
    // name but same "nextBlock" attribute.

    uint16_t const nBlocks = storage.GetnBlocks();
    uint16_t sectionHeadIdx;
    uint16_t firstDataBlockIdx;
    uint16_t version;
    bool checkByNextBlockRequired = true;
    SectionHeadBlock_t const * const pHead = static_cast<SectionHeadBlock_t const *>(static_cast<void const*>(spMem.get()));

    uint8_t const hash = CalcHash(name.c_str());

    // locate section head
    sectionHeadIdx = FindSectionHead(1, name.c_str(), hash, spMem.get());
    if (sectionHeadIdx == NOBLOCK)
      throw NoSuchFileError(name);
    firstDataBlockIdx = pHead->head.nextBlock;
    version = pHead->version;

    // Check if there are any other section heads with same section name.
    // If so, choose the one with the highest version.
    uint16_t idx = sectionHeadIdx;
    while (idx != nBlocks - 1U)
    {
      idx = FindSectionHead(idx + 1U, name.c_str(), hash, spMem.get());
      if (idx == NOBLOCK)
        break;

      checkByNextBlockRequired = false;

      if (pHead->version == version)
      {
        state = States::defect;
        throw BlockLinkageError("EEPROMSectionSystem::Open: Found second section head (by name) with same version", idx);
      }
      else if (((pHead->version != std::numeric_limits<uint16_t>::max()) && (pHead->version > version)) ||
               ((pHead->version == 0) && (version == std::numeric_limits<uint16_t>::max())))
      {
        sectionHeadIdx    = idx;
        firstDataBlockIdx = pHead->head.nextBlock;
        version           = pHead->version;
      }
    }

    if (checkByNextBlockRequired)
    {
      // Check if there are any other section heads with the same "nextBlock" attribute.
      // If so, and if any other section head has a higher version, then the section
      // referenced by "name" has been renamed and is not existing.
      idx = 0;
      while (idx != nBlocks - 1U)
      {
        idx = FindSectionHeadByNextBlock(idx + 1U, firstDataBlockIdx, spMem.get());
        if (idx == NOBLOCK)
          break;

        if (idx == sectionHeadIdx)
          continue;

        if (pHead->version == version)
        {
          state = States::defect;
          throw BlockLinkageError("EEPROMSectionSystem::Open: Found second section head (by nextBlock) with same version", idx);
        }
        else if (((pHead->version != std::numeric_limits<uint16_t>::max()) && (pHead->version > version)) ||
                 ((pHead->version == 0) && (version == std::numeric_limits<uint16_t>::max())))
          throw NoSuchFileError(name);
      }
    }

    // finally load the located section head back into spMem
    storage.LoadBlock(sectionHeadIdx, spMem.get(), blockSize);
    if (static_cast<BlockTypes>(pHead->head.type) != BlockTypes::sectionHead)
      throw VolatileStorageError(sectionHeadIdx);
  }

  // create SectionReader instance
  std::unique_ptr<Stream::IStreamReader> spISR(new SectionReader(*this, name, std::move(spMem)));

  ON_SCOPE_EXIT_DISMISS(releaseReadLock);

  return spISR;
}
std::unique_ptr<Stream::IStreamWriter> EEPROMSectionSystem::Create(std::string const & name, bool const overwriteIfExisting)
/**
 * \brief Creates a section or overwrites an existing section.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - `state` is set to @ref States::defect, if the Section System is corrupted.
 * - apart from that there are no further side effects, so all data retain their original values.
 *
 * Be aware of the following exceptions:
 * - @ref FileAlreadyExistingError
 * - any derived from std::exception
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param name
 * Name of the section that shall be created or overwritten.
 * \param overwriteIfExisting
 * This determines the behavior if a section with the given name is already existing:
 * - `true` = overwrite
 * - `false` = do not overwrite (will throw @ref FileAlreadyExistingError)
 * \return
 * A std::unique_ptr to an @ref Stream::IStreamWriter for writing to the new section.\n
 * The calling function must finally close the @ref Stream::IStreamWriter and release it.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (!CheckSectionName(name))
    throw std::invalid_argument("EEPROMSectionSystem::Create: Invalid name");

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::Create", state, States::mounted);

  if (!sectionLockManager.GetWriteLock(name))
    throw FileAlreadyAccessedError(name);
  ON_SCOPE_EXIT(releaseWriteLock)
  {
    sectionLockManager.ReleaseWriteLock(name);
  };

  // allocate memory for reading and writing storage blocks
  uint16_t const blockSize = storage.GetBlockSize();
  std::unique_ptr<char[]> spMem(new char[blockSize]);

  // section already existing?
  uint16_t const oldSectionHeadIndex = FindSectionHead(1, name.c_str(), CalcHash(name.c_str()), spMem.get());

  // leave if section is already existing and if overwriting is disabled
  if ((!overwriteIfExisting) && (oldSectionHeadIndex != NOBLOCK))
    throw FileAlreadyExistingError(name);

  // get two free blocks (new section head plus one data block)
  auto const fbl_backup = GetFreeBlockListBackup();
  uint16_t freeBlocks[2];
  if (!GetBlocksFromListOfFreeBlocks(freeBlocks, 2, true))
    throw InsufficientSpaceError();
  ON_SCOPE_EXIT(releaseAllocatedFreeBlocks)
  {
    RewindFreeBlockLists(fbl_backup);
  };

  // determine version for the new section head
  uint16_t version;
  if (oldSectionHeadIndex == NOBLOCK)
    version = 1U;
  else
    version = static_cast<SectionHeadBlock_t const *>(static_cast<void const *>(spMem.get()))->version + 1U;

  // create SectionWriter instance
  std::unique_ptr<Stream::IStreamWriter> spISW(new SectionWriter(*this,
                                                                 name,
                                                                 oldSectionHeadIndex,
                                                                 freeBlocks[0],
                                                                 version,
                                                                 freeBlocks[1],
                                                                 std::move(spMem)));

  ON_SCOPE_EXIT_DISMISS(releaseAllocatedFreeBlocks);
  ON_SCOPE_EXIT_DISMISS(releaseWriteLock);

  return spISW;
}
void EEPROMSectionSystem::Delete(std::string const & name)
/**
 * \brief Deletes a section.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the storage blocks of the section that shall be deleted may be left in an undefined state
 *   (added to free block list or not).
 * - the Section System may be corrupted (free block list).
 * - `state` is set to @ref States::defect, if the Section System is corrupted or if the state
 *   of the blocks of the section that shall be deleted is undefined.
 *
 * Be aware of the following exceptions:
 * - @ref NoSuchFileError
 * - any derived from std::exception
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the storage blocks of the section that shall be deleted may be left in an undefined state
 *   (added to free block list or not).
 * - the Section System may be corrupted (free block list).
 * - `state` is set to @ref States::defect, if the Section System is corrupted or if the state
 *   of the blocks of the section that shall be deleted is undefined.
 *
 * ---
 *
 * \param name
 * Name of the section that shall be deleted.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (!CheckSectionName(name))
    throw std::invalid_argument("EEPROMSectionSystem::Delete: Invalid name");

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::Delete", state, States::mounted);

  if (sectionLockManager.IsLocked(name))
    throw FileAlreadyAccessedError(name);

  // allocate memory for reading and writing storage blocks
  uint16_t const blockSize = storage.GetBlockSize();
  std::unique_ptr<char[]> spMem(new char[blockSize]);

  // load section head
  uint16_t const sectionHeadIdx = FindSectionHead(1, name.c_str(), CalcHash(name.c_str()), spMem.get());
  if (sectionHeadIdx == NOBLOCK)
    throw NoSuchFileError(name);

  AddChainOfBlocksToListOfFreeBlocks(sectionHeadIdx, NOBLOCK, spMem.get(), true);
}
void EEPROMSectionSystem::Rename(std::string const & currName, std::string const & newName)
/**
 * \brief Renames an existing section.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the modifications done to the storage may not be completed:
 *   + the new section head may be incompletely written to the storage
 *   + the old section head may be not properly added to the list of free blocks
 * - the Section System may be corrupted (free block list).
 * - `state` is set to @ref States::defect, if the Section System is corrupted.
 *
 * Be aware of the following exceptions:
 * - @ref NoSuchFileError
 * - @ref FileAlreadyExistingError
 * - any derived from std::exception
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the modifications done to the storage may not be completed:
 *   + the new section head may be incompletely written to the storage
 *   + the old section head may be not properly added to the list of free blocks
 * - the Section System may be corrupted (free block list).
 * - `state` is set to @ref States::defect, if the Section System is corrupted.
 *
 * ---
 *
 * \param currName
 * Name of the section that shall be renamed.
 * \param newName
 * New name for the section.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if ((!CheckSectionName(currName)) ||
      (!CheckSectionName(newName)))
    throw std::invalid_argument("EEPROMSectionSystem::Rename: Invalid currName/newName");

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::Rename", state, States::mounted);

  if (sectionLockManager.IsLocked(currName))
    throw FileAlreadyAccessedError(currName);

  if (sectionLockManager.IsLocked(newName))
    throw FileAlreadyAccessedError(newName);

  // allocate memory for reading and writing storage blocks
  uint16_t const blockSize = storage.GetBlockSize();
  std::unique_ptr<char[]> spMem(new char[blockSize]);

  // check whether a section with the new name is already existing
  if (FindSectionHead(1, newName.c_str(), CalcHash(newName.c_str()), spMem.get()) != NOBLOCK)
    throw FileAlreadyExistingError(newName);

  // locate the section that shall be renamed
  uint16_t const sectionHeadIdx = FindSectionHead(1, currName.c_str(), CalcHash(currName.c_str()), spMem.get());
  if (sectionHeadIdx == NOBLOCK)
    throw NoSuchFileError(currName);

  // (OK, section has been found and there is no section with the same name as the new name yet)

  SectionHeadBlock_t* const pHead = static_cast<SectionHeadBlock_t*>(static_cast<void*>(spMem.get()));
  uint32_t const totalNbOfWritesOldSectionHead = pHead->head.totalNbOfWrites;

  // build a new head from the current one
  pHead->head.sectionNameHash = CalcHash(newName.c_str());
  pHead->head.nBytes          = sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + newName.length() + 1U;
  pHead->version++;
  memcpy(spMem.get() + sizeof(SectionHeadBlock_t), newName.c_str(), newName.length() + 1U);

  // get a free block for the new section head
  uint32_t tnow = pHead->head.totalNbOfWrites; // <- avoid warning about potentially unaligned pointer
  uint16_t const newSectionHeadIdx = GetBlockFromListOfFreeBlocks(&tnow, true);
  pHead->head.totalNbOfWrites = tnow;

  if (newSectionHeadIdx == NOBLOCK)
    throw InsufficientSpaceError();

  ON_SCOPE_EXIT(switchToDefect) { state = States::defect; };

  storage.StoreBlock(newSectionHeadIdx, spMem.get(), nullptr, false);

  AddBlockToListOfFreeBlocks(sectionHeadIdx, &totalNbOfWritesOldSectionHead, true);

  ON_SCOPE_EXIT_DISMISS(switchToDefect);
}

std::list<std::string> EEPROMSectionSystem::Enumerate(void) const
/**
 * \brief Enumerates all sections.
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
 * List containing the names of all currently existing sections, sorted alphabetically and by upper/lower-case.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::Enumerate", state, States::mounted);

  // allocate memory for reading and writing storage blocks
  uint16_t const blockSize = storage.GetBlockSize();
  std::unique_ptr<char[]> spMem(new char[blockSize]);

  // enumerate all section heads in "list"
  std::list<std::string> list;
  uint16_t const nBlocks = storage.GetnBlocks();
  uint16_t idx = 0;
  while (idx != nBlocks - 1U)
  {
    idx = FindAnySectionHead(idx + 1U, spMem.get());
    if (idx == NOBLOCK)
      break;

    list.push_back(std::string(spMem.get() + sizeof(SectionHeadBlock_t)));
  }

  // sort alphabetically
  list.sort();

  return list;
}
size_t EEPROMSectionSystem::DetermineSize(std::string const & name, size_t * const pTotalSize) const
/**
 * \brief Determines the size of a section.
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
 * \param name
 * Name of the section.
 * \param pTotalSize
 * Pointer to a memory location into which the total number of bytes occupied inside the underlying
 * storage shall be written.\n
 * nullptr, if the caller is not interested in this value.
 * \return
 * Number of data bytes stored inside the section.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (!CheckSectionName(name))
    throw std::invalid_argument("EEPROMSectionSystem::DetermineSize: Invalid name");

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::DetermineSize", state, States::mounted);

  if (!sectionLockManager.GetReadLock(name))
    throw FileAlreadyAccessedError(name);
  ON_SCOPE_EXIT(releaseReadLock)
  {
    sectionLockManager.ReleaseReadLock(name);
  };

  // allocate memory for reading and writing storage blocks
  uint16_t const blockSize = storage.GetBlockSize();
  std::unique_ptr<char[]> spMem(new char[blockSize]);

  uint16_t currIdx = FindSectionHead(1U, name.c_str(), CalcHash(name.c_str()), spMem.get());
  if (currIdx == NOBLOCK)
    throw NoSuchFileError(name);

  size_t dataSize = 0;
  size_t totalSize = blockSize;

  // walk through all blocks of data
  uint16_t maxCycles = storage.GetnBlocks() - 1U;
  while (1)
  {
    // endless loop?
    if (maxCycles-- == 0)
      throw BlockLinkageError("EEPROMSectionSystem::DetermineSize: Loop limit", currIdx);

    currIdx = LoadNextBlockOfSection(spMem.get());

    if (currIdx == NOBLOCK)
      break;

    dataSize += static_cast<DataBlock_t const *>(static_cast<void const*>(spMem.get()))->head.nBytes - (sizeof(DataBlock_t) + sizeof(uint16_t));
    totalSize += blockSize;
  }

  if (pTotalSize != nullptr)
    *pTotalSize = totalSize;
  return dataSize;
}
size_t EEPROMSectionSystem::GetFreeSpace(void) const
/**
 * \brief Retrieves the amount of free space available for data.
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
 * Amount of free space available for data in bytes, as it would be after creation of a new EEPROM section.
 */
{
  osal::MutexLocker mutexLocker(mutex);

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::GetFreeSpace", state, States::mounted);

  if (nFreeBlocks <= 1U)
    return 0;
  else
    return static_cast<size_t>(nFreeBlocks - 1U) * (storage.GetBlockSize() - (sizeof(DataBlock_t) + sizeof(uint16_t)));
}

void EEPROMSectionSystem::Mount_LoadAndCheckSecSysInfoBlock(void* const pMem) const
/**
 * \brief One part of the @ref MountStep1() operation.
 *
 * Loads the Section System Info Block, checks its CRC and checks consistency of the content. The block size
 * and the number of blocks are not checked against the properties of the underlying storage. These tests must
 * be done by the caller.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the buffer referenced by `pMem` contains undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the buffer referenced by `pMem` contains undefined data.
 *
 * ---
 *
 * \param pMem
 * Pointer to a buffer into which the Section System Info Block shall be loaded.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 */
{
  // load section system info block
  try
  {
    storage.LoadBlock(0, pMem, storage.GetBlockSize());
  }
  catch (DataIntegrityError const &)
  {
    throw BadSectionSystemInfoBlockError();
  }

  // setup pointer to access section system info block
  SectionSystemInfoBlock_t const * const pSSIB = static_cast<SectionSystemInfoBlock_t*>(pMem);

  // check type
  if (static_cast<BlockTypes>(pSSIB->head.type) != BlockTypes::sectionSystemInfo)
    throw BadSectionSystemInfoBlockError();

  // checks already done by storage.LoadBlock on common header:
  // - sectionNameHash
  // - nBytes
  // - nextBlock

  // check EEPROMSectionSystem version compatibility
  if (pSSIB->sectionSystemVersion != Version)
    throw InvalidVersionError();

  // check EEPROMSectionSystem block size and number of blocks
  if ((pSSIB->blockSize < MinimumBlockSize) ||
      (pSSIB->blockSize > MaximumBlockSize) ||
      (pSSIB->nBlocks < MinimumNbOfBlocks))
    throw InvalidHeaderError("Bad \"blockSize\" or \"nBlocks\" in Info Block", 0);
}
void EEPROMSectionSystem::Mount_ProcessFreeBlock(uint16_t const currIndex, void* const pMem, container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks)
/**
 * \brief One part of the @ref MountStep2() operation.
 *
 * Looks at a free block and adds it and adjacent free blocks (-> nextBlock-attribute) to either
 * `BfUsedUnusedBlocks` and to the single linked list of free blocks, or it adds the block and adjacent free
 * blocks to `BfGarbageBlocks`.
 *
 * BfGarbageBlocks | BfUsedUnusedBlocks | Meaning
 * :-------------: | :----------------: | ----------------------------------------
 * 0               | 0                  | not yet examined
 * 0               | 1                  | used/unused
 * 1               | 0                  | garbage
 * 1               | 1                  | examination in process / do not know yet
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - `BfUsedUnusedBlocks` and `BfGarbageBlocks` are left in an undefined state.
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - `BfUsedUnusedBlocks` and `BfGarbageBlocks` are left in an undefined state.
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * ---
 *
 * \param currIndex
 * Index of the free block that shall be examined.
 * \param pMem
 * Pointer to a memory block. It must contain the block referenced by `currIndex`.\n
 * The memory will be used by this method as a buffer for loading further blocks.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 * \param BfUsedUnusedBlocks
 * Reference to BitField `BfUsedUnusedBlocks` used in @ref MountStep2().
 * \param BfGarbageBlocks
 * Reference to BitField `BfGarbageBlocks` used in @ref MountStep2().
 */
{
  // loop through the chain of free blocks
  uint_fast16_t nbOfLocatedFreeBlocks = 0;
  uint16_t index = currIndex;
  uint16_t lastIndex;
  do
  {
    // block already examined?
    if (BfUsedUnusedBlocks.GetBit(index) || BfGarbageBlocks.GetBit(index))
    {
      Mount_SetDNKYtoGarbage(BfUsedUnusedBlocks, BfGarbageBlocks);
      return;
    }

    nbOfLocatedFreeBlocks++;

    // mark current block as Do-Not-Know-Yet
    BfUsedUnusedBlocks.SetBit(index);
    BfGarbageBlocks.SetBit(index);

    // keep current index in mind
    lastIndex = index;

    // proceed to the next block
    try
    {
      index = LoadNextFreeBlock(pMem);
    }
    catch (DataIntegrityError const &)
    {
      // (this only happens if the next block is not a free block or if it is defect)
      break;
    }
  }
  while ((index != NOBLOCK) && (index != freeBlockListHeadIdx));

  // (found "nbOflocatedFreeBlocks" free blocks)

  ON_SCOPE_EXIT(markDefect) { state = States::defect; };

  // first free block(s) ever detected?
  if (nFreeBlocks == 0)
  {
    nFreeBlocks          = nbOfLocatedFreeBlocks;
    freeBlockListHeadIdx = currIndex;
    freeBlockListEndIdx  = lastIndex;

    Mount_SetDNKYtoUsed(BfUsedUnusedBlocks, BfGarbageBlocks);
  }
  // ...or is the block or chain of blocks sitting in front of the first block
  // in the single linked list of free blocks?
  else if (index == freeBlockListHeadIdx)
  {
    // extend existing single linked list of free blocks at the beginning
    nFreeBlocks          += nbOfLocatedFreeBlocks;
    freeBlockListHeadIdx = currIndex;

    Mount_SetDNKYtoUsed(BfUsedUnusedBlocks, BfGarbageBlocks);
  }
  // ...else the block or the chain of blocks is not part of the already existing list of free blocks
  else
  {
    // ...therefore it is garbage
    Mount_SetDNKYtoGarbage(BfUsedUnusedBlocks, BfGarbageBlocks);
  }

  ON_SCOPE_EXIT_DISMISS(markDefect);
}
void EEPROMSectionSystem::Mount_ProcessSectionHead(uint16_t currIndex, void* const pMem, char* const pSecName, container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks) const
/**
 * \brief One part of the @ref MountStep2() operation.
 *
 * Looks at a Section Head Block and adds it and potential data blocks to either `BfUsedUnusedBlocks`,
 * or to `BfGarbageBlocks`. The method also checks whether there are older or newer versions of
 * the section head (with the same or a different name) and marks any old version(s) of the section head
 * as garbage.
 *
 * BfGarbageBlocks | BfUsedUnusedBlocks | Meaning
 * :-------------: | :----------------: | ----------------------------------------
 * 0               | 0                  | not yet examined
 * 0               | 1                  | used/unused
 * 1               | 0                  | garbage
 * 1               | 1                  | examination in process / do not know yet
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - `BfUsedUnusedBlocks` and `BfGarbageBlocks` are undefined.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - `BfUsedUnusedBlocks` and `BfGarbageBlocks` are undefined.
 *
 * ---
 *
 * \param currIndex
 * Index of the section head that shall be examined.\n
 * The section head must not be marked as "used", as "garbage", or as "DNKY" yet.
 * \param pMem
 * Pointer to a buffer. It must contain the block referenced by `currIndex`.\n
 * The buffer will be used by this method as a buffer for loading further blocks.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 * \param pSecName
 * Pointer to a memory block that can be used by this method as a buffer for section names.
 * The referenced memory block must have a sufficient capacity to store a section name
 * inclusive null-terminator (storage.MaxSectionNameLength() + 1U).
 * \param BfUsedUnusedBlocks
 * Reference to BitField `BfUsedUnusedBlocks` used in @ref MountStep2().
 * \param BfGarbageBlocks
 * Reference to BitField `BfGarbageBlocks` used in @ref MountStep2().
 */
{
  SectionHeadBlock_t const * const pHead = static_cast<SectionHeadBlock_t*>(pMem);

  uint16_t const nBlocks = storage.GetnBlocks();

  // --------------------------------------------------
  // Extract section name, hash and nextBlock-attribute
  // --------------------------------------------------
  // Note: storage.LoadBlock() called in MountStep2() has checked that the null-terminator is present
  // and that pHead->head.nBytes is OK.
  memcpy(pSecName,
         static_cast<char const*>(pMem) + sizeof(SectionHeadBlock_t),
         pHead->head.nBytes - (sizeof(SectionHeadBlock_t) + sizeof(uint16_t)));
  uint8_t  const hash      = pHead->head.sectionNameHash;
  uint16_t const nextBlock = pHead->head.nextBlock;

  // ------------------------------------------------------------------------
  // Check if there are other section heads with the same nextBlock-attribute
  // ------------------------------------------------------------------------
  bool needToCheckForSectionsWithSameName = true;

  uint16_t currVersion = pHead->version;

  // start looking behind the the current block
  uint16_t index = currIndex + 1U;
  while (index != nBlocks)
  {
    // look for a valid section head with specific nextBlock-attribute
    try
    {
      index = FindSectionHeadByNextBlock(index, nextBlock, pMem);
    }
    catch (DataIntegrityError const & e)
    {
      // continue at the block where the error occurred
      index = e.GetBlockIndex();

      // block successfully loaded before?
      if (BfUsedUnusedBlocks.GetBit(index))
        throw VolatileStorageError(index);

      // block is garbage
      BfGarbageBlocks.SetBit(index);

      // go on
      index++;
      continue;
    }

    // nothing found?
    if (index == NOBLOCK)
      break;

    // block already examined? -> not possible -> storage has changed somehow
    if (BfUsedUnusedBlocks.GetBit(index) || BfGarbageBlocks.GetBit(index))
      throw VolatileStorageError(index);

    // check for section heads with same name is not required any more
    needToCheckForSectionsWithSameName = false;

    // same version? -> serious error
    if (pHead->version == currVersion)
      throw BlockLinkageError("Found second section head (by nextBlock) with same version", index);

    // compare versions
    if (((pHead->version != std::numeric_limits<uint16_t>::max()) && (pHead->version > currVersion)) ||
        ((pHead->version == 0) && (currVersion == std::numeric_limits<uint16_t>::max())))
    {
      // The block just found is newer. Mark the block referenced by currIndex as garbage.
      BfGarbageBlocks.SetBit(currIndex);

      // The block just found is the actual block now.
      currIndex   = index;
      currVersion = pHead->version;
    }
    else
    {
      // The block just found is older. Mark it as garbage.
      BfGarbageBlocks.SetBit(index);
    }

    index++;
  }

  // -------------------------------------------------------------------------------------------
  // Check if there are other section heads with the same name.
  //
  // This is only executed, if there was no other section head with the same nextBlock-attribute
  // located before. Skipping this is allowed because either a rename- or overwrite operation on
  // the section system can be incomplete at any time, but never both.
  // -------------------------------------------------------------------------------------------
  if (needToCheckForSectionsWithSameName)
  {
    // start looking behind the the current block
    index = currIndex + 1U;
    while (index != nBlocks)
    {
      // look for a section head with the same name
      try
      {
        index = FindSectionHead(index, pSecName, hash, pMem);
      }
      catch (DataIntegrityError const & e)
      {
        // continue at the block where the error occurred
        index = e.GetBlockIndex();

        // block successfully loaded before?
        if (BfUsedUnusedBlocks.GetBit(index))
          throw VolatileStorageError(index);

        // block is garbage
        BfGarbageBlocks.SetBit(index);

        // go on
        index++;
        continue;
      }

      // nothing found?
      if (index == NOBLOCK)
        break;

      // block already examined? -> not possible -> storage has changed somehow
      if (BfUsedUnusedBlocks.GetBit(index) || BfGarbageBlocks.GetBit(index))
        throw VolatileStorageError(index);

      // same version? -> serious error
      if (pHead->version == currVersion)
        throw BlockLinkageError("Found second section head (by name) with same version", index);

      // compare versions
      if (((pHead->version != std::numeric_limits<uint16_t>::max()) && (pHead->version > currVersion)) ||
          ((pHead->version == 0) && (currVersion == std::numeric_limits<uint16_t>::max())))
      {
        // The block just found is newer. Mark the block referenced by currIndex as garbage.
        BfGarbageBlocks.SetBit(currIndex);

        // The block just found is the actual block now.
        currIndex   = index;
        currVersion = pHead->version;
      }
      else
      {
        // The block just found is older. Mark it as garbage.
        BfGarbageBlocks.SetBit(index);
      }

      index++;
    } // while (index != nBlocks)
  } // if (needToCheckForSectionsWithSameName)

  // -------------------------------------------------------
  // Examine section referenced by currIndex and currVersion
  // -------------------------------------------------------
  // reload section
  try
  {
    storage.LoadBlock(currIndex, pMem, storage.GetBlockSize());
  }
  catch (DataIntegrityError const &)
  {
    // block has been loaded before, loading should have worked a second time
    throw VolatileStorageError(currIndex);
  }

  // Check block type. Block type must still be BlockTypes::sectionHead.
  if (static_cast<BlockTypes>(pHead->head.type) != BlockTypes::sectionHead)
    throw VolatileStorageError(currIndex);

  // check section name
  std::string secNameStr(static_cast<char const *>(pMem) + sizeof(SectionHeadBlock_t));
  if (!CheckSectionName(secNameStr))
    throw InvalidHeaderError("Bad section name", currIndex);

  // loop through the blocks of the section
  index = currIndex;
  do
  {
    // block already examined?
    if (BfUsedUnusedBlocks.GetBit(index) || BfGarbageBlocks.GetBit(index))
    {
      // Section is buggy, mark all blocks examined up to now as garbage
      Mount_SetDNKYtoGarbage(BfUsedUnusedBlocks, BfGarbageBlocks);
      return;
    }

    // mark current block as Do-Not-Know-Yet
    BfUsedUnusedBlocks.SetBit(index);
    BfGarbageBlocks.SetBit(index);

    // proceed to next block
    try
    {
      index = LoadNextBlockOfSection(pMem);
    }
    catch (BlockLinkageError const &)
    {
      // Section is buggy, mark all blocks examined up to now as garbage
      Mount_SetDNKYtoGarbage(BfUsedUnusedBlocks, BfGarbageBlocks);
      return;
    }
    catch (DataIntegrityError const & e)
    {
      // examine buggy block
      uint16_t const buggyIndex = e.GetBlockIndex();

      // block successfully loaded before?
      if (BfUsedUnusedBlocks.GetBit(buggyIndex))
        throw VolatileStorageError(buggyIndex);

      // block is garbage
      BfGarbageBlocks.SetBit(buggyIndex);

      // Section is buggy, mark all blocks examined up to now as garbage
      Mount_SetDNKYtoGarbage(BfUsedUnusedBlocks, BfGarbageBlocks);
      return;
    }
  }
  while (index != NOBLOCK);

  // OK
  Mount_SetDNKYtoUsed(BfUsedUnusedBlocks, BfGarbageBlocks);
}
void EEPROMSectionSystem::Mount_CheckLastFreeBlock(void* const pMem)
/**
 * \brief One part of the @ref MountStep2() operation.
 *
 * Makes sure that the nextBlock-attribute of the last block in the single linked list of
 * free blocks is NOBLOCK. If required, then this method updates the storage content.
 * If the single linked list of free blocks is empty, then this method does nothing.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * ---
 *
 * \param pMem
 * Pointer to a memory block that can be used by this method as an buffer for internal stuff.\n
 * The referenced memory must have a capacity of at least the block size of the underlying @ref storage.
 */
{
  // single linked list of free blocks not empty?
  if (nFreeBlocks != 0)
  {
    // load last block in the chain of free blocks
    storage.LoadBlock(freeBlockListEndIdx, pMem, storage.GetBlockSize());
    CommonBlockHead_t* const pCommonHead = static_cast<CommonBlockHead_t*>(pMem);

    if (static_cast<BlockTypes>(pCommonHead->type) != BlockTypes::freeBlock)
      throw BlockLinkageError("EEPROMSectionSystem::Mount_CheckLastFreeBlock: Last free block has unexpected type", freeBlockListEndIdx);

    // If the loaded block's nextBlock-attribute does not refer to NOBLOCK, then
    // set it to NOBLOCK and write the block back to the storage.
    if (pCommonHead->nextBlock != NOBLOCK)
    {
      ON_SCOPE_EXIT(markDefect) { state = States::defect; };
      pCommonHead->nextBlock = NOBLOCK;
      storage.StoreBlock(freeBlockListEndIdx, pMem, nullptr, false);
      ON_SCOPE_EXIT_DISMISS(markDefect);
    }
  }
}
void EEPROMSectionSystem::Mount_CollectGarbageBlocks(void* const pMem, container::BitField const & BfUsedUnusedBlocks)
/**
 * \brief One part of the MountStep2() operation.
 *
 * Adds all blocks which are not marked as "used/unused" to the single linked list of free blocks.
 * This includes blocks marked as garbage and blocks which have never been examined
 * (e.g. BlockTypes::sectionData).
 *
 * @ref Mount_CheckLastFreeBlock() must have been executed before.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect (always).
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect (always).
 *
 * ---
 *
 * \param pMem
 * Pointer to the memory block used in @ref MountStep2(). \n
 * The referenced memory must have a capacity of at least the block size of the underlying @ref storage. \n
 * The address must be aligned to a 16bit boundary.\n
 * The memory will be used as temporary storage for internal purposes of this method.
 * \param BfUsedUnusedBlocks
 * Reference to BitField `BfUsedUnusedBlocks` used in @ref MountStep2(). \n
 * Any asserted bit indicates that the block corresponding to the bit is "used/unused".
 */
{
  // recycle pMem as garbage list
  uint16_t* const pGarbageList = static_cast<uint16_t*>(pMem);
  size_t const garbageListSize = storage.GetBlockSize() / sizeof(uint16_t);
  size_t nbOfCollectedGarbageBlocks = 0;

  ON_SCOPE_EXIT(markDefect) { state = States::defect; };

  // add all blocks that are not marked in BfUsedUnusedBlocks to the single linked list of free blocks
  size_t currIndex = BfUsedUnusedBlocks.FindFirstClearedBit(0);
  while (currIndex != container::BitField::NO_BIT)
  {
    pGarbageList[nbOfCollectedGarbageBlocks++] = static_cast<uint16_t>(currIndex);

    // garbage list full?
    if (nbOfCollectedGarbageBlocks == garbageListSize)
    {
      AddBlocksToListOfFreeBlocks(pGarbageList, nbOfCollectedGarbageBlocks, true);
      nbOfCollectedGarbageBlocks = 0;
    }

    currIndex = BfUsedUnusedBlocks.FindFirstClearedBit(currIndex + 1U);
  }

  // process potential rest of garbage list
  AddBlocksToListOfFreeBlocks(pGarbageList, nbOfCollectedGarbageBlocks, true);

  ON_SCOPE_EXIT_DISMISS(markDefect);
}
void EEPROMSectionSystem::Mount_SetDNKYtoUsed(container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks) const
/**
 * \brief Helper method for mounting. Sets all examination-in-process / Do-Not-Know-Yet bits to "used/unused".
 *
 * BfGarbageBlocks | BfUsedUnusedBlocks | Meaning                | Action taken by this method
 * :-------------: | :----------------: | ---------------------- | --------------------------------------
 * 0               | 0                  | not yet examined       | none
 * 0               | 1                  | used/unused            | none
 * 1               | 0                  | garbage                | none
 * 1               | 1                  | examination in process | BfGarbageBlocks cleared -> used/unused
 *
 * ---
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - `BfGarbageBlocks` may be left in an inconsistent state.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param BfUsedUnusedBlocks
 * Bit field indicating which blocks are used or unused.
 * \param BfGarbageBlocks
 * Bit field indicating which blocks are garbage.
 */
{
  // get access to internal data of bit fields
  size_t nElementsBf0;
  container::BitField::storage_t* const pIntBf0 = BfUsedUnusedBlocks.GetInternalStorage(&nElementsBf0);
  size_t nElementsBf1;
  container::BitField::storage_t* const pIntBf1 = BfGarbageBlocks.GetInternalStorage(&nElementsBf1);

  if (nElementsBf0 != nElementsBf1)
    throw std::logic_error("EEPROMSectionSystem::Mount_SetDNKYtoUsed: Bitfields differ in length");

  for (size_t i = 0; i < nElementsBf0; i++)
    pIntBf1[i] = pIntBf1[i] & static_cast<container::BitField::storage_t>(~(pIntBf0[i] & pIntBf1[i]));
}
void EEPROMSectionSystem::Mount_SetDNKYtoGarbage(container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks) const
/**
 * \brief Helper method for mounting. Sets all examination-in-process / Do-Not-Know-Yet bits to "garbage".
 *
 * BfGarbageBlocks | BfUsedUnusedBlocks | Meaning                | Action taken by this method
 * :-------------: | :----------------: | ---------------------- | --------------------------------------
 * 0               | 0                  | not yet examined       | none
 * 0               | 1                  | used/unused            | none
 * 1               | 0                  | garbage                | none
 * 1               | 1                  | examination in process | BfUsedUnusedBlocks cleared -> garbage
 *
 * ---
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - `BfUsedUnusedBlocks` may be left in an inconsistent state.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param BfUsedUnusedBlocks
 * Bit field indicating which blocks are used or unused.
 * \param BfGarbageBlocks
 * Bit field indicating which blocks are garbage.
 */
{
  // get access to internal data of bit fields
  size_t nElementsBf0;
  container::BitField::storage_t* const pIntBf0 = BfUsedUnusedBlocks.GetInternalStorage(&nElementsBf0);
  size_t nElementsBf1;
  container::BitField::storage_t* const pIntBf1 = BfGarbageBlocks.GetInternalStorage(&nElementsBf1);

  if (nElementsBf0 != nElementsBf1)
    throw std::logic_error("EEPROMSectionSystem::Mount_SetDNKYtoGarbage: Bitfields differ in length");

  for (size_t i = 0; i < nElementsBf0; i++)
    pIntBf0[i] = pIntBf0[i] & static_cast<container::BitField::storage_t>(~(pIntBf0[i] & pIntBf1[i]));
}

bool EEPROMSectionSystem::CheckSectionName(std::string const & s) const
/**
 * \brief Examines if a given string is able to be used as a valid section name.
 *
 * The following checks are done:
 * - minimum length: 1 char
 * - maximum length: must fit into a Section Head Block
 * - no leading and trailing spaces
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
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
 * \param s
 * Reference to the string that shall be checked.
 * \return
 * Result of the check:\n
 * true  = The string is suitable to be used as a section name\n
 * false = The string is not suitable to be used as a section name
 */
{
  if (state == States::not_mounted)
    throw InsufficientStateError("EEPROMSectionSystem::CheckSectionName", state, States::ro_mount);

  // too short / too long?
  if ((s.length() == 0) || (s.length() > storage.GetMaxSectionNameLength()))
    return false;

  // any leading or trailing spaces?
  if ((s.find_first_of(' ') == 0) || (s.find_last_of(' ') == s.length() - 1U))
    return false;

  // OK
  return true;
}
uint16_t EEPROMSectionSystem::LoadNextBlockOfSection(void* const pMem) const
/**
 * \brief Loads the next block of a section.
 *
 * This includes CRC and consistency checks.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * ---
 *
 * \param pMem
 * Pointer to a buffer containing the current block. This must be either a section head
 * (BlockTypes::sectionHead) or a data block (BlockTypes::sectionData).\n
 * If there is a next block, then it will be loaded from the @ref storage into the referenced buffer.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 * \return
 * Block index of the next block.\n
 * NOBLOCK, if there is no next block.
 */
{
  if ((state == States::not_mounted) || (state == States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::LoadNextBlockOfSection", state, States::ro_mount);

  // Pointers to access headers in pMem
  CommonBlockHead_t const * const pCommonHead = static_cast<CommonBlockHead_t*>(pMem);
  DataBlock_t const * const pDataBlock = static_cast<DataBlock_t*>(pMem);

  // check type of current block and determine the expected sequence number
  uint16_t expectedSeqNb;
  switch (static_cast<BlockTypes>(pCommonHead->type))
  {
    case BlockTypes::sectionHead:
      expectedSeqNb = 1U;
      break;

    case BlockTypes::sectionData:
      expectedSeqNb = pDataBlock->seqNb + 1U;
      break;

    default:
      throw std::invalid_argument("EEPROMSectionSystem::LoadNextBlockOfSection: pMem does not contain a section head or data block");
  }

  // load and check next block (if any)
  uint16_t const nextBlockIndex = pCommonHead->nextBlock;
  if (nextBlockIndex != NOBLOCK)
  {
    storage.LoadBlock(nextBlockIndex, pMem, storage.GetBlockSize());

    if (static_cast<BlockTypes>(pCommonHead->type) != BlockTypes::sectionData)
      throw BlockLinkageError("EEPROMSectionSystem::LoadNextBlockOfSection: Block type should have been \"sectionData\"", nextBlockIndex);

    if (pDataBlock->seqNb != expectedSeqNb)
      throw BlockLinkageError("EEPROMSectionSystem::LoadNextBlockOfSection: Invalid sequence number", nextBlockIndex);
  }

  return nextBlockIndex;
}
uint16_t EEPROMSectionSystem::FindAnySectionHead(uint16_t const startBlockIndex, void* const pMem) const
/**
 * \brief Searches for any section head (BlockTypes::sectionHead) starting at an specific block index and
 * loads the first located section head.
 *
 * This includes CRC and consistency checks.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * ---
 *
 * \param startBlockIndex
 * Index where the search shall start at.
 * \param pMem
 * Pointer to a buffer into which the section head shall be loaded.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 * \return
 * Index of the section head.\n
 * NOBLOCK, if no section head is found.
 */
{
  if ((state == States::not_mounted) || (state == States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::FindAnySectionHead", state, States::ro_mount);

  uint16_t const nBlocks = storage.GetnBlocks();

  if (startBlockIndex >= nBlocks)
    throw std::invalid_argument("EEPROMSectionSystem::FindAnySectionHead: startBlockIndex invalid");

  SectionHeadBlock_t const * const pHead = static_cast<SectionHeadBlock_t const *>(pMem);
  for (uint16_t blockIndex = startBlockIndex; blockIndex < nBlocks; blockIndex++)
  {
    // load type-field and check for section head
    uint8_t const type = storage.LoadField_type(blockIndex);
    if (static_cast<BlockTypes>(type) == BlockTypes::sectionHead)
    {
      // load block, double check type and finish
      storage.LoadBlock(blockIndex, pMem, storage.GetBlockSize());
      if (static_cast<BlockTypes>(pHead->head.type) != BlockTypes::sectionHead)
        throw VolatileStorageError(blockIndex);

      return blockIndex;
    }
  }

  // found no section
  return NOBLOCK;
}
uint16_t EEPROMSectionSystem::FindSectionHead(uint16_t const startBlockIndex, char const * const name, uint8_t const hash, void* const pMem) const
/**
 * \brief Searches for a section head (BlockTypes::sectionHead) with a specific name and
 * loads the matching section head.
 *
 * This includes CRC and consistency checks.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * ---
 *
 * \param startBlockIndex
 * Index where the search shall start at.
 * \param name
 * Null-terminated c-string with the name of the section.
 * \param hash
 * Hash of the section's name.
 * \param pMem
 * Pointer to a buffer into which the section head shall be loaded in case of a match.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage. \n
 * Note that even in case of no match the buffer may have been modified by this method.
 * \return
 * Index of the matching section head.\n
 * NOBLOCK in case of no match.
 */
{
  if ((state == States::not_mounted) || (state == States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::FindSectionHead", state, States::ro_mount);

  uint16_t const nBlocks = storage.GetnBlocks();

  if (startBlockIndex >= nBlocks)
    throw std::invalid_argument("EEPROMSectionSystem::FindSectionHead: startBlockIndex invalid");

  if (name == nullptr)
    throw std::invalid_argument("EEPROMSectionSystem::FindSectionHead: !name");

  SectionHeadBlock_t const * const pHead = static_cast<SectionHeadBlock_t*>(pMem);
  uint16_t blockIndex = startBlockIndex;
  do
  {
    // look for a potential match by hash
    blockIndex = FindSectionHeadByHash(blockIndex, hash);

    if (blockIndex == NOBLOCK)
      return NOBLOCK;

    // load complete block and check for match by name
    storage.LoadBlock(blockIndex, pMem, storage.GetBlockSize());

    if (static_cast<BlockTypes>(pHead->head.type) != BlockTypes::sectionHead)
      throw VolatileStorageError(blockIndex);

    if (strcmp(name, static_cast<char const*>(pMem) + sizeof(SectionHeadBlock_t)) == 0)
      return blockIndex;

    // still here? no match!
    blockIndex++;
  }
  while (blockIndex != nBlocks);

  // finished, no match
  return NOBLOCK;
}
uint16_t EEPROMSectionSystem::FindSectionHeadByHash(uint16_t const startBlockIndex, uint8_t const hash) const
/**
 * \brief Searches for a section head (BlockTypes::sectionHead) with a specific section name hash.
 *
 * There are no CRC checks. In case of a match, the calling function must load the whole block
 * and check the CRC and the exact name of the section to verify the match.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
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
 * \param startBlockIndex
 * Index where the search shall start at.
 * \param hash
 * Value of the section name hash we are looking for.
 * \return
 * Index of the first section head whose section name hash matches.\n
 * NOBLOCK in case of no match.
 */
{
  if ((state == States::not_mounted) || (state == States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::FindSectionHeadByHash", state, States::ro_mount);

  uint16_t const nBlocks = storage.GetnBlocks();

  if (startBlockIndex >= nBlocks)
    throw std::invalid_argument("EEPROMSectionSystem::FindSectionHeadByHash: startBlockIndex invalid");

  uint16_t const searchValue = static_cast<uint16_t>(BlockTypes::sectionHead) | (static_cast<uint16_t>(hash) << 8U);
  for (uint16_t blockIndex = startBlockIndex; blockIndex < nBlocks; blockIndex++)
  {
    if (storage.LoadFields_type_sectionNameHash(blockIndex) == searchValue)
      return blockIndex;
  }

  // no match
  return NOBLOCK;
}
uint16_t EEPROMSectionSystem::FindSectionHeadByNextBlock(uint16_t const startBlockIndex, uint16_t const nextBlock, void* const pMem) const
/**
 * \brief Searches for a valid section head (BlockTypes::sectionHead) with a specific nextBlock-attribute and
 * loads the first matching section head.
 *
 * This includes CRC and consistency checks.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * ---
 *
 * \param startBlockIndex
 * Index where the search shall start at.
 * \param nextBlock
 * Value of the nextBlock-attribute we are looking for.
 * \param pMem
 * Pointer to a buffer into which the matching section head shall be loaded.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage. \n
 * Note that even in case of no match the buffer may have been modified by this method.
 * \return
 * Index of the matching section head.\n
 * NOBLOCK in case of no match.
 */
{
  if ((state == States::not_mounted) || (state == States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::FindSectionHeadByNextBlock", state, States::ro_mount);

  uint16_t const nBlocks = storage.GetnBlocks();

  if (startBlockIndex >= nBlocks)
    throw std::invalid_argument("EEPROMSectionSystem::FindSectionHeadByNextBlock: startBlockIndex invalid");

  CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t*>(pMem);
  for (uint16_t blockIndex = startBlockIndex; blockIndex < nBlocks; blockIndex++)
  {
    if (storage.LoadField_nextBlock(blockIndex) == nextBlock)
    {
      // load the whole block, double check nextBlock, and finish if type is BlockTypes::sectionHead
      storage.LoadBlock(blockIndex, pMem, storage.GetBlockSize());
      if (pHead->nextBlock != nextBlock)
        throw VolatileStorageError(blockIndex);

      if (static_cast<BlockTypes>(pHead->type) == BlockTypes::sectionHead)
        return blockIndex;
    }
  }

  // no match
  return NOBLOCK;
}

internal::FreeBlockListBackup EEPROMSectionSystem::GetFreeBlockListBackup(void) const noexcept
/**
 * \brief Creates a backup of the current state of the free block lists.
 *
 * This can be used to create a backup of the free block lists of the Section System before
 * allocating blocks via @ref GetBlockFromListOfFreeBlocks() or @ref GetBlocksFromListOfFreeBlocks().
 *
 * The allocation can be undone by recovering the free block list via @ref RewindFreeBlockLists() and
 * the @ref internal::FreeBlockListBackup instance retrieved from this. The following requirements must
 * be met:
 * - @ref mutex must be permanently locked from the call to this method until recovery.
 * - the allocated blocks must not be manipulated inside the storage.
 * - no block must be released between the call to this method and the recovery.
 *
 * Example:
 * ~~~{.cpp}
 * osal::MutexLocker locker(ESS.mutex);
 * auto backup = ESS.GetFreeBlockListBackup();
 * ON_SCOPE_EXIT(rewindAllocations)
 * {
 *   ESS.RewindFreeBlockLists(backup);
 * };
 * auto myFreeBlock = ESS.GetBlockFromListOfFreeBlocks(nullptr, true);
 * auto anotherFreeBlock = ESS.GetBlockFromListOfFreeBlocks(nullptr, true);
 *
 * // some stuff that may throw
 *
 * ON_SCOPE_EXIT_DISMISS(rewindAllocations);
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
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
 * Backup of the current state of the free block lists.
 */
{
  return internal::FreeBlockListBackup(nFreeBlocks, freeBlockListHeadIdx, freeBlockListEndIdx);
}
void EEPROMSectionSystem::RewindFreeBlockLists(internal::FreeBlockListBackup const & backup) noexcept
/**
 * \brief Restores a backup of the current state of the free block lists that has been
 * created via @ref GetFreeBlockListBackup().
 *
 * For details on usage please refer to documentation of @ref GetFreeBlockListBackup().
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
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
 * \param backup
 * Unmodifiable reference to the backup that shall be restored.
 */
{
  nFreeBlocks          = backup.nFreeBlocks;
  freeBlockListHeadIdx = backup.freeBlockListHeadIdx;
  freeBlockListEndIdx  = backup.freeBlockListEndIdx;
}
uint16_t EEPROMSectionSystem::LoadNextFreeBlock(void* const pMem) const
/**
 * \brief Loads the next block from a chain of free blocks (BlockTypes::freeBlock).
 *
 * This includes CRC and consistency checks.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the buffer referenced by `pMem` will contain undefined data.
 *
 * ---
 *
 * \param pMem
 * Pointer to a buffer that contains the current free block (BlockTypes::freeBlock).\n
 * The next free block (if any) will be loaded into this buffer.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 * \return
 * Index of the next free block.\n
 * NOBLOCK if there is no next free block.
 */
{
  if ((state == States::not_mounted) || (state == States::defect))
    throw InsufficientStateError("EEPROMSectionSystem::LoadNextFreeBlock", state, States::ro_mount);

  CommonBlockHead_t const * const pCommonHead = static_cast<CommonBlockHead_t*>(pMem);

  // check type of current block
  if (static_cast<BlockTypes>(pCommonHead->type) != BlockTypes::freeBlock)
    throw std::invalid_argument("EEPROMSectionSystem::LoadNextFreeBlock: pMem does not contain a free block");

  // no more blocks?
  if (pCommonHead->nextBlock == NOBLOCK)
    return NOBLOCK;

  // load next block
  uint16_t const nextBlockIndex = pCommonHead->nextBlock;
  storage.LoadBlock(nextBlockIndex, pMem, storage.GetBlockSize());

  // check block type
  if (static_cast<BlockTypes>(pCommonHead->type) != BlockTypes::freeBlock)
    throw BlockLinkageError("EEPROMSectionSystem::LoadNextFreeBlock: Unexpected block type", nextBlockIndex);

  return nextBlockIndex;
}
void EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks(uint16_t const startIndex, uint16_t const reservedBlockIndex, void* const pMem, bool const mutexLocked)
/**
 * \brief Walks through a chain of blocks (a section or part of a section) and appends the
 * visited blocks to the end of the list of free blocks. The blocks are therefore "deleted".
 *
 * The first visited block of the chain of blocks must have the type BlockTypes::sectionHead or
 * BlockTypes::sectionData. All other blocks must have the type BlockTypes::sectionData.
 * If any block with a different type or with an invalid sequence number occurs in the chain of blocks,
 * then an exception will be thrown.
 *
 * The last visited block in the chain of blocks will be the block whose nextBlock-field has the value
 * NOBLOCK or whose block index is equal to parameter `reservedBlockIndex`. The block referenced
 * by 'reservedBlockIndex' may have the type BlockTypes::freeBlock without causing an error, even
 * if it is the first block (parameter `startIndex`).
 *
 * To reduce the number of writes to the underlying @ref storage, blocks are appended to the
 * end of the list of free blocks in chunks of eight.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked, either by the caller or by this (see parameter `mutexLocked`).
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the blocks in the chain may be left in an undefined state (added to free block list or not).
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted or if the state of the blocks
 *   in the chain of blocks is undefined.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the blocks in the chain may be left in an undefined state (added to free block list or not).
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted or if the state of the blocks
 *   in the chain of blocks is undefined.
 *
 * ---
 *
 * \param startIndex
 * Index of the first block that shall be appended to the end of the list of free blocks.\n
 * If this is NOBLOCK, then this method does nothing.
 * \param reservedBlockIndex
 * Index of the last block that shall be deleted. If there is no specific block after which the delete
 * process shall stop, then this must be NOBLOCK. Note that the delete process may stop before
 * reaching this block (some nextBlock attribute equal to NOBLOCK).\n
 * The block referenced by this may have the type BlockTypes::freeBlock without causing an error,
 * even if it is the first block (startIndex).
 * \param pMem
 * Pointer to a buffer that can be used internally by this method.\n
 * The buffer must have a capacity of at least the block size of the underlying @ref storage.
 * \param mutexLocked
 * Indicates if @ref mutex is already locked by the calling method or not:\n
 * true  = @ref mutex is already locked\n
 * false = @ref mutex is NOT yet locked
 */
{
  osal::MutexLocker mutexLocker(mutexLocked ? nullptr : &mutex);

  if ((state != States::checking) && (state != States::mounted))
    throw InsufficientStateError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks", state, States::checking);

  ON_SCOPE_EXIT(markDefect) { state = States::defect; };

  // Declare a list for blocks that shall be deleted. The list allows to delete up to eight blocks
  // at once and thus safes some write accesses to the underlying storage.
  uint16_t toBeDeleted[8];
  size_t nbOfBlocksToBeDeleted = 0;

  bool first = true;
  uint16_t seqNb = 0;
  uint16_t currIndex = startIndex;
  uint16_t maxCycles = storage.GetnBlocks() - 1U;

  // loop until the end of the chain of blocks is reached
  while (currIndex != NOBLOCK)
  {
    // beware of endless loops
    if (maxCycles-- == 0)
      throw BlockLinkageError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks: Loop limit", currIndex);

    // load block
    storage.LoadBlock(currIndex, pMem, storage.GetBlockSize());
    CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t*>(pMem);

    // check block type
    switch (static_cast<BlockTypes>(pHead->type))
    {
      case BlockTypes::freeBlock:
        if (currIndex != reservedBlockIndex)
          throw BlockLinkageError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks: Unexpected block type", currIndex);
        break;

      case BlockTypes::sectionHead:
        if (!first)
          throw BlockLinkageError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks: Unexpected block type", currIndex);
        seqNb = 0;
        break;

      case BlockTypes::sectionData:
      {
        DataBlock_t const * const pData = static_cast<DataBlock_t*>(pMem);
        if (first)
          seqNb = pData->seqNb;
        else
        {
          seqNb++;
          if (pData->seqNb != seqNb)
            throw BlockLinkageError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks: Bad sequence number", currIndex);
        }
        break;
      }

      default:
        throw BlockLinkageError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks: Unexpected block type", currIndex);
    }
    first = false;

    // add block to the list of blocks that must be deleted
    toBeDeleted[nbOfBlocksToBeDeleted++] = currIndex;

    // list full?
    if (nbOfBlocksToBeDeleted == 8U)
    {
      AddBlocksToListOfFreeBlocks(toBeDeleted, nbOfBlocksToBeDeleted, true);
      nbOfBlocksToBeDeleted = 0;
    }

    // last block?
    if (currIndex == reservedBlockIndex)
      break;

    // next block during next loop cycle
    currIndex = pHead->nextBlock;
  }

  // delete the rest (if any)
  AddBlocksToListOfFreeBlocks(toBeDeleted, nbOfBlocksToBeDeleted, true);

  ON_SCOPE_EXIT_DISMISS(markDefect);
}
void EEPROMSectionSystem::AddBlockToListOfFreeBlocks(uint16_t const blockIndex, uint32_t const * const pCurrTotalNbOfWrites, bool const mutexLocked)
/**
 * \brief Appends one block to the end of the list of free blocks.
 *
 * __Thread safety:__\n
 * @ref mutex must be locked, either by the caller or by this (see parameter `mutexLocked`).
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the block referenced by "blockIndex" may be left with undefined content in @ref storage (_Section System is not_
 *   _considered corrupted in this case_).
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the block referenced by "blockIndex" may be left with undefined content in @ref storage (_Section System is not_
 *   _considered corrupted in this case_).
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * ---
 *
 * \param blockIndex
 * Index of the block that shall be appended to the end of the list of free blocks.
 * \param pCurrTotalNbOfWrites
 * If the current value of the field "totalNbOfWrites" of the block that shall be appended to
 * the end of the list of free blocks is known, then this pointer shall point to a variable
 * containing the value.\n
 * Otherwise this shall be `nullptr` and this method will load the value from the @ref storage.
 * Note that there are no data integrity checks (even not CRC) included, if this method loads the value.
 * Providing a valid pointer safes one read access from the @ref storage.
 * \param mutexLocked
 * Indicates if @ref mutex is already locked by the calling method or not:\n
 * true  = @ref mutex is already locked\n
 * false = @ref mutex is NOT yet locked
 */
{
  osal::MutexLocker mutexLocker(mutexLocked ? nullptr : &mutex);

  if ((state != States::checking) && (state != States::mounted))
    throw InsufficientStateError("EEPROMSectionSystem::AddBlockToListOfFreeBlocks", state, States::checking);

  // cannot have more free blocks than existing in the whole storage (minus Section System Info Block)
  if (nFreeBlocks >= storage.GetnBlocks() - 1U)
    throw std::logic_error("EEPROMSectionSystem::AddBlockToListOfFreeBlocks: Free blocks would exceed number of blocks");

  // determine the total number of writes already done to the block that shall be deleted
  uint32_t nWrites;
  if (pCurrTotalNbOfWrites != nullptr)
    nWrites = *pCurrTotalNbOfWrites;
  else
  {
    // Load the value of the field "totalNbOfWrites" from the storage.
    // This includes check of parameter "blockIndex".
    nWrites = storage.LoadField_totalNbOfWrites(blockIndex);
  }

  // allocate memory for a common header and a CRC from the stack
  char memFromStack[sizeof(CommonBlockHead_t) + sizeof(uint16_t)];
  char auxMemFromStack[sizeof(CommonBlockHead_t) + sizeof(uint16_t)];
  CommonBlockHead_t * const pHead = static_cast<CommonBlockHead_t*>(static_cast<void*>(memFromStack));

  // create a common header for a free block
  pHead->type            = static_cast<uint8_t>(BlockTypes::freeBlock);
  pHead->sectionNameHash = 0;
  pHead->nBytes          = sizeof(CommonBlockHead_t) + sizeof(uint16_t);
  pHead->totalNbOfWrites = nWrites;
  pHead->nextBlock       = NOBLOCK;

  // Write the just created header into the storage block that shall be added to the end of the
  // list of free blocks. This includes check of parameter "blockIndex".
  storage.StoreBlock(blockIndex, memFromStack, auxMemFromStack, false);

  // update stuff for tracking free blocks
  if (nFreeBlocks == 0)
  {
    freeBlockListHeadIdx = blockIndex;
  }
  else
  {
    // load the common header of the current last block in the list of free blocks and check it
    try
    {
      storage.LoadBlock(freeBlockListEndIdx, memFromStack, sizeof(memFromStack));

      if (static_cast<BlockTypes>(pHead->type) != BlockTypes::freeBlock)
        throw BlockLinkageError("EEPROMSectionSystem::AddBlockToListOfFreeBlocks: Last free block has unexpected type", freeBlockListEndIdx);

      if (pHead->nextBlock != NOBLOCK)
        throw BlockLinkageError("EEPROMSectionSystem::AddBlockToListOfFreeBlocks: Last free block has unexpected nextBlock", freeBlockListEndIdx);
    }
    catch (DataIntegrityError const &)
    {
      state = States::defect;
      throw;
    }

    // update header and store the block back to the storage
    pHead->nextBlock = blockIndex;
    ON_SCOPE_EXIT(markDefect) { state = States::defect; };
    storage.StoreBlock(freeBlockListEndIdx, memFromStack, auxMemFromStack, false);
    ON_SCOPE_EXIT_DISMISS(markDefect);
  }

  freeBlockListEndIdx = blockIndex;
  nFreeBlocks++;
}
void EEPROMSectionSystem::AddBlocksToListOfFreeBlocks(uint16_t const * const pBlockIndexList, uint16_t const n, bool const mutexLocked)
/**
 * \brief Appends zero, one, or more blocks to the end of the list of free blocks.
 *
 * __Thread safety:__\n
 * @ref mutex must be locked, either by the caller or by this (see parameter `mutexLocked`).
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the blocks in "pBlockIndexList" may be left with undefined content in @ref storage (_Section System is not_
 *   _considered corrupted in this case_).
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the blocks in "pBlockIndexList" may be left with undefined content in @ref storage (_Section System is not_
 *   _considered corrupted in this case_).
 * - the Section System may be corrupted (free block list).
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 *
 * ---
 *
 * \param pBlockIndexList
 * Pointer to a list containing the indices of the blocks that shall be appended to the end of the list of free blocks.
 * The content of the blocks will be modified by this method, even if the method does not finish regularly.
 * \param n
 * Number of blocks. If this is zero, then this method does nothing.
 * \param mutexLocked
 * Indicates if @ref mutex is already locked by the calling method or not:\n
 * true  = @ref mutex is already locked\n
 * false = @ref mutex is NOT yet locked
 */
{
  if (n == 0)
    return;

  if (pBlockIndexList == nullptr)
    throw std::invalid_argument("EEPROMSectionSystem::AddBlocksToListOfFreeBlocks: !pBlockIndexList");

  osal::MutexLocker mutexLocker(mutexLocked ? nullptr : &mutex);

  if ((state != States::checking) && (state != States::mounted))
    throw InsufficientStateError("EEPROMSectionSystem::AddChainOfBlocksToListOfFreeBlocks", state, States::checking);

  // cannot have more blocks than existing (minus Section System Info Block) in the list of free blocks
  if (static_cast<uint32_t>(nFreeBlocks) + n >= storage.GetnBlocks())
    throw std::logic_error("EEPROMSectionSystem::AddBlocksToListOfFreeBlocks: Free blocks would exceed number of blocks");

  // allocate memory from stack
  char memFromStack[sizeof(CommonBlockHead_t) + sizeof(uint16_t)];
  char auxMemFromStack[sizeof(CommonBlockHead_t) + sizeof(uint16_t)];
  CommonBlockHead_t* const pHead = static_cast<CommonBlockHead_t*>(static_cast<void*>(memFromStack));

  // Create a header for a free block. This is a template, it will be completed in the for-loop below...
  pHead->type            = static_cast<uint8_t>(BlockTypes::freeBlock);
  pHead->sectionNameHash = 0;
  pHead->nBytes          = sizeof(CommonBlockHead_t) + sizeof(uint16_t);

  // write new headers to all blocks in the list pBlockIndexList
  for (uint16_t i = 0; i < n; i++)
  {
    // Load the value of the field "totalNbOfWrites" from storage. This
    // also checks pBlockIndexList[i].
    uint32_t const nWrites = storage.LoadField_totalNbOfWrites(pBlockIndexList[i]);

    // complete the prepared header for the free block
    pHead->totalNbOfWrites = nWrites;
    if (i < n - 1U)
      pHead->nextBlock = pBlockIndexList[i + 1U];
    else
      pHead->nextBlock = NOBLOCK;

    // write the header into the storage
    storage.StoreBlock(pBlockIndexList[i], memFromStack, auxMemFromStack, true);
  }

  // update stuff for management of free blocks
  if (nFreeBlocks == 0)
    freeBlockListHeadIdx = pBlockIndexList[0];
  else
  {
    // load the common header of the current last block in the list of free blocks and check it
    try
    {
      storage.LoadBlock(freeBlockListEndIdx, memFromStack, sizeof(memFromStack));

      if (static_cast<BlockTypes>(pHead->type) != BlockTypes::freeBlock)
        throw BlockLinkageError("EEPROMSectionSystem::AddBlocksToListOfFreeBlocks: Last free block has unexpected type", freeBlockListEndIdx);

      if (pHead->nextBlock != NOBLOCK)
        throw BlockLinkageError("EEPROMSectionSystem::AddBlocksToListOfFreeBlocks: Last free block has unexpected nextBlock", freeBlockListEndIdx);
    }
    catch (DataIntegrityError const &)
    {
      state = States::defect;
      throw;
    }

    // update header and store the block back to the storage
    pHead->nextBlock = pBlockIndexList[0];
    ON_SCOPE_EXIT(markDefect) { state = States::defect; };
    storage.StoreBlock(freeBlockListEndIdx, memFromStack, auxMemFromStack, false);
    ON_SCOPE_EXIT_DISMISS(markDefect);
  }

  nFreeBlocks += n;
  freeBlockListEndIdx = pBlockIndexList[n - 1U];
}
uint16_t EEPROMSectionSystem::GetBlockFromListOfFreeBlocks(uint32_t* const pTotalNbOfWrites, bool const mutexLocked)
/**
 * \brief Retrieves one block from the beginning of the list of free blocks.
 *
 * This includes CRC and consistency checks on the removed block.
 *
 * __Thread safety:__\n
 * @ref mutex must be locked, either by the caller or by this (see parameter `mutexLocked`).
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 * - apart from that there are no further side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pTotalNbOfWrites
 * If this is not nullptr, then the value of the totalNbOfWrites-attribute of the header
 * of the removed block is written into the referenced variable. The referenced variable is only
 * altered if this method returns regularly (no exception and no deferred cancellation) and if the
 * return value is not NOBLOCK.
 * \param mutexLocked
 * Indicates if @ref mutex is already locked by the calling method or not:\n
 * true  = @ref mutex is already locked\n
 * false = @ref mutex is NOT yet locked
 * \return
 * Index of the block removed from the list of free blocks.\n
 * NOBLOCK, if there are no free blocks.
 */
{
  osal::MutexLocker mutexLocker(mutexLocked ? nullptr : &mutex);

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::GetBlockFromListOfFreeBlocks", state, States::mounted);

  if (nFreeBlocks == 0)
    return NOBLOCK;

  if (freeBlockListHeadIdx == NOBLOCK)
  {
    state = States::defect;
    throw std::logic_error("EEPROMSectionSystem::GetBlockFromListOfFreeBlocks: freeBlockListHeadIdx is invalid");
  }

  // allocate memory for a common header and a CRC from the stack
  char memFromStack[sizeof(CommonBlockHead_t) + sizeof(uint16_t)];
  CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t*>(static_cast<void*>(memFromStack));

  // load the first block from the list of free blocks and check its type
  try
  {
    storage.LoadBlock(freeBlockListHeadIdx, memFromStack, sizeof(memFromStack));
    if (static_cast<BlockTypes>(pHead->type) != BlockTypes::freeBlock)
      throw BlockLinkageError("EEPROMSectionSystem::GetBlockFromListOfFreeBlocks: First free block has unexpected type", freeBlockListHeadIdx);

    // check linkage to next block
    if (((nFreeBlocks == 1U) && (pHead->nextBlock != NOBLOCK)) ||
        ((nFreeBlocks != 1U) && (pHead->nextBlock == NOBLOCK)))
      throw BlockLinkageError("EEPROMSectionSystem::GetBlockFromListOfFreeBlocks: Last free block has invalid nextBlock", freeBlockListHeadIdx);
  }
  catch (DataIntegrityError const &)
  {
    state = States::defect;
    throw;
  }

  // we have a free block
  uint16_t const blockIndex = freeBlockListHeadIdx;

  // update management of free blocks
  freeBlockListHeadIdx = pHead->nextBlock;
  nFreeBlocks--;
  if (nFreeBlocks == 0)
    freeBlockListEndIdx = NOBLOCK;

  // finished
  if (pTotalNbOfWrites != nullptr)
    *pTotalNbOfWrites = pHead->totalNbOfWrites;
  return blockIndex;
}
bool EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks(uint16_t* pBlockIndexList, uint16_t const n, bool const mutexLocked)
/**
 * \brief Retrieves zero, one, or more blocks from the beginning of the list of free blocks.
 *
 * __Thread safety:__\n
 * @ref mutex must be locked, either by the caller or by this (see parameter `mutexLocked`).
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - @ref state is set to @ref States::defect, if the Section System is corrupted.
 * - apart from that there are no further side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pBlockIndexList
 * The indices of the blocks removed from the list of free blocks are written into the referenced list.
 * \param n
 * Number of blocks that shall be removed. Zero is allowed.\n
 * The list referenced by `pBlockIndexList` must provide at least `n` entries.
 * \param mutexLocked
 * Indicates if @ref mutex is already locked by the calling method or not:\n
 * true  = @ref mutex is already locked\n
 * false = @ref mutex is NOT yet locked
 * \return
 * Indicates if the request has been satisfied:\n
 * true  = yes\n
 * false = no, requested number of blocks was not available
 */
{
  if (n == 0)
    return true;

  osal::MutexLocker mutexLocker(mutexLocked ? nullptr : &mutex);

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks", state, States::mounted);

  // not enough free blocks?
  if (nFreeBlocks < n)
    return false;

  // allocate memory for a common header plus CRC from the stack
  char memFromStack[sizeof(CommonBlockHead_t) + sizeof(uint16_t)];
  CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t*>(static_cast<void*>(memFromStack));

  // collect block indices of n free blocks
  uint16_t lastIdx = NOBLOCK;
  uint16_t currIdx = freeBlockListHeadIdx;
  for (uint16_t i = 0; i < n; i++)
  {
    if (currIdx == NOBLOCK)
    {
      state = States::defect;
      if (i == 0)
        throw std::logic_error("EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks: freeBlockListHeadIdx is invalid");
      else
        throw BlockLinkageError("EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks: Unexpected nextBlock (NOBLOCK)", lastIdx);
    }

    // unexpected end of list of free blocks?
    if ((i < n - 1U) &&
        (currIdx == freeBlockListEndIdx))
    {
      state = States::defect;
      throw std::logic_error("EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks: Unexpected end of free block list");
    }

    try
    {
      storage.LoadBlock(currIdx, memFromStack, sizeof(memFromStack));
      if (static_cast<BlockTypes>(pHead->type) != BlockTypes::freeBlock)
        throw BlockLinkageError("EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks: Free block has unexpected type", currIdx);
    }
    catch (DataIntegrityError const &)
    {
      state = States::defect;
      throw;
    }

    // add block to list
    *pBlockIndexList++ = currIdx;
    lastIdx = currIdx;
    currIdx = pHead->nextBlock;
  }

  if (((nFreeBlocks == n) && (currIdx != NOBLOCK)) ||
      ((nFreeBlocks != n) && (currIdx == NOBLOCK)))
  {
    state = States::defect;
    throw BlockLinkageError("EEPROMSectionSystem::GetBlocksFromListOfFreeBlocks: Last free block has invalid nextBlock", lastIdx);
  }

  // update management of free blocks
  freeBlockListHeadIdx = currIdx;
  nFreeBlocks -= n;

  if (nFreeBlocks == 0)
    freeBlockListEndIdx = NOBLOCK;

  return true;
}

void EEPROMSectionSystem::StoreBlock(uint16_t const blockIndex, void* const pMem, bool const mutexLocked)
/**
 * \brief Stores an block into the underlying @ref storage.
 *
 * This is a helper for class @ref internal::SectionWriter.
 *
 * ---
 *
 * __Thread safety:__\n
 * @ref mutex must be locked, either by the caller or by this (see parameter `mutexLocked`).
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the storage block referenced by `blockIndex` may be left with undefined data.
 * - the memory referenced by `pMem` may be modified.
 * - @ref state is set to @ref States::defect, if the storage block referenced by `blockIndex` is left with undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the storage block referenced by `blockIndex` may be left with undefined data.
 * - the memory referenced by `pMem` may be modified.
 * - @ref state is set to @ref States::defect, if the storage block referenced by `blockIndex` is left with undefined data.
 *
 * ---
 *
 * \param blockIndex
 * Index of the storage block that shall be written.
 * \param pMem
 * Pointer to a buffer containing the data (incl. common header and placeholder for CRC) that shall
 * be written into the block referenced by `blockIndex`. The field "totalNbOfWrites" must contain the
 * current value.\n
 * Note that the content of the referenced memory will always be modified by this method, even in case of failure.
 * \param mutexLocked
 * Indicates if @ref mutex is already locked by the calling method or not:\n
 * true  = @ref mutex is already locked\n
 * false = @ref mutex is NOT yet locked
 */
{
  osal::MutexLocker mutexLocker(mutexLocked ? nullptr : &mutex);

  if (state != States::mounted)
    throw InsufficientStateError("EEPROMSectionSystem::StoreBlock", state, States::mounted);

  ON_SCOPE_EXIT(markDefect) { state = States::defect; };
  storage.StoreBlock(blockIndex, pMem, nullptr, false);
  ON_SCOPE_EXIT_DISMISS(markDefect);
}

} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc
