/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TOOLS_HPP_202209062108
#define TOOLS_HPP_202209062108

#include "II2C_Master.hpp"

namespace gpcc  {
namespace StdIf {

bool CheckDescriptor(II2C_Master::stI2CTransferDescriptor_t const * pTD,
                     size_t const maxSingleTransferSize) noexcept;
size_t DetermineTotalTransferSize(II2C_Master::stI2CTransferDescriptor_t const * pTD,
                                  size_t const maxTotalTransferSize) noexcept;

} // namespace StdIf
} // namespace gpcc

#endif /* TOOLS_HPP_202209062108 */
