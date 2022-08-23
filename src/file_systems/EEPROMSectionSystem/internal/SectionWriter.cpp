/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "SectionWriter.hpp"
#include "EEPROMSectionSystemInternals.hpp"
#include "../EEPROMSectionSystem.hpp"
#include "gpcc/src/file_systems/EEPROMSectionSystem/Exceptions.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/Stream/StreamErrors.hpp"
#include <exception>
#include <stdexcept>
#include <cstring>

namespace gpcc
{
namespace file_systems
{
namespace EEPROMSectionSystem
{
namespace internal
{

SectionWriter::SectionWriter(EEPROMSectionSystem & _ESS,
                             std::string const & _sectionName,
                             uint16_t const _oldSectionHeadIndex,
                             uint16_t const _sectionHeadIndex,
                             uint16_t const _version,
                             uint16_t const _nextBlockIndex,
                             std::unique_ptr<char[]> _spMem)
: gpcc::Stream::StreamWriterBase(States::open, Endian::Little)
, pESS(&_ESS)
, sectionName(_sectionName)
, oldSectionHeadIndex(_oldSectionHeadIndex)
, sectionHeadIndex(_sectionHeadIndex)
, version(_version)
, firstDataBlockIndex(_nextBlockIndex)
, nextBlockIndex(_nextBlockIndex)
, seqNb(1)
, spMem(std::move(_spMem))
, wrPtr(spMem.get() + sizeof(DataBlock_t))
, nbOfBitsWritten(0)
, bitData(0)
, remainingBytesInCurrentBlock(_ESS.storage.GetBlockSize() - (sizeof(DataBlock_t) + sizeof(uint16_t)))
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
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _ESS
 * Reference to the @ref EEPROMSectionSystem instance in which the new section shall be created.
 * \param _sectionName
 * Name of the section.\n
 * The section must be locked at the @ref EEPROMSectionSystem for writing. It will be unlocked by this
 * when the stream is finally closed.
 * \param _oldSectionHeadIndex
 * If an existing section shall be overwritten, then this shall contain the block index of the
 * section head of the section that shall be overwritten.\n
 * If a new section shall be created, then this must be NOBLOCK.
 * \param _sectionHeadIndex
 * Index of an allocated storage block that shall be used to store the section head of the new section.
 * \param _version
 * Version of the new section head.
 * \param _nextBlockIndex
 * Index of an allocated storage block that shall be used to store the section's data. More blocks
 * will be allocated automatically if necessary.
 * \param _spMem
 * Pointer to a block of memory granted to the new @ref SectionWriter instance for internal usage.\n
 * The memory block must have a capacity of at least the block size of the underlying storage.
 */
{
  if (!spMem)
    throw std::invalid_argument("SectionWriter::SectionWriter: !_spMem");

  pESS->storage.LoadBlock(nextBlockIndex, spMem.get(), pESS->storage.GetBlockSize());
}
SectionWriter::SectionWriter(SectionWriter && other) noexcept
: gpcc::Stream::StreamWriterBase(std::move(other))
, pESS(other.pESS)
, sectionName(std::move(other.sectionName))
, oldSectionHeadIndex(other.oldSectionHeadIndex)
, sectionHeadIndex(other.sectionHeadIndex)
, version(other.version)
, firstDataBlockIndex(other.firstDataBlockIndex)
, nextBlockIndex(other.nextBlockIndex)
, seqNb(other.seqNb)
, spMem(std::move(other.spMem))
, wrPtr(other.wrPtr)
, nbOfBitsWritten(other.nbOfBitsWritten)
, bitData(other.bitData)
, remainingBytesInCurrentBlock(other.remainingBytesInCurrentBlock)
/**
 * \brief Move constructor.
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
 * \param other
 * A @ref SectionWriter instance that shall be moved into the new constructed one.\n
 * The referenced instance is switched to @ref States::closed.
 */
{
  other.wrPtr = nullptr;
  other.pESS = nullptr;
  other.state = States::closed;
}
SectionWriter::~SectionWriter(void)
/**
 * \brief Destructor. Closes the section (if not yet done) and releases the object.
 *
 * _Any stream should be closed via @ref Close() before it is released._\n
 * If it is not closed yet, then it will be closed now by this destructor.\n
 * If the close-operation fails or if thread cancellation occurs, then the application
 * will terminate via @ref gpcc::osal::Panic().
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object after invocation of destructor.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations may only fail due to serious errors that will result in program termination via Panic(...).
 * To prevent any error, ensure that the stream is closed __before__ it is released.
 *
 * __Thread cancellation safety:__\n
 * Cancellation not allowed:\n
 * Cancellation will result in @ref gpcc::osal::Panic().
 */
{
  try
  {
    if (state != States::closed)
      Close();
  }
  catch (...)
  {
    PANIC();
  }
}

bool SectionWriter::IsRemainingCapacitySupported(void) const
/// \copydoc gpcc::Stream::IStreamWriter::IsRemainingCapacitySupported
{
  return false;
}

size_t SectionWriter::RemainingCapacity(void) const
/// \copydoc gpcc::Stream::IStreamWriter::RemainingCapacity
{
  switch (state)
  {
    case States::open:
      throw std::logic_error("SectionWriter::RemainingCapacity: Operation not supported");

    case States::full:
      // (this state is not used by class SectionWriter)
      throw std::logic_error("SectionWriter::RemainingCapacity: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  }

  PANIC();
}
uint_fast8_t SectionWriter::GetNbOfCachedBits(void) const
/// \copydoc gpcc::Stream::IStreamWriter::GetNbOfCachedBits
{
  switch (state)
  {
    case States::open:
      return nbOfBitsWritten;

    case States::full:
      // (this state is not used by class SectionWriter)
      throw std::logic_error("SectionWriter::GetNbOfCachedBits: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  }

  PANIC();
}

void SectionWriter::Close(void)
/// \copydoc gpcc::Stream::IStreamWriter::Close
{
  switch (state)
  {
    case States::open:
      CloseAnOpenSectionWriter();
      break;

    case States::full:
      // (this state is not used by class SectionWriter)
      state = States::error;
      CloseCrashedSectionWriter();
      throw std::logic_error("SectionWriter::Close: Unused state (States::full) encountered");

    case States::closed:
      break;

    case States::error:
      CloseCrashedSectionWriter();
      break;
  } // switch (state)
}

void SectionWriter::Push(char c)
/// \copydoc gpcc::Stream::StreamWriterBase::Push(char c)
{
  if (nbOfBitsWritten != 0)
    PushBitsPlusGap();

  switch (state)
  {
    case States::open:
    {
      // buffer full?
      if (remainingBytesInCurrentBlock == 0)
        StoreCurrentBlockAndReserveNextBlock();

      // write byte
      *wrPtr++ = c;
      remainingBytesInCurrentBlock--;

      break;
    }

    case States::full:
      // (this state is not used by class SectionWriter)
      state = States::error;
      throw std::logic_error("SectionWriter::Push: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}
void SectionWriter::Push(void const * pData, size_t n)
/// \copydoc gpcc::Stream::StreamWriterBase::Push(void const * pData, size_t n)
{
  if (n == 0)
    return;

  if (nbOfBitsWritten != 0)
    PushBitsPlusGap();

  switch (state)
  {
    case States::open:
    {
      uint8_t const * rdPtr = static_cast<uint8_t const *>(pData);
      while (n != 0)
      {
        // buffer full?
        if (remainingBytesInCurrentBlock == 0)
          StoreCurrentBlockAndReserveNextBlock();

        // determine chunksize
        size_t chunksize;
        if (remainingBytesInCurrentBlock < n)
          chunksize = remainingBytesInCurrentBlock;
        else
          chunksize = n;

        // write data
        memcpy(wrPtr, rdPtr, chunksize);
        wrPtr += chunksize;
        remainingBytesInCurrentBlock -= chunksize;

        rdPtr += chunksize;
        n -= chunksize;
      } // while (n != 0)

      break;
    }

    case States::full:
      // (this state is not used by class SectionWriter)
      state = States::error;
      throw std::logic_error("SectionWriter::Push: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}
void SectionWriter::PushBits(uint8_t bits, uint_fast8_t n)
/// \copydoc gpcc::Stream::StreamWriterBase::PushBits(uint8_t bits, uint_fast8_t n)
{
  if (n == 0)
    return;

  if (n > 8)
    throw std::invalid_argument("SectionWriter::PushBits: n must be [0..8].");

  switch (state)
  {
    case States::open:
    {
      // buffer full?
      if (remainingBytesInCurrentBlock == 0)
        StoreCurrentBlockAndReserveNextBlock();

      // clear upper bits that shall be ignored
      bits &= static_cast<uint_fast16_t>(static_cast<uint_fast16_t>(1U) << n) - 1U;

      // combine potential previously written bits with the bits that shall be written
      uint_fast16_t data = static_cast<uint_fast16_t>(bitData) | (static_cast<uint_fast16_t>(bits) << nbOfBitsWritten);
      nbOfBitsWritten += n;

      // one byte filled up with bits?
      if (nbOfBitsWritten >= 8U)
      {
        // write byte into the stream
        *wrPtr++ = static_cast<char>(data);
        remainingBytesInCurrentBlock--;

        nbOfBitsWritten -= 8U;
        data >>= 8U;

        // buffer full and more bits must be written?
        if ((remainingBytesInCurrentBlock == 0) &&
            (nbOfBitsWritten != 0))
        {
          // Store data and reserve a new block here. Doing so is required because
          // closeAnOpenStream(...) can then rely on that there is at least one byte
          // capacity in the current block when the remaining bits are written.
          StoreCurrentBlockAndReserveNextBlock();
        }
      }

      // store temporary stuff back in bitData
      bitData = static_cast<uint8_t>(data);

      break;
    }

    case States::full:
      // (this state is not used by class SectionWriter)
      state = States::error;
      throw std::logic_error("SectionWriter::PushBits: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

void SectionWriter::PushBitsPlusGap(void)
/**
 * \brief Pushes bits from @ref bitData into the buffer. Upper bits are filled with zeros to get 8 bits.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the @ref state of the @ref SectionWriter is set to @ref States::error or left in @ref States::closed.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if the
 *   Section System is corrupted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the @ref state of the @ref SectionWriter is set to @ref States::error or left in @ref States::closed.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if the
 *   Section System is corrupted by this.
 */
{
  // move the bits to be written into d
  char const d = static_cast<char>(bitData);

  // clear bit buffer now and not after writing the bits, because this may be part of a recursive call
  nbOfBitsWritten = 0;
  bitData = 0;

  // write bits
  Push(d);
}
void SectionWriter::StoreCurrentBlock(uint16_t const blockSize, uint16_t const nextBlock)
/**
 * \brief Sets the header for the current block (buffer @ref spMem) and stores the current block
 * at @ref nextBlockIndex into the storage.
 *
 * Note:\n
 * - The field "totalNbOfWrites" of the header (in buffer @ref spMem) must already be setup.
 * - The content of the buffer @ref spMem will always be modified by this method in case of failure and
 *   in case of success.
 *
 * ---
 *
 * __Thread safety:__\n
 * - The state of the object is modified. Concurrent accesses are not safe.
 * - `pESS->mutex` must be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the storage block referenced by `nextBlockIndex` may be left with undefined data.
 * - the @ref state of the @ref SectionWriter is set to @ref States::error.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if the
 *   storage block referenced by `nextBlockIndex` is left with undefined data.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the storage block referenced by `nextBlockIndex` may be left with undefined data.
 * - the @ref state of the @ref SectionWriter is set to @ref States::error.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if the
 *   storage block referenced by `nextBlockIndex` is left with undefined data.
 *
 * ---
 *
 * \param blockSize
 * Block size of the underlying storage in bytes.
 * \param nextBlock
 * Desired value for the nextBlock-attribute of the header of the current block.
 */
{
  ON_SCOPE_EXIT(SetErrorState) { state = States::error; };

  if ((state != States::open) || (pESS == nullptr))
    PANIC(); // state/pESS bad

  // finish the header for the block that shall be stored
  DataBlock_t* const pData = static_cast<DataBlock_t*>(static_cast<void*>(spMem.get()));

  pData->head.type            = static_cast<uint8_t>(BlockTypes::sectionData);
  pData->head.sectionNameHash = 0;
  pData->head.nBytes          = blockSize - remainingBytesInCurrentBlock;
  // totalNbOfWrites is already setup
  pData->head.nextBlock       = nextBlock;
  pData->seqNb                = seqNb;

  // store
  pESS->StoreBlock(nextBlockIndex, spMem.get(), true);

  ON_SCOPE_EXIT_DISMISS(SetErrorState);
}
void SectionWriter::StoreCurrentBlockAndReserveNextBlock(void)
/**
 * \brief Allocates a new storage block, stores the current one, and prepares the
 * @ref SectionWriter for writing into the new block.
 *
 * __Thread safety:__\n
 * - The state of the object is modified. Concurrent accesses are not safe.
 * - `pESS->mutex` must not yet be locked.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the @ref state of the @ref SectionWriter is set to @ref States::error.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if the
 *   section system is corrupted.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the @ref state of the @ref SectionWriter is set to @ref States::error.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if the
 *   section system is corrupted by this.
 */
{
  // if this fails or is aborted (deferred thread cancellation), then the SectionWriter is defect.
  ON_SCOPE_EXIT(SetErrorState) { state = States::error; };

  if ((state != States::open) || (pESS == nullptr))
    PANIC(); // state/pESS bad

  try
  {
    osal::MutexLocker mutexLocker(pESS->mutex);

    // get storage properties
    uint16_t const blockSize = pESS->storage.GetBlockSize();

    // allocate next free block
    auto const fbl_backup = pESS->GetFreeBlockListBackup();
    uint32_t nextFreeBlockNbOfWrites;
    uint16_t const nextFreeBlock = pESS->GetBlockFromListOfFreeBlocks(&nextFreeBlockNbOfWrites, true);
    if (nextFreeBlock == NOBLOCK)
      throw Stream::FullError();
    ON_SCOPE_EXIT(releaseAllocatedFreeBlock)
    {
      pESS->RewindFreeBlockLists(fbl_backup);
    };

    StoreCurrentBlock(blockSize, nextFreeBlock);

    // prepare SectionWriter for further data
    static_cast<DataBlock_t*>(static_cast<void*>(spMem.get()))->head.totalNbOfWrites = nextFreeBlockNbOfWrites;
    nextBlockIndex = nextFreeBlock;
    ON_SCOPE_EXIT_DISMISS(releaseAllocatedFreeBlock);
    seqNb++;
    wrPtr = spMem.get() + sizeof(DataBlock_t);
    remainingBytesInCurrentBlock = blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t));
  }
  catch (Stream::FullError const &)
  {
    throw;
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(Stream::IOError("SectionWriter::StoreCurrentBlockAndReserveNextBlock: failed"));
  }

  ON_SCOPE_EXIT_DISMISS(SetErrorState);
}
void SectionWriter::EnterClosedState(void)
/**
 * \brief Closes the stream.
 *
 * The caller is responsible for finishing operations on the Section System's storage, e.g.:
 * - writing the section head and the last data block.
 * - or deleting the (unfinished) section in case of an error.
 * - or whatever is appropriate.
 *
 * This method does the following:
 * - resources allocated by the @ref SectionWriter (buffer, section's name) are released.
 * - the SectionWriter's lock-entry in EEPROMSectionSystem::sectionLockManager is removed.
 * - @ref state is switched to @ref States::closed.
 *
 * ---
 *
 * __Thread safety:__\n
 * - The state of the object is modified. Concurrent accesses are not safe.
 * - pESS->mutex must be locked.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.\n
 * Operations may also fail due to serious errors that will result in program termination via Panic(...).]
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if ((state == States::closed) || (pESS == nullptr))
    PANIC(); // state/pESS bad

  pESS->sectionLockManager.ReleaseWriteLock(sectionName);
  sectionName.clear();
  wrPtr = nullptr;
  spMem.reset();
  pESS = nullptr;

  state = States::closed;
}
void SectionWriter::CloseAnOpenSectionWriter(void)
/**
 * \brief Closes an @ref SectionWriter that is in state @ref States::open. The @ref SectionWriter
 * is always closed, even if an exception is thrown.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the @ref SectionWriter is always closed.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if it is
 *   corrupted by this.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the @ref SectionWriter is always closed.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if it is
 *   corrupted by this.
 */
{
  if ((state != States::open) || (pESS == nullptr))
    PANIC(); // state/pESS bad

  ON_SCOPE_EXIT(mutexLocker) { PANIC(); };
  osal::MutexLocker mutexLocker(pESS->mutex);
  ON_SCOPE_EXIT_DISMISS(mutexLocker);

  ON_SCOPE_EXIT(EnterClosedState) { EnterClosedState(); };

  try
  {
    // any bits left to be written?
    if (nbOfBitsWritten != 0)
      PushBitsPlusGap();

    uint16_t const blockSize = pESS->storage.GetBlockSize();

    // store currently written block
    StoreCurrentBlock(blockSize, NOBLOCK);

    // load the block that is foreseen to become the new section head
    pESS->storage.LoadBlock(sectionHeadIndex, spMem.get(), blockSize);

    // create a new section head
    SectionHeadBlock_t* const pHead = static_cast<SectionHeadBlock_t*>(static_cast<void*>(spMem.get()));

    pHead->head.type            = static_cast<uint8_t>(BlockTypes::sectionHead);
    pHead->head.sectionNameHash = CalcHash(sectionName.c_str());
    pHead->head.nBytes          = sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + sectionName.length() + 1U;
    // totalNbOfWrites is not touched
    pHead->head.nextBlock       = firstDataBlockIndex;
    pHead->version              = version;

    // copy section name into the section header (incl. null-terminator)
    if (pHead->head.nBytes > blockSize)
      throw std::logic_error("SectionWriter::CloseAnOpenStream: nBytes too large");
    memcpy(spMem.get() + sizeof(SectionHeadBlock_t), sectionName.c_str(), sectionName.length() + 1U);

    // store the new section head
    pESS->StoreBlock(sectionHeadIndex, spMem.get(), true);

    // The new section has been created now. Finally we have to delete the old section, if there is any.
    if (oldSectionHeadIndex != NOBLOCK)
      pESS->AddChainOfBlocksToListOfFreeBlocks(oldSectionHeadIndex, NOBLOCK, spMem.get(), true);
  }
  catch (std::exception const &)
  {
    state = States::error;
    pESS->state = EEPROMSectionSystem::States::defect;
    std::throw_with_nested(Stream::IOError("SectionWriter::CloseAnOpenSectionWriter: failed"));
  }
  catch (...)
  {
    state = States::error;
    pESS->state = EEPROMSectionSystem::States::defect;
    throw;
  }
}
void SectionWriter::CloseCrashedSectionWriter(void)
/**
 * \brief Closes a @ref SectionWriter which is in state @ref States::error. The Section System is properly cleaned up.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - the @ref SectionWriter is always closed.
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if it is
 *   corrupted by this.
 *
 * Operations may also fail due to serious errors that will result in program termination via Panic(...).
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, the @ref SectionWriter will always be switched to @ref States::closed, but:
 * - the state of the Section System is set to @ref EEPROMSectionSystem::States::defect, if it is
 *   corrupted by this.
 */
{
  if ((state != States::error) || (pESS == nullptr))
    PANIC(); // state/pESS bad

  ON_SCOPE_EXIT(mutexLocker) { PANIC(); };
  osal::MutexLocker mutexLocker(pESS->mutex);
  ON_SCOPE_EXIT_DISMISS(mutexLocker);

  ON_SCOPE_EXIT(EnterClosedState) { EnterClosedState(); };

  try
  {
    pESS->AddChainOfBlocksToListOfFreeBlocks(firstDataBlockIndex, nextBlockIndex, spMem.get(), true);
    pESS->AddBlockToListOfFreeBlocks(sectionHeadIndex, nullptr, true);
  }
  catch (std::exception const &)
  {
    pESS->state = EEPROMSectionSystem::States::defect;
    std::throw_with_nested(Stream::IOError("SectionWriter::CloseCrashedSectionWriter: failed"));
  }
  catch (...)
  {
    pESS->state = EEPROMSectionSystem::States::defect;
    throw;
  }
}

} // namespace internal
} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc
