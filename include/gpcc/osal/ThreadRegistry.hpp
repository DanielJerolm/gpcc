/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifndef THREADREGISTRY_HPP_201701271627
#define THREADREGISTRY_HPP_201701271627

#ifdef OS_CHIBIOS_ARM
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_EPOS_ARM
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_ARM
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_ARM_TFC
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_X64
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_X64_TFC
#include "universal/ThreadRegistry.hpp"
#endif

#endif // #ifndef THREADREGISTRY_HPP_201701271627
