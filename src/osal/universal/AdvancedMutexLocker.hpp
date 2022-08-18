/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#ifndef ADVANCEDMUTEXLOCKER_HPP_201701271629
#define ADVANCEDMUTEXLOCKER_HPP_201701271629

#include "../Mutex.hpp"
#include "MutexLocker.hpp"

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING GPCC_RAII
 * \copydoc MutexLocker
 */
class AdvancedMutexLocker final
{
  public:
    AdvancedMutexLocker(void) = delete;
    explicit AdvancedMutexLocker(Mutex* const _pMutex);
    explicit AdvancedMutexLocker(Mutex& mutex);
    AdvancedMutexLocker(AdvancedMutexLocker const &) = delete;
    AdvancedMutexLocker(AdvancedMutexLocker&& other) noexcept;
    AdvancedMutexLocker(MutexLocker&& other) noexcept;
    ~AdvancedMutexLocker(void);

    AdvancedMutexLocker& operator=(AdvancedMutexLocker const &) = delete;
    AdvancedMutexLocker& operator=(AdvancedMutexLocker&&) = delete;

    void Unlock(void) noexcept;
    void Relock(void);
    bool IsLocked(void) const noexcept;

  private:
    /// Pointer to the managed mutex. nullptr = no mutex.
    Mutex* pMutex;

    /// Lock-Flag.
    /** true =  Mutex locked.\n
        false = Mutex NOT locked. */
    bool locked;
};

/**
 * \brief Constructor. Creates an @ref AdvancedMutexLocker instance and locks the managed @ref Mutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _pMutex
 * Pointer to the @ref Mutex that shall be managed by this @ref AdvancedMutexLocker. \n
 * The @ref Mutex is locked when the @ref AdvancedMutexLocker is instantiated and the @ref Mutex is released
 * when the @ref AdvancedMutexLocker is destroyed.\n
 * If this is nullptr, then the @ref AdvancedMutexLocker will be passive and
 * any call to @ref Relock() and @ref Unlock() will be ignored.
 */
inline AdvancedMutexLocker::AdvancedMutexLocker(Mutex* const _pMutex)
: pMutex(_pMutex)
, locked(_pMutex != nullptr)
{
  if (locked)
    pMutex->Lock();
}

/**
 * \brief Constructor. Creates an @ref AdvancedMutexLocker instance and locks the managed @ref Mutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param mutex
 * Reference to the @ref Mutex that shall be managed by this @ref AdvancedMutexLocker. \n
 * The @ref Mutex is locked when the @ref AdvancedMutexLocker is instantiated and the @ref Mutex is released
 * when the @ref AdvancedMutexLocker is destroyed.
 */
inline AdvancedMutexLocker::AdvancedMutexLocker(Mutex& mutex)
: pMutex(&mutex)
, locked(true)
{
  pMutex->Lock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref Mutex is moved from another @ref AdvancedMutexLocker to the newly
 * constructed @ref AdvancedMutexLocker.
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * The responsibility to release the @ref Mutex is moved from the referenced @ref AdvancedMutexLocker `other` to
 * the new constructed @ref AdvancedMutexLocker. After moving, the @ref AdvancedMutexLocker referenced by `other`
 * will behave passive.
 */
inline AdvancedMutexLocker::AdvancedMutexLocker(AdvancedMutexLocker&& other) noexcept
: pMutex(other.pMutex)
, locked(other.locked)
{
  other.pMutex = nullptr;
  other.locked = false;
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref Mutex is moved from an @ref MutexLocker to the newly
 * constructed @ref AdvancedMutexLocker.
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * The responsibility to release the @ref Mutex is moved from the referenced @ref MutexLocker `other` to the
 * new constructed @ref AdvancedMutexLocker. After moving, the @ref MutexLocker referenced by `other` will
 * behave passive.
 */
inline AdvancedMutexLocker::AdvancedMutexLocker(MutexLocker&& other) noexcept
: pMutex(other.pMutex)
, locked(other.pMutex != nullptr)
{
  other.pMutex = nullptr;
}

/**
 * \brief Destructor. Unlocks the managed @ref Mutex if it is locked and if the @ref AdvancedMutexLocker
 * is not passive.
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
inline AdvancedMutexLocker::~AdvancedMutexLocker(void)
{
  if (locked)
    pMutex->Unlock();
}

inline bool AdvancedMutexLocker::IsLocked(void) const noexcept
/**
 * \brief Retrieves if this @ref AdvancedMutexLocker instance has its managed @ref Mutex currently locked.
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * true  = Mutex is locked.\n
 * false = Mutex is not locked.
 */
{
  return locked;
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef ADVANCEDMUTEXLOCKER_HPP_201701271629
#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
