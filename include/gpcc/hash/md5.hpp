/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2020 Daniel Jerolm
*/

#ifndef MD5_HPP_202008042210
#define MD5_HPP_202008042210

#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace hash {

std::vector<uint8_t> MD5Sum(void const * const pData, size_t const s);
std::vector<uint8_t> MD5Sum(std::vector<uint8_t> const & data);
std::vector<uint8_t> MD5Sum(std::vector<char> const & data);

}
}

#endif // MD5_HPP_202008042210