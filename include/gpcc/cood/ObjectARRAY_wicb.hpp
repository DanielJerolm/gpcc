/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef OBJECT_ARRAY_WICB_HPP_202103132155
#define OBJECT_ARRAY_WICB_HPP_202103132155

#include <gpcc/cood/IObjectNotifiable.hpp>
#include <gpcc/cood/ObjectARRAY.hpp>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD
 * \brief Same as [ObjectARRAY](@ref gpcc::cood::ObjectARRAY), but provides individual callbacks using separate functors
 *        instead of using [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) for notifications.
 *
 * Note that the RAM footprint of this class may be 90-100 bytes larger than the footprint of class @ref ObjectARRAY on
 * some platforms.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectARRAY_wicb : private IObjectNotifiable, public ObjectARRAY
{
  public:
    ObjectARRAY_wicb(void) = delete;
    ObjectARRAY_wicb(std::string            const & _name,
                     attr_t                 const   _attributesSI0,
                     uint8_t                const   _SI0,
                     uint8_t                const   _min_SI0,
                     uint8_t                const   _max_SI0,
                     DataType               const   _type,
                     attr_t                 const   _attributes,
                     void*                  const   _pData,
                     gpcc::osal::Mutex *    const   _pMutex,
                     tOnBeforeReadCallback  const & _onBeforeReadCallback,
                     tOnBeforeWriteCallback const & _onBeforeWriteCallback,
                     tOnAfterWriteCallback  const & _onAfterWriteCallback);
    ObjectARRAY_wicb(ObjectARRAY_wicb const &) = delete;
    ObjectARRAY_wicb(ObjectARRAY_wicb &&) = delete;

    virtual ~ObjectARRAY_wicb(void) = default;

    ObjectARRAY_wicb& operator=(ObjectARRAY_wicb const &) = delete;
    ObjectARRAY_wicb& operator=(ObjectARRAY_wicb &&) = delete;

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

#endif // OBJECT_ARRAY_WICB_HPP_202103132155