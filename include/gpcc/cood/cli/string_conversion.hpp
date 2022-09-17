/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#ifndef STRING_CONVERSION_HPP_201905072210
#define STRING_CONVERSION_HPP_201905072210

#include <string>
#include <cstdint>

namespace gpcc {
namespace cood {

uint16_t StringToObjIndex(std::string const & s);
void StringToObjIndexAndSubindex(std::string const & s, uint16_t & idx, uint8_t & subIdx);

} // namespace cood
} // namespace gpcc

#endif // STRING_CONVERSION_HPP_201905072210
