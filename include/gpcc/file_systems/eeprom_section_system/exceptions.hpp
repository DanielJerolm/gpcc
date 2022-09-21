/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS_HPP_

#include <gpcc/file_systems/eeprom_section_system/EEPROMSectionSystem.hpp>
#include <stdexcept>
#include <string>
#include <cstdint>

namespace gpcc
{
namespace file_systems
{
namespace eeprom_section_system
{

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem or related classes if the section system's
 * data stored inside the storage is buggy.
 *
 * There are three errors derived from this:
 * - @ref CRCError
 * - @ref InvalidHeaderError
 * - @ref BlockLinkageError
 */
class DataIntegrityError: public std::runtime_error
{
  public:
    DataIntegrityError(std::string const & what_arg, uint16_t const _blockIndex);
    DataIntegrityError(char const * what_arg, uint16_t const _blockIndex);
    virtual ~DataIntegrityError(void) noexcept = default;

    inline uint16_t GetBlockIndex(void) const { return blockIndex; }

  private:
    uint16_t blockIndex;
};

inline DataIntegrityError::DataIntegrityError(std::string const & what_arg, uint16_t const _blockIndex)
: std::runtime_error(what_arg + " (Block " + std::to_string(_blockIndex) + ')')
, blockIndex(_blockIndex)
{
}
inline DataIntegrityError::DataIntegrityError(char const * what_arg, uint16_t const _blockIndex)
: std::runtime_error(std::string(what_arg) + " (Block " + std::to_string(_blockIndex) + ')')
, blockIndex(_blockIndex)
{
}

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem or related classes if an CRC error is
 * detected inside an storage block.
 */
class CRCError: public DataIntegrityError
{
  public:
    inline CRCError(uint16_t const _blockIndex): DataIntegrityError("CRC Error", _blockIndex) {};
    virtual ~CRCError(void) = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem or related classes if an storage block
 * with valid CRC but invalid header is detected.
 */
class InvalidHeaderError: public DataIntegrityError
{
  public:
    inline InvalidHeaderError(std::string const & what_arg, uint16_t const _blockIndex): DataIntegrityError(what_arg, _blockIndex) {};
    inline InvalidHeaderError(char const * what_arg, uint16_t const _blockIndex): DataIntegrityError(what_arg, _blockIndex) {};
    virtual ~InvalidHeaderError(void) = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem or related classes if the linkage between
 * two valid storage blocks is invalid (e.g. a free block cannot be linked to a section head).
 */
class BlockLinkageError: public DataIntegrityError
{
  public:
    inline BlockLinkageError(std::string const & what_arg, uint16_t const _blockIndex): DataIntegrityError(what_arg, _blockIndex) {};
    inline BlockLinkageError(char const * what_arg, uint16_t const _blockIndex): DataIntegrityError(what_arg, _blockIndex) {};
    virtual ~BlockLinkageError(void) = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem or related classes if two read operations
 * from the same address inside the storage result in different data.
 */
class VolatileStorageError : public std::runtime_error
{
  public:
    explicit VolatileStorageError(uint16_t const _blockIndex);
    virtual ~VolatileStorageError(void) noexcept = default;

    inline uint16_t GetBlockIndex(void) const { return blockIndex; }

  private:
    uint16_t blockIndex;
};

inline VolatileStorageError::VolatileStorageError(uint16_t const _blockIndex)
: std::runtime_error(std::string("Storage is volatile/unstable (Block ") + std::to_string(_blockIndex) + ')')
, blockIndex(_blockIndex)
{
}

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem if the current state is insufficient for an action.
 */
class InsufficientStateError : public std::logic_error
{
  public:
    inline InsufficientStateError(char const * const pFncName,
                                  EEPROMSectionSystem::States const current,
                                  EEPROMSectionSystem::States const required)
    : std::logic_error(std::string(pFncName) + ": Insufficient state (current " + EEPROMSectionSystem::States2String(current) + ", required " + EEPROMSectionSystem::States2String(required) + " or higher)") {}
    inline InsufficientStateError(std::string const & fncName,
                                  EEPROMSectionSystem::States const current,
                                  EEPROMSectionSystem::States const required)
    : std::logic_error(fncName + ": Insufficient state (current " + EEPROMSectionSystem::States2String(current) + ", required " + EEPROMSectionSystem::States2String(required) + " or higher)") {}
    inline InsufficientStateError(char const * const pFncName,
                                  EEPROMSectionSystem::States const current)
    : std::logic_error(std::string(pFncName) + ": Insufficient state (current " + EEPROMSectionSystem::States2String(current) + ')') {}
    inline InsufficientStateError(std::string const & fncName,
                                  EEPROMSectionSystem::States const current)
    : std::logic_error(fncName + ": Insufficient state (current " + EEPROMSectionSystem::States2String(current) + ')') {}
    inline InsufficientStateError(void) : std::logic_error("Insufficient state") {};
    virtual ~InsufficientStateError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem if an action requires all sections
 * to be closed, but there is still at least one section open.
 */
class NotAllSectionsClosedError : public std::runtime_error
{
  public:
    inline NotAllSectionsClosedError(void) : std::runtime_error("At least one section is still open") {};
    virtual ~NotAllSectionsClosedError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem during mount, if the size of the storage guessed from
 * the content of the Section System Info Block does not match the size of the storage.
 */
class StorageSizeMismatchError : public std::logic_error
{
  public:
    inline StorageSizeMismatchError(void) : std::logic_error("Section System Info Block does not match storage's size.") {};
    virtual ~StorageSizeMismatchError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem during mount, if the Section System Info Block
 * is invalid or not present.
 */
class BadSectionSystemInfoBlockError : public std::runtime_error
{
  public:
    inline BadSectionSystemInfoBlockError(void) : std::runtime_error("Section System Info Block bad or not present") {};
    virtual ~BadSectionSystemInfoBlockError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS
 * \brief Exception thrown by @ref EEPROMSectionSystem during mount, if the Section System Info Block
 * indicates an incompatible version of the Section System.
 */
class InvalidVersionError : public std::runtime_error
{
  public:
    inline InvalidVersionError(void) : std::runtime_error("Section System version incompatible") {};
    virtual ~InvalidVersionError(void) noexcept = default;
};

} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_EXCEPTIONS_HPP_
