/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifndef THREAD_HPP_201701271627
#define THREAD_HPP_201701271627

#ifdef OS_CHIBIOS_ARM
#include "os/chibios_arm/Thread.hpp"
#endif

#ifdef OS_EPOS_ARM
#include "os/epos_arm/Thread.hpp"
#endif

#ifdef OS_LINUX_ARM
#include "os/linux_arm/Thread.hpp"
#endif

#ifdef OS_LINUX_ARM_TFC
#include "os/linux_arm_tfc/Thread.hpp"
#endif

#ifdef OS_LINUX_X64
#include "os/linux_x64/Thread.hpp"
#endif

#ifdef OS_LINUX_X64_TFC
#include "os/linux_x64_tfc/Thread.hpp"
#endif

#endif // #ifndef THREAD_HPP_201701271627
