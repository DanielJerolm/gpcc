/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021, 2025 Daniel Jerolm
*/

#ifndef OBJECT_VAR_WICB_HPP_202103121720
#define OBJECT_VAR_WICB_HPP_202103121720

#include <gpcc/cood/IObjectNotifiable.hpp>
#include <gpcc/cood/ObjectVAR.hpp>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD
 * \brief Same as [ObjectVAR](@ref gpcc::cood::ObjectVAR), but provides individual callbacks using separate functors
 *        instead of using [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) for notifications.
 *
 * Note that the RAM footprint of this class may be 90-100 bytes larger than the footprint of class @ref ObjectVAR on
 * some platforms.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectVAR_wicb : private IObjectNotifiable, public ObjectVAR
{
  public:
    ObjectVAR_wicb(void) = delete;
    ObjectVAR_wicb(std::string            const & name,
                   DataType               const   type,
                   uint16_t               const   nElements,
                   attr_t                 const   attributes,
                   void*                  const   pData,
                   gpcc::osal::Mutex *    const   pMutex,
                   tOnBeforeReadCallback  const & onBeforeReadCallback,
                   tOnBeforeWriteCallback const & onBeforeWriteCallback,
                   tOnAfterWriteCallback  const & onAfterWriteCallback);
    ObjectVAR_wicb(ObjectVAR_wicb const &) = delete;
    ObjectVAR_wicb(ObjectVAR_wicb &&) = delete;

    virtual ~ObjectVAR_wicb(void) = default;

    ObjectVAR_wicb& operator=(ObjectVAR_wicb const &) = delete;
    ObjectVAR_wicb& operator=(ObjectVAR_wicb &&) = delete;

  private:
    /// Functor to the before-read-callback.
    tOnBeforeReadCallback const onBeforeReadCallback_;

    /// Functor to the before-write-callback.
    tOnBeforeWriteCallback const onBeforeWriteCallback_;

    /// Functor to the after-write-callback.
    tOnAfterWriteCallback const onAfterWriteCallback_;


    // <-- IObjectNotifiable
    SDOAbortCode OnBeforeRead(gpcc::cood::Object const * pObj,
                              uint8_t const subindex,
                              bool const completeAccess,
                              bool const querySizeWillNotRead) override;

    SDOAbortCode OnBeforeWrite(gpcc::cood::Object const * pObj,
                               uint8_t const subindex,
                               bool const completeAccess,
                               uint8_t const valueWrittenToSI0,
                               void const * pData) override;

    void OnAfterWrite(gpcc::cood::Object const * pObj,
                      uint8_t const subindex,
                      bool const completeAccess) override;
    // --> IObjectNotifiable
};

} // namespace cood
} // namespace gpcc

#endif // OBJECT_VAR_WICB_HPP_202103121720