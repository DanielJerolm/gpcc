/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019 Daniel Jerolm

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

#ifndef STRING_CONVERSION_HPP_201905072210
#define STRING_CONVERSION_HPP_201905072210

#include <cstdint>
#include <string>

namespace gpcc {
namespace cood {

uint16_t StringToObjIndex(std::string const & s);
void StringToObjIndexAndSubindex(std::string const & s, uint16_t & idx, uint8_t & subIdx);

} // namespace cood
} // namespace gpcc

#endif // STRING_CONVERSION_HPP_201905072210
