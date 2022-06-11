/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef SRC_GPCC_STDIF_I2C_II2C_TOOLS_HPP_
#define SRC_GPCC_STDIF_I2C_II2C_TOOLS_HPP_

#include "II2C_Master_Driver.hpp"

namespace gpcc
{
namespace StdIf
{

bool CheckDescriptor(II2C_Master_Driver::stI2CTransferDescriptor_t const * pTD,
                     size_t const maxTransferSize) noexcept;
size_t DetermineTotalTransferSize(II2C_Master_Driver::stI2CTransferDescriptor_t const * pTD,
                                  size_t const maxTotalTransferSize) noexcept;

} // namespace StdIf
} // namespace gpcc

#endif /* SRC_GPCC_STDIF_I2C_II2C_TOOLS_HPP_ */
