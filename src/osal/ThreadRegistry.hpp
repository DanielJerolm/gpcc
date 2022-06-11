/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#ifndef THREADREGISTRY_HPP_201701271627
#define THREADREGISTRY_HPP_201701271627

#ifdef OS_LINUX_X64
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_X64_TFC
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_ARM
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_LINUX_ARM_TFC
#include "universal/ThreadRegistry.hpp"
#endif

#ifdef OS_CHIBIOS_ARM
#include "universal/ThreadRegistry.hpp"
#endif

#endif // #ifndef THREADREGISTRY_HPP_201701271627
