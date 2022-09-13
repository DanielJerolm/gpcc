/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/stdif/i2c/tools.hpp>


namespace gpcc  {
namespace stdif {

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Tooling for drivers: Checks a list of I2C transfer descriptors for consistency.
 *
 * The following checks are performed on each descriptor inside the list of chained transfers:
 * - Invalid address
 * - Read access to global call address
 * - Data pointer nullptr
 * - Zero bytes
 * - Number of bytes for one descriptor exceeds capability of the I2C master
 * - pNext referencing to self
 *
 * The following checks are performed on scattered/chained descriptors:
 * - Same slave address
 * - Same transfer direction (read/write)
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pTD
 * Pointer to the first transfer descriptor.
 *
 * \param maxSingleTransferSize
 * Maximum size for a single I2C transfer or one scattered fragment of an I2C transfer supported by the driver's
 * implementation in byte.
 *
 * \retval true  Descriptor and all chained descriptors look OK.
 * \retval false Descriptor or a chained descriptor is invalid.
 */
bool CheckDescriptor(II2C_Master::stI2CTransferDescriptor_t const * pTD, size_t const maxSingleTransferSize) noexcept
{
  if (pTD == nullptr)
    return false;

  do
  {
    if (   ((pTD->address & 0x80) != 0U)
        || ((pTD->address == 0U) && (!pTD->writeNotRead))
        || (pTD->pData == nullptr)
        || (pTD->nBytes <= 0U)
        || (pTD->nBytes > maxSingleTransferSize)
        || (pTD->pNext == pTD))
    {
      return false;
    }

    if ((pTD->pNext != nullptr) && (pTD->scattered))
    {
      if ((pTD->pNext->address != pTD->address) ||
          (pTD->pNext->writeNotRead != pTD->writeNotRead))
      {
        return false;
      }
    }

    pTD = pTD->pNext;
  }
  while (pTD != nullptr);

  return true;
}

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Tooling for drivers: Determines the total size of a (potentially scattered) I2C transfer.
 *
 * This function walks through a list of chained transfer descriptors and accumulates the sizes of the transfers
 * until either the end of the list is reached or a transfer descriptor that requires a repeated start condition on the
 * I2C bus is encountered.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pTD
 * Pointer to the transfer descriptor.
 *
 * \param maxTotalTransferSize
 * Maximum total size for the I2C transfer supported by the driver's implementation in byte.
 *
 * \return
 * Total size of the I2C transfer in bytes.\n
 * If the total size exceeds `maxTotalTransferSize`, then `maxTotalTransferSize+1` will be returned.
 */
size_t DetermineTotalTransferSize(II2C_Master::stI2CTransferDescriptor_t const * pTD,
                                  size_t const maxTotalTransferSize) noexcept
{
  size_t size = 0U;

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

} // namespace stdif
} // namespace gpcc
