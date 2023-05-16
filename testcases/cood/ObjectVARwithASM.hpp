/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef OBJECTVARWITHASM_HPP_202103212106
#define OBJECTVARWITHASM_HPP_202103212106

#include <gpcc/cood/ObjectVAR.hpp>
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTS
 * \brief [ObjectVAR](@ref gpcc::cood::ObjectVAR) with application specific data. This is for unit-test purposes only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectVARwithASM final : public gpcc::cood::ObjectVAR
{
  public:
    ObjectVARwithASM(void) = delete;
    ObjectVARwithASM(std::string                     const & _name,
                     gpcc::cood::DataType            const   _type,
                     uint16_t                        const   _nElements,
                     gpcc::cood::Object::attr_t      const   _attributes,
                     void*                           const   _pData,
                     gpcc::osal::Mutex *             const   _pMutex,
                     gpcc::cood::IObjectNotifiable * const   _pNotifiable,
                     std::vector<uint8_t>                 && _appSpecMetaData);
    ObjectVARwithASM(ObjectVARwithASM const &) = delete;
    ObjectVARwithASM(ObjectVARwithASM &&) = delete;
    ~ObjectVARwithASM(void) = default;

    ObjectVARwithASM& operator=(ObjectVARwithASM const &) = delete;
    ObjectVARwithASM& operator=(ObjectVARwithASM &&) = delete;

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

#endif // OBJECTVARWITHASM_HPP_202103212106
