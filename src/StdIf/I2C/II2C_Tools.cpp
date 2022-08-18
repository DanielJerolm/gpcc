/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "II2C_Tools.hpp"


namespace gpcc
{
namespace StdIf
{

bool CheckDescriptor(II2C_Master_Driver::stI2CTransferDescriptor_t const * pTD, size_t const maxTransferSize) noexcept
/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Tooling for drivers: Checks a list of I2C transfer descriptors for consistency.
 *
 * The following checks are performed on each descriptor inside the list of chained transfers:
 * - nullptr-pointer
 * - Invalid address
 * - Read access to global call address
 * - data pointer nullptr
 * - zero or negative number of bytes
 * - number of bytes exceeds capability of the I2C master
 * - pNext referencing to self
 *
 * The following checks are performed on scattered/chained descriptors:
 * - same slave address
 * - same transfer direction (read/write)
 * - at least one byte to transfer
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pTD
 * Pointer to the first transfer descriptor.
 * \param maxTransferSize
 * Maximum transfer size supported by the driver's implementation in byte.\n
 * This is not the total size, but the maximum size of a _single_ transfer.
 * \return
 * true  = descriptor and all chained descriptors look OK\n
 * false = descriptor or a chained descriptor is invalid
 */
{
  if (pTD == nullptr)
    return false;

  do
  {
    if ((pTD->address & 0x80) ||
        ((pTD->address == 0) && (!pTD->writeNotRead)) ||
        (pTD->data == nullptr) ||
        (pTD->nBytes <= 0) ||
        (pTD->nBytes > maxTransferSize) ||
        (pTD->pNext == pTD))
      return false;

    if ((pTD->pNext != nullptr) && (pTD->scattered))
    {
      if ((pTD->pNext->address != pTD->address) ||
          (pTD->pNext->writeNotRead != pTD->writeNotRead))
        return false;
    }

    pTD = pTD->pNext;
  }
  while (pTD);

  return true;
}
size_t DetermineTotalTransferSize(II2C_Master_Driver::stI2CTransferDescriptor_t const * pTD,
                                  size_t const maxTotalTransferSize) noexcept
/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Tooling for drivers: Determines the total size of a scattered transfer composed of multiple descriptors.
 *
 * This methods walks through a list of chained I2C transfer descriptors and accumulates the
 * sizes of the transfers until either the end of the list is reached or a transfer
 * descriptor that requires a restart-condition of the I2C bus is encountered.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pTD
 * Pointer to the transfer descriptor.
 * \param maxTotalTransferSize
 * Maximum total transfer size supported by the driver's implementation in byte.
 * \return
 * Total size of the transfer.\n
 * If the total size exceeds `maxTotalTransferSize`, then `maxTotalTransferSize+1` is returned.
 */
{
  size_t size = 0;

  while (true)
  {
    size += pTD->nBytes;

    if (size > maxTotalTransferSize)
      break;

    if ((pTD->pNext == nullptr) || (!pTD->scattered))
      return size;

    pTD = pTD->pNext;
  }

  return maxTotalTransferSize + 1U;
}

} // namespace StdIf
} // namespace gpcc
