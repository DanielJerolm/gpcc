/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef RAMBLOCK_HPP_201806202217
#define RAMBLOCK_HPP_201806202217

#include <gpcc/stdif/storage/IRandomAccessStorage.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <vector>

namespace gpcc      {

namespace stream    {
  class IStreamReader;
  class IStreamWriter;
}

namespace container {

/**
 * \ingroup GPCC_CONTAINER
 * \brief Class providing a piece of random accessible memory that can be used to store binary data and for
 *        emulation of storage devices whose drivers provide the @ref gpcc::stdif::IRandomAccessStorage interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class RAMBlock final : public gpcc::stdif::IRandomAccessStorage
{
  public:
    explicit RAMBlock(size_t const size);
    RAMBlock(size_t const size, uint8_t const v);
    RAMBlock(size_t const size, gpcc::stream::IStreamReader& sr);
    explicit RAMBlock(std::vector<uint8_t> const & data);
    explicit RAMBlock(std::vector<uint8_t> && data);
    explicit RAMBlock(RAMBlock const & other);
    explicit RAMBlock(RAMBlock && other);
    ~RAMBlock(void) = default;

    RAMBlock& operator=(RAMBlock const & rhv);
    RAMBlock& operator=(RAMBlock && rhv);
    RAMBlock& operator=(std::vector<uint8_t> const & rhv);
    RAMBlock& operator=(std::vector<uint8_t> && rhv);

    bool IsDirty(void) const;
    void SetDirtyFlag(void);
    void ClearDirtyFlag(void);
    std::vector<uint8_t> GetDataAndClearDirtyFlag(void);
    void WriteToStreamAndClearDirtyFlag(gpcc::stream::IStreamWriter& sw);

    // <-- gpcc::stdif::IRandomAccessStorage
    size_t GetSize(void) const override;
    size_t GetPageSize(void) const override;

    void Read(uint32_t address, size_t n, void* pBuffer) const override;
    void Write(uint32_t address, size_t n, void const * pBuffer) override;
    bool WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer) override;
    // --> gpcc::stdif::IRandomAccessStorage

  private:
    /// Mutex used to make the API thread safe.
    osal::Mutex mutable apiMutex;

    /// Storage for the encapsulated data.
    /** @ref apiMutex is required. */
    std::vector<uint8_t> storage;

    /// Dirty-flag.
    /** @ref apiMutex is required. */
    bool dirty;


    void CheckBounds(uint32_t const address, size_t const n) const;
};

} // namespace container
} // namespace gpcc

#endif // RAMBLOCK_HPP_201806202217
