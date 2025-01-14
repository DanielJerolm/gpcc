/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include <gpcc/raii/unique_c_ptr.hpp>
#include <cassert>

namespace gpcc {
namespace raii {

static_assert(sizeof(void*) == sizeof(unique_c_ptr<void>), "gpcc::raii::unique_c_ptr has unexpected size.");

} // namespace raii
} // namespace gpcc
