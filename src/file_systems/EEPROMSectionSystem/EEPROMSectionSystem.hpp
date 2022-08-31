/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EEPROMSECTIONSYSTEM_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EEPROMSECTIONSYSTEM_HPP_

#include "internal/EEPROMSectionSystemInternals.hpp"
#include "internal/BlockAccessor.hpp"
#include "internal/FreeBlockListBackup.hpp"
#include "../IFileStorage.hpp"
#include <gpcc/osal/Mutex.hpp>
#include "gpcc/src/ResourceManagement/Objects/SmallDynamicNamedRWLock.hpp"
#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <cstddef>

namespace gpcc
{

namespace StdIf
{
  class IRandomAccessStorage;
}

namespace container
{
  class BitField;
}

namespace Stream
{
  class IStreamReader;
  class IStreamWriter;
}

namespace file_systems
{
namespace EEPROMSectionSystem
{

namespace internal
{
  class SectionReader;
  class SectionWriter;
}

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM
 * \brief EEPROM Content Management System, Power-Fail-Safe.
 *
 * @ref EEPROMSectionSystem is a content management system that allows to store and manage data
 * stored in an EEPROM or similar device in a file-system-like fashion. Data is stored in so-called
 * "sections" which are referenced by their names (text strings). @ref EEPROMSectionSystem does not provide
 * folders, but you can use text strings as section names that look like a full path with filename on an
 * e.g. FAT partition.
 *
 * @ref EEPROMSectionSystem implements the @ref IFileStorage interface. Sections can be read and written via the
 * [IStreamReader](@ref gpcc::Stream::IStreamReader) and [IStreamWriter](@ref gpcc::Stream::IStreamWriter) interfaces.
 * [IStreamReader::RemainingBytes()](@ref gpcc::Stream::IStreamReader::RemainingBytes) and
 * [IStreamWriter::RemainingCapacity()](@ref gpcc::Stream::IStreamWriter::RemainingCapacity) are not supported.
 *
 * If the requirements described in section "Storage Requirements" are met, then @ref EEPROMSectionSystem
 * is _power-fail-safe_, even if power fails during an operation that modifies the content of the storage device.
 * This means that after restart from a power-fail and after mounting the Section System, the stored data
 * will always be in a valid state:
 * - Either it will be in the state in which it had been before the operation was started, or ...
 * - The operation has finished just before the power-fail
 *
 * # Storage Requirements
 * The content management system has originally been designed for EEPROM devices, but it can be used with
 * any other storage devices like serial FRAMs, flash, or even plain RAM too. Drivers for these devices
 * just have to provide the @ref StdIf::IRandomAccessStorage interface to offer access to the storage.
 *
 * If power-fail-safety is required, then the driver for accessing the storage and the storage itself must
 * fulfill the following requirements:
 *
 * Devices that write all data in a page-write in parallel (e.g. standard I2C EEPROM devices):
 * - All data written via the @ref StdIf::IRandomAccessStorage interface using page-write/block-write must be
 *   either completely written, or no write to any memory cell shall occur.
 * - The write access must complete properly in order to guarantee sufficient data retention. For details on
 *   this topic please refer to AN2014 from ST Microelectronics.
 * - a solution might be to provide sufficient backup for the EEPROM's power supply in order to allow any
 *   write access issued even just before power-fail to complete properly.
 *
 * Devices that write all data from a page-write sequentially byte by byte (e.g. I2C FRAM devices):
 * - All data written via the @ref StdIf::IRandomAccessStorage interface using page-write/block-write must be
 *   written one by one into the storage. The order in which the writes occur must correspond to the addresses
 *   of the written memory cells in ascending order.
 * - The page-write/block-write access during which power fails does not need to complete, but the last
 *   written byte before power-fail must be written properly into the storage cell. The next byte must not
 *   be attempted to be written. This guarantees that the storage cells are in a physically proper state.
 *
 * # Mounting
 * Mounting an Section System is a two step operation:\n
 * A call to @ref MountStep1() mounts the Section System for read-only access.\n
 * A subsequent call to @ref MountStep2() checks the Section System for errors, repairs it if necessary, and
 * mounts the Section System for full read-write access.
 *
 * During any mount operation, access to the Section System is denied while the mount operation is in process.
 * The first mounting step (@ref MountStep1()) finishes quickly (only one block of the storage is read), while
 * the second step (@ref MountStep2()) requires examination of all storage blocks. Examination includes reading
 * all blocks from storage one-by-one. During step 2 any garbage left during a sudden reset or power-fail is
 * cleaned up.
 *
 * The intention of splitting the mount operation in two steps is to allow for fast system startup. Read access
 * to e.g. parameters and configuration data is possible at an early point in time during system startup, at the
 * cost of denying write access to the Section System until the second (more time consuming) step of the mount
 * operation has been done.
 *
 * Read operations are slightly more time-consuming before the second step of the mount operation has been done.
 * This is because read operations must be aware of some potential errors that are checked and fixed during
 * @ref MountStep2().
 *
 * # States
 * Class @ref EEPROMSectionSystem has an internal `state` attribute that describes the state of the Section System
 * managed by an instance of class @ref EEPROMSectionSystem. The current state can be retrieved via @ref GetState().
 *
 * The different state values (@ref States) have the following meaning:
 *
 * `state` value            | Meaning
 * ------------------------ | ------------------------------------------------
 * @ref States::not_mounted | Section System is not mounted.
 * @ref States::ro_mount    | Section System is mounted for read-only access.
 * @ref States::checking    | Section System is currently checked for errors (@ref MountStep2()).
 * @ref States::mounted     | Section System is mounted for full read/write-access.
 * @ref States::defect      | Section System is defect and needs check (call to @ref MountStep2()).
 *
 * \htmlonly <style>div.image img[src="file_systems/EEPROMSectionSystem/EEPROMSectionSystemInternalStates.jpg"]{width:80%;}</style> \endhtmlonly
 * \image html "file_systems/EEPROMSectionSystem/EEPROMSectionSystemInternalStates.jpg" "States of the Section System"
 *
 * # Internals
 * ## Definition of the term "Section System"
 * The term "Section System" refers to the contents of the underlying `storage` plus the free block list
 * management comprised of the attributes `nFreeBlocks`, `freeBlockListHeadIdx`, and `freeBlockListEndIdx` of
 * class @ref EEPROMSectionSystem.
 *
 * Attribute `state` describes the current state of the Section System:
 * `state` value            | Block size | free block list management | `storage` content
 * ------------------------ | ---------- | -------------------------- | ----------------------------------
 * @ref States::not_mounted | not set    | not initialized            | unknown
 * @ref States::ro_mount    | setup      | not initialized            | unchecked Section System
 * @ref States::checking    | setup      | incomplete                 | check of Section System in process
 * @ref States::mounted     | setup      | valid                      | consistent Section System
 * @ref States::defect      | setup      | may be inconsistent        | maybe inconsistent Section System
 *
 * ## States::defect
 * ### Meaning of States::defect
 * @ref States::defect indicates that the Section System is in an inconsistent state. This means that any of
 * the following stuff may be invalid or inconsistent:
 * - free block list management comprised of attributes `nFreeBlocks`, `freeBlockListHeadIdx`,
 *   and `freeBlockListEndIdx` of class @ref EEPROMSectionSystem.
 * - `storage` content (free block list, sections, section heads, block linkage, ...)
 *
 * ### Entering States::defect
 * Any method that __modifies__ the Section System (storage and/or free block list management) and that did
 * not __finish__ the modification __successfully__ due to an exception, an error, or due to deferred thread
 * cancellation __must__ switch `state` to @ref States::defect. It is not relevant if the error, the exception,
 * or the thread cancellation occurred in the method itself or in a subroutine.
 *
 * Any error, exception, or thread-cancellation that occurs __before__ a modification is started does usually
 * not corrupt the Section System. In these cases methods are not required to switch `state` to
 * @ref States::defect, but they are _allowed_ to do so in order to simplify error handling.
 *
 * If an error, exception, or thread-cancellation occurs while an object or method has some free blocks
 * allocated from the Section System, then the allocation must be undone or the blocks must be released to
 * the Section System's free block list. If this is not possible, then `state` must be switched to
 * @ref States::defect.
 *
 * Nevertheless, any error that occurs, especially if `state` is not required to be set to @ref States::defect,
 * must be handled properly and reported, e.g. by throwing an exception or by rethrowing/forwarding an exception
 * that has been thrown in a subroutine.
 *
 * Methods that read the Section System (storage and/or free block list management) and that are not used
 * during the mount process should switch `state` to @ref States::defect if an inconsistency or error is detected.
 * This behavior is not a must, it is optional.
 *
 * ## Sections
 * Data is organized in sections. Each section has a unique name and is comparable to a file known from
 * standard file systems like FAT. A section can store any number of bytes from zero up to the EEPROM's capacity.
 * The data stored in a section is protected by a 16bit CRC and by sequence numbers. Sections can be created,
 * overwritten, opened, deleted, renamed, enumerated, and measured.
 *
 * ## Storage organization
 * The EEPROM/storage is divided into "blocks". The page size of the EEPROM/storage must be equal to the block
 * size or must be a whole numbered multiple of the block size. The first block always contains information
 * about the Section System, such as version, block size, number of blocks, and so on. The other blocks can
 * be either free blocks, section heads, or data blocks.
 *
 * ## Wear leveling
 * The number of times each storage block has been written is recorded, but the current implementation of class
 * @ref EEPROMSectionSystem does not perform any wear-leveling.
 *
 * ## Organization of free blocks
 * Free blocks are organized in a single linked list. Blocks are appended to the end of the list and
 * removed from the head of the list.
 *
 * ## Structure of Sections
 * Each section is made up by a section head block and at least one data block, even if zero bytes are stored
 * inside the section. This is required to recover the Section System after a power-fail or brown-out condition
 * during renaming of a section containing zero bytes.
 * Section heads are equipped with a version number that is required to recover the Section System after a
 * power-fail/brown-out condition during overwriting or renaming of a section.
 *
 * ## Thread-safety
 * The API of the Section System is thread-safe. Note that if multiple write operations are in process when
 * a brown out occurs, then the data modified by each write operation is recovered to the state
 * _before_ the operation or _after_ the operation. It is not the case that _all_ data modified by _all_ the
 * threads is recovered to the same state (e.g. before the operations).
 */
class EEPROMSectionSystem: public IFileStorage
{
    friend class internal::SectionReader;
    friend class internal::SectionWriter;

  public:
    /// Minimum supported block size of the underlying storage in bytes.
    static size_t const MinimumBlockSize = internal::MinimumBlockSize;

    /// Maximum supported block size of the underlying storage in bytes.
    static size_t const MaximumBlockSize = internal::MaximumBlockSize;

    /// Minimum required number of blocks in the underlying storage.
    static size_t const MinimumNbOfBlocks = internal::MinimumNbOfBlocks;

    /// Maximum supported number of blocks in the underlying storage.
    static size_t const MaximumNbOfBlocks = internal::MaximumNbOfBlocks;

    /// Version of the Section System (not version of the implementation).
    static uint16_t const Version = 0x0002U;


    /// States of the Section System.
    enum class States
    {
      not_mounted,  ///<Section System is not mounted.
      ro_mount,     ///<Section System is mounted for read-only access.
      checking,     ///<Section System is currently checked for errors (@ref MountStep2()).
      mounted,      ///<Section System is mounted for full read/write-access.
      defect        ///<Section System is defect and needs check (call to @ref MountStep2()).
    };


    EEPROMSectionSystem(StdIf::IRandomAccessStorage& _storage, uint32_t const _startAddressInStorage, size_t const _sizeInStorage);
    EEPROMSectionSystem(EEPROMSectionSystem const &) = delete;
    EEPROMSectionSystem(EEPROMSectionSystem &&) = delete;
    ~EEPROMSectionSystem(void);

    EEPROMSectionSystem& operator=(EEPROMSectionSystem const &) = delete;
    EEPROMSectionSystem& operator=(EEPROMSectionSystem &&) = delete;


    static char const * States2String(States const state);


    States GetState(void) const;
    void Format(uint16_t const desiredBlockSize);
    void MountStep1(void);
    void MountStep2(void);
    void Unmount(void);

    // --> IFileStorage
    std::unique_ptr<Stream::IStreamReader> Open(std::string const & name) override;
    std::unique_ptr<Stream::IStreamWriter> Create(std::string const & name, bool const overwriteIfExisting) override;
    void Delete(std::string const & name) override;
    void Rename(std::string const & currName, std::string const & newName) override;

    std::list<std::string> Enumerate(void) const override;
    size_t DetermineSize(std::string const & name, size_t * const pTotalSize) const override;
    size_t GetFreeSpace(void) const override;
    // <-- IFileStorage

  private:
    /// Mutex used to make the API thread-safe.
    /** The mutex is required for accessing the following attributes:
        - @ref state
        - @ref sectionLockManager
        - @ref storage :\n
          + Reading any block
          + Writing any block
          + Writing multiple blocks which make up a transaction, e.g. rename/overwrite a section or
            manipulation of free-block lists
          + any manipulation on free blocks; @ref storage content must be kept in sync with
            @ref nFreeBlocks, @ref freeBlockListHeadIdx, and @ref freeBlockListEndIdx.
        - @ref nFreeBlocks
        - @ref freeBlockListHeadIdx
        - @ref freeBlockListEndIdx. */
    mutable osal::Mutex mutex;

    /// Current state of the Section System.
    /** @ref mutex is required. */
    States state;

    /// Section Lock Manager.
    /** @ref mutex is required.
        Any modifying operation on @ref storage (creation, modification, deletion of sections)
        requires acquisition of write-locks on all involved sections before the operation starts.\n
        Any non-modifying operation on @ref storage (open sections for reading) requires acquisition
        of a read-lock for the section before the operation starts. */
    mutable ResourceManagement::Objects::SmallDynamicNamedRWLock sectionLockManager;

    /// Block-level access to the storage the @ref EEPROMSectionSystem is working on.
    /** @ref mutex is required for reading and writing blocks.\n
        Blocksize is only allowed to be changed during mount or formatting when the Section System is in
        state @ref States::not_mounted. */
    internal::BlockAccessor storage;

    /// Number of free blocks in @ref storage.
    /** @ref mutex is required. */
    uint16_t nFreeBlocks;

    /// Index of the first storage block in the list of free storage blocks.
    /** @ref mutex is required.\n
        Free blocks are removed from the head of the list.\n
        If @ref nFreeBlocks is zero, then this is NOBLOCK. */
    uint16_t freeBlockListHeadIdx;

    /// Index of the last storage block in the list of free storage blocks.
    /** @ref mutex is required.\n
        Free blocks are appended to the end of the list.\n
        If @ref nFreeBlocks is zero, then this is NOBLOCK. */
    uint16_t freeBlockListEndIdx;


    // mount support
    void Mount_LoadAndCheckSecSysInfoBlock(void* const pMem) const;
    void Mount_ProcessFreeBlock(uint16_t const currIndex, void* const pMem, container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks);
    void Mount_ProcessSectionHead(uint16_t currIndex, void* const pMem, char* const pSecName, container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks) const;
    void Mount_CheckLastFreeBlock(void* const pMem);
    void Mount_CollectGarbageBlocks(void* const pMem, container::BitField const & BfUsedUnusedBlocks);
    void Mount_SetDNKYtoUsed(container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks) const;
    void Mount_SetDNKYtoGarbage(container::BitField& BfUsedUnusedBlocks, container::BitField& BfGarbageBlocks) const;

    // management of sections
    bool CheckSectionName(std::string const & s) const;
    uint16_t LoadNextBlockOfSection(void* const pMem) const;
    uint16_t FindAnySectionHead(uint16_t const startBlockIndex, void* const pMem) const;
    uint16_t FindSectionHead(uint16_t const startBlockIndex, char const * const name, uint8_t const hash, void* const pMem) const;
    uint16_t FindSectionHeadByHash(uint16_t const startBlockIndex, uint8_t const hash) const;
    uint16_t FindSectionHeadByNextBlock(uint16_t const startBlockIndex, uint16_t const nextBlock, void* const pMem) const;

    // management of free blocks
    internal::FreeBlockListBackup GetFreeBlockListBackup(void) const noexcept;
    void RewindFreeBlockLists(internal::FreeBlockListBackup const & backup) noexcept;
    uint16_t LoadNextFreeBlock(void* const pMem) const;
    void AddChainOfBlocksToListOfFreeBlocks(uint16_t const startIndex, uint16_t const reservedBlockIndex, void* const pMem, bool const mutexLocked);
    void AddBlockToListOfFreeBlocks(uint16_t const blockIndex, uint32_t const * const pCurrTotalNbOfWrites, bool const mutexLocked);
    void AddBlocksToListOfFreeBlocks(uint16_t const * const pBlockIndexList, uint16_t const n, bool const mutexLocked);
    uint16_t GetBlockFromListOfFreeBlocks(uint32_t* const pTotalNbOfWrites, bool const mutexLocked);
    bool GetBlocksFromListOfFreeBlocks(uint16_t* pBlockIndexList, uint16_t const n, bool const mutexLocked);

    // section reader/writer support
    void StoreBlock(uint16_t const blockIndex, void* const pMem, bool const mutexLocked);
};

} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EEPROMSECTIONSYSTEM_HPP_
