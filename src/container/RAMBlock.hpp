/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018 Daniel Jerolm

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

#ifndef RAMBLOCK_HPP_201806202217
#define RAMBLOCK_HPP_201806202217

#include "gpcc/src/StdIf/IRandomAccessStorage.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include <vector>

namespace gpcc      {

namespace Stream    {
  class IStreamReader;
  class IStreamWriter;
}

namespace container {

/**
 * \ingroup GPCC_CONTAINER
 * \brief Class providing a piece of random accessible memory that can be used to store binary data and for
 *        emulation of storage devices whose drivers provide the @ref gpcc::StdIf::IRandomAccessStorage interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class RAMBlock final : public gpcc::StdIf::IRandomAccessStorage
{
  public:
    explicit RAMBlock(size_t const size);
    RAMBlock(size_t const size, uint8_t const v);
    RAMBlock(size_t const size, gpcc::Stream::IStreamReader& sr);
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
    void WriteToStreamAndClearDirtyFlag(gpcc::Stream::IStreamWriter& sw);

    // <-- gpcc::StdIf::IRandomAccessStorage
    size_t GetSize(void) const override;
    size_t GetPageSize(void) const override;

    void Read(uint32_t address, size_t n, void* pBuffer) const override;
    void Write(uint32_t address, size_t n, void const * pBuffer) override;
    bool WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer) override;
    // --> gpcc::StdIf::IRandomAccessStorage

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
