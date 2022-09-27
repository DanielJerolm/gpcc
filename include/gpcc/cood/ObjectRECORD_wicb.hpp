/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef OBJECT_RECORD_WICB_HPP_202103141647
#define OBJECT_RECORD_WICB_HPP_202103141647

#include <gpcc/cood/IObjectNotifiable.hpp>
#include <gpcc/cood/ObjectRECORD.hpp>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD
 * \brief Same as [ObjectRECORD](@ref gpcc::cood::ObjectRECORD), but provides individual callbacks using separate functors
 *        instead of using [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) for notifications.
 *
 * Note that the RAM footprint of this class may be 90-100 bytes larger than the footprint of class @ref ObjectRECORD on
 * some platforms.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectRECORD_wicb : private IObjectNotifiable, public ObjectRECORD
{
  public:
    ObjectRECORD_wicb(void) = delete;
    ObjectRECORD_wicb(std::string            const & _name,
                      uint8_t                const   _SI0,
                      void*                  const   _pStruct,
                      size_t                 const   _structsNativeSizeInByte,
                      gpcc::osal::Mutex*     const   _pMutex,
                      SubIdxDescr const *    const   _pSIDescriptions,
                      tOnBeforeReadCallback  const & _onBeforeReadCallback,
                      tOnBeforeWriteCallback const & _onBeforeWriteCallback,
                      tOnAfterWriteCallback  const & _onAfterWriteCallback);
    ObjectRECORD_wicb(ObjectRECORD_wicb const &) = delete;
    ObjectRECORD_wicb(ObjectRECORD_wicb &&) = delete;

    virtual ~ObjectRECORD_wicb(void) = default;

    ObjectRECORD_wicb& operator=(ObjectRECORD_wicb const &) = delete;
    ObjectRECORD_wicb& operator=(ObjectRECORD_wicb &&) = delete;

  private:
    /// Functor to the before-read-callback.
    tOnBeforeReadCallback const onBeforeReadCallback;

    /// Functor to the before-write-callback.
    tOnBeforeWriteCallback const onBeforeWriteCallback;

    /// Functor to the after-write-callback.
    tOnAfterWriteCallback const onAfterWriteCallback;


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

#endif // OBJECT_RECORD_WICB_HPP_202103141647