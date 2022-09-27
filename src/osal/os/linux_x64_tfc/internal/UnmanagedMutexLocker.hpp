/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#ifndef UNMANAGEDMUTEXLOCKER_HPP_201702252258
#define UNMANAGEDMUTEXLOCKER_HPP_201702252258

#include "UnmanagedMutex.hpp"

namespace gpcc {
namespace osal {
namespace internal {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \class UnmanagedMutexLocker UnmanagedMutexLocker.hpp "src/osal/os/linux_x64_tfc/internal/UnmanagedMutexLocker.hpp"
 * \brief Automatic locker/unlocker for unmanaged mutexes (class @ref UnmanagedMutex).
 *
 * This class provides the same functionality like class @ref MutexLocker, but this one is intended
 * to be used with class @ref UnmanagedMutex.
 */
class UnmanagedMutexLocker final
{
    friend class AdvancedUnmanagedMutexLocker;

  public:
    UnmanagedMutexLocker(void) = delete;
    explicit UnmanagedMutexLocker(UnmanagedMutex* const _pMutex);
    explicit UnmanagedMutexLocker(UnmanagedMutex& mutex);
    UnmanagedMutexLocker(UnmanagedMutexLocker const &) = delete;
    UnmanagedMutexLocker(UnmanagedMutexLocker&& other) noexcept;
    ~UnmanagedMutexLocker(void);

    UnmanagedMutexLocker& operator=(UnmanagedMutexLocker const &) = delete;
    UnmanagedMutexLocker& operator=(UnmanagedMutexLocker&&) = delete;

  private:
    /// Pointer to the @ref UnmanagedMutex managed by this class. nullptr = none (e.g. stolen by move constructor).
    UnmanagedMutex* pMutex;
};

/**
 * \brief Constructor. Creates an @ref UnmanagedMutexLocker instance and locks the given @ref UnmanagedMutex.
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
 * Pointer to the @ref UnmanagedMutex that shall be managed by this @ref UnmanagedMutexLocker. \n
 * The @ref UnmanagedMutex is locked when the @ref UnmanagedMutexLocker is instantiated and the @ref UnmanagedMutex
 * is released when the @ref UnmanagedMutexLocker is destroyed.\n
 * If this is nullptr, then the @ref UnmanagedMutexLocker will be passive.
 */
inline UnmanagedMutexLocker::UnmanagedMutexLocker(UnmanagedMutex* const _pMutex)
: pMutex(_pMutex)
{
  if (pMutex != nullptr)
    pMutex->Lock();
}

/**
 * \brief Constructor. Creates an @ref UnmanagedMutexLocker instance and locks the given @ref UnmanagedMutex.
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
 * Reference to the @ref UnmanagedMutex instance that shall be managed by this @ref UnmanagedMutexLocker. \n
 * The @ref UnmanagedMutex is locked when the @ref UnmanagedMutexLocker is instantiated and the @ref UnmanagedMutex
 * is released when the @ref UnmanagedMutexLocker is destroyed.
 */
inline UnmanagedMutexLocker::UnmanagedMutexLocker(UnmanagedMutex& mutex)
: pMutex(&mutex)
{
  pMutex->Lock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref UnmanagedMutex is moved from another @ref UnmanagedMutexLocker to the newly
 * constructed @ref UnmanagedMutexLocker.
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
 * The responsibility to release the @ref UnmanagedMutex is moved from the referenced @ref UnmanagedMutexLocker
 * `other` to the new constructed @ref UnmanagedMutexLocker. After moving, the @ref UnmanagedMutexLocker referenced
 * by `other` will behave passive.
 */
inline UnmanagedMutexLocker::UnmanagedMutexLocker(UnmanagedMutexLocker&& other) noexcept
: pMutex(other.pMutex)
{
  other.pMutex = nullptr;
}

inline UnmanagedMutexLocker::~UnmanagedMutexLocker(void)
/**
 * \brief Destructor. Unlocks the @ref UnmanagedMutex managed by the @ref UnmanagedMutexLocker instance.
 *
 * If the @ref UnmanagedMutexLocker instance is passive, then this does nothing.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref UnmanagedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (pMutex != nullptr)
    pMutex->Unlock();
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef UNMANAGEDMUTEXLOCKER_HPP_201702252258
#endif // #ifdef OS_LINUX_X64_TFC
