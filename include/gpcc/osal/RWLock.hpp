/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef RWLOCK_HPP_201701271625
#define RWLOCK_HPP_201701271625

#ifdef OS_LINUX_X64
#include "universal/RWLock.hpp"
#endif

#ifdef OS_LINUX_X64_TFC
#include "universal/RWLock.hpp"
#endif

#ifdef OS_LINUX_ARM
#include "universal/RWLock.hpp"
#endif

#ifdef OS_LINUX_ARM_TFC
#include "universal/RWLock.hpp"
#endif

#ifdef OS_CHIBIOS_ARM
#include "universal/RWLock.hpp"
#endif

#endif // #ifndef RWLOCK_HPP_201701271625
