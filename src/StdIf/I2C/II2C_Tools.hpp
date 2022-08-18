/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
