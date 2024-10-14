/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifndef TFC_TRAPS_HPP_202409172133
#define TFC_TRAPS_HPP_202409172133

#ifdef OS_CHIBIOS_ARM
#error "TFC not available for configured OSAL";
#endif

#ifdef OS_EPOS_ARM
#error "TFC not available for configured OSAL";
#endif

#ifdef OS_LINUX_ARM
#error "TFC not available for configured OSAL";
#endif

#ifdef OS_LINUX_ARM_TFC
#include "os/linux_arm_tfc/tfc_traps.hpp"
#endif

#ifdef OS_LINUX_X64
#error "TFC not available for configured OSAL";
#endif

#ifdef OS_LINUX_X64_TFC
#include "os/linux_x64_tfc/tfc_traps.hpp"
#endif

#endif // #ifndef TFC_TRAPS_HPP_202409172133
