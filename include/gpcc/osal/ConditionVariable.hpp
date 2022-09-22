/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef CONDITIONVARIABLE_HPP_201701271622
#define CONDITIONVARIABLE_HPP_201701271622

#ifdef OS_LINUX_X64
#include "os/linux_x64/ConditionVariable.hpp"
#endif

#ifdef OS_LINUX_X64_TFC
#include "os/linux_x64_tfc/ConditionVariable.hpp"
#endif

#ifdef OS_LINUX_ARM
#include "os/linux_arm/ConditionVariable.hpp"
#endif

#ifdef OS_LINUX_ARM_TFC
#include "os/linux_arm_tfc/ConditionVariable.hpp"
#endif

#ifdef OS_CHIBIOS_ARM
#include "os/chibios_arm/ConditionVariable.hpp"
#endif

#endif // #ifndef CONDITIONVARIABLE_HPP_201701271622
