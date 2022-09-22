/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef OBJECTVAR_WICB_WITHASM_HPP_202110012059
#define OBJECTVAR_WICB_WITHASM_HPP_202110012059

#include <gpcc/cood/ObjectVAR_wicb.hpp>
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
