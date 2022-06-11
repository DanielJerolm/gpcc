/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#ifndef OBJECTVAR_WICB_WITHASM_HPP_202110012059
#define OBJECTVAR_WICB_WITHASM_HPP_202110012059

#include "gpcc/src/cood/ObjectVAR_wicb.hpp"
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTS
 * \brief [ObjectVAR_wicb](@ref gpcc::cood::ObjectVAR_wicb) with application specific data. This is for unit-test
 *        purposes only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectVAR_wicb_withASM final : public gpcc::cood::ObjectVAR_wicb
{
  public:
    ObjectVAR_wicb_withASM(void) = delete;
    ObjectVAR_wicb_withASM(std::string                const & _name,
                           gpcc::cood::DataType       const   _type,
                           uint16_t                   const   _nElements,
                           gpcc::cood::Object::attr_t const   _attributes,
                           void*                      const   _pData,
                           gpcc::osal::Mutex *        const   _pMutex,
                           tOnBeforeReadCallback      const & _onBeforeReadCallback,
                           tOnBeforeWriteCallback     const & _onBeforeWriteCallback,
                           tOnAfterWriteCallback      const & _onAfterWriteCallback,
                           std::vector<uint8_t>            && _appSpecMetaData);
    ObjectVAR_wicb_withASM(ObjectVAR_wicb_withASM const &) = delete;
    ObjectVAR_wicb_withASM(ObjectVAR_wicb_withASM &&) = delete;
    ~ObjectVAR_wicb_withASM(void) = default;

    ObjectVAR_wicb_withASM& operator=(ObjectVAR_wicb_withASM const &) = delete;
    ObjectVAR_wicb_withASM& operator=(ObjectVAR_wicb_withASM &&) = delete;

  private:
    /// Application specific meta data.
    std::vector<uint8_t> appSpecMetaData;

    // <-- gpcc::cood::Object
    size_t GetAppSpecificMetaDataSize(uint8_t const subIdx) const override;
    std::vector<uint8_t> GetAppSpecificMetaData(uint8_t const subIdx) const override;
    // --> gpcc::cood::Object
};

} // namespace cood
} // namespace gpcc_tests

#endif // OBJECTVAR_WICB_WITHASM_HPP_202110012059
