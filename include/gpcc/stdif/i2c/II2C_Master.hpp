/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef II2C_MASTER_HPP_202209062105
#define II2C_MASTER_HPP_202209062105

#include <gpcc/osal/MutexLocker.hpp>
#include <cstddef>
#include <cstdint>

namespace gpcc  {
namespace StdIf {

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Common interface for I2C bus master drivers.
 *
 * Features/restrictions:
 * - This interface supports I2C master operation only.
 * - This interface supports single master operation on an I2C bus only.
 * - Drivers which implement this interface must recover the I2C bus after any bus error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class II2C_Master
{
  public:
    // (forward declaration)
    struct stI2CTransferDescriptor_t;

    /// I2C transfer descriptor.
    /** Multiple descriptors can be chained in two ways:
        a) To setup a scattered read or write comprising multiple data buffers but only one I2C transfer.
        b) To chain multiple I2C transfers using repeated start conditions.

        In case of a), all descriptors must access the same I2C device and the direction of all transfers must be the
        same. In case of a) each descriptor must incorporate at least one byte of data (`nBytes` > 0).

        In case of b), the I2C transfers are completely independent. Subsequent transfers are initiated using a
        repeated start condition on the I2C bus. Each transfer must incorporate at least one byte of data.

        a) and b) can be combined within the same chain of descriptors to setup multiple independent I2C transfers that
        use scattered buffers.

        Transfers are chained using the descriptor's `pNext` pointer. The descriptor's `scattered` flag determines
        whether mode a) or b) shall be applied when the next descriptor is processed.

        The `pNext` pointer of the last transfer descriptor must be nullptr to indicate the end of the chain.

        Note that the elements `pData` and `nBytes` may be modified by the I2C master driver during a transfer. The
        other elements are guaranteed to be not modified.
    */
    typedef struct stI2CTransferDescriptor_t
    {
      /// I2C address of the device that shall be accessed.
      uint8_t address;

      /// Flag indicating the direction of the transfer.
      /** true  = write\n
          false = read */
      bool writeNotRead;

      /// Pointer to the data buffer.
      /** Note: During the transfer, this may be modified by the I2C master driver. */
      void* pData;

      /// Number of bytes that shall be transferred.
      /** Note: During the transfer, this may be modified by the I2C master driver. */
      size_t nBytes;

      /// Pointer to the next transfer descriptor.
      /** nullptr indicates that this is the last transfer descriptor.\n
          If this is not nullptr, then the I2C master driver will process the descriptor referenced by this after
          processing of the current transfer descriptor has finished.\n
          `scattered` selects the mode for processing the next descriptor:\n
          true : The current I2C transfer is simply continued, but a different buffer (`pData`) is used.\n
          false: A repeated start condition is generated on the I2C bus to start a new I2C transfer. */
      stI2CTransferDescriptor_t* pNext;

      /// Scattered-flag.
      /** This is only valid, if `pNext` is not nullptr.\n
          true  = The next descriptor is part of a scattered read/write. No repeated start condition will be created.\n
          false = A repeated start condition shall be created on the I2C bus before processing the next descriptor. */
      bool scattered;
    } stI2CTransferDescriptor_t;

    virtual gpcc::osal::MutexLocker LockBus(void) = 0;

    virtual uint32_t CalcMaxTransferTime(size_t nBytes, size_t nTransfers) const = 0;
    virtual bool WriteSync(uint8_t address, void const * pData, size_t nBytes, uint32_t timeoutMS) = 0;
    virtual bool ReadSync(uint8_t address, void* pData, size_t nBytes, uint32_t timeoutMS) = 0;
    virtual bool TransferSync(stI2CTransferDescriptor_t* pTransferDescriptor, uint32_t timeoutMS) = 0;

  protected:
    II2C_Master(void) noexcept = default;
    II2C_Master(II2C_Master const &) = delete;
    II2C_Master(II2C_Master &&) noexcept = default;
    virtual ~II2C_Master(void) = default;

    II2C_Master& operator=(II2C_Master const &) = delete;
    II2C_Master& operator=(II2C_Master &&) noexcept = default;
};


/**
 * \fn void II2C_Master::LockBus(void)
 * \brief Locks the I2C bus.
 *
 * If the bus is already locked, then this will block until the bus is unlocked.
 *
 * \post  The I2C bus is locked by the calling thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * A @ref gpcc::osal::MutexLocker instance managing the aquired lock.
 */

/**
 * \fn uint32_t II2C_Master::CalcMaxTransferTime(size_t nBytes, size_t nTransfers) const
 * \brief Calculates the maximum time required to carry out a transfer (e.g. for setting up timeout values).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The I2C bus must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master::LockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param nBytes
 * Total number of bytes that shall be transferred.\n
 * In case of a chained transfer (either scattered or not), calculate the sum incorporating all single transfers.
 *
 * \param nTransfers
 * Number of I2C transfers used to transfer the `nBytes` byte of data. In case of multiple I2C transfers, the
 * repeated start conditions will be considered in the calculation.
 *
 * \return
 * Maximum time in ms required to transfer `nBytes` byte of data.\n
 * _The calculated value does not include any potential delay introduced by the I2C device due to clock stretching._
 */

/**
 * \fn bool II2C_Master::WriteSync(uint8_t address, void const * pData, size_t nBytes, uint32_t timeoutMS)
 * \brief Performs a synchronous write access to a device connected to the I2C bus.
 *
 * This method blocks, until the transfer has finished or an error occurs.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The I2C bus must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master::LockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The transfer may be incomplete.
 * - The I2C bus has been recovered if required.
 *
 * Be aware of [I2CBusError](@ref gpcc::StdIf::I2CBusError) and exceptions derived from that.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The transfer may be incomplete.
 * - A stop condition will be generated.
 *
 * - - -
 *
 * \param address
 * I2C address of the device that shall be accessed.
 *
 * \param pData
 * Pointer to the data that shall be written.
 *
 * \param nBytes
 * Number of bytes that shall be written.
 *
 * \param timeoutMS
 * Timeout in ms for the whole transfer.
 *
 * \retval true  OK.
 * \retval false No ACK on I2C bus received from slave.
 */

/**
 * \fn bool II2C_Master::ReadSync(uint8_t address, void* pData, size_t nBytes, uint32_t timeoutMS)
 * \brief Performs a synchronous read access to a device connected to the I2C bus.
 *
 * This method blocks, until the transfer has finished or an error occurs.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The I2C bus must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master::LockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The transfer may be incomplete.
 * - The I2C bus has been recovered if required.
 * - Data may have been written to the memory location referenced by `pData`.
 *
 * Be aware of [I2CBusError](@ref gpcc::StdIf::I2CBusError) and exceptions derived from that.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The transfer may be incomplete.
 * - A stop condition will be generated.
 * - Data may have been written to the memory location referenced by `pData`.
 *
 * - - -
 *
 * \param address
 * I2C address of the device that shall be accessed.
 *
 * \param pData
 * Pointer to a buffer into which the data that has been read shall be written.
 *
 * \param nBytes
 * Number of bytes that shall be read.
 *
 * \param timeoutMS
 * Timeout in ms for the whole transfer.
 *
 * \retval true  OK.
 * \retval false No ACK on I2C bus received from slave.
 */

/**
 * \fn bool II2C_Master::TransferSync(stI2CTransferDescriptor_t* pTransferDescriptor, uint32_t timeoutMS)
 * \brief Performs a single transfer or a series of transfers on the I2C bus.
 *
 * This method blocks, until all transfers have finished or an error occurs.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The I2C bus must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master::LockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The transfer may be incomplete.
 * - The I2C bus has been recovered if required.
 * - Data may have been written to the memory locations referenced by the transfer descriptors.
 * - The transfer descriptors may have been modified and __must not__ be reused.
 *
 * Be aware of [I2CBusError](@ref gpcc::StdIf::I2CBusError) and exceptions derived from that.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The transfer may be incomplete.
 * - A stop condition will be generated.
 * - Data may have been written to the memory locations referenced by the transfer descriptors.
 * - The transfer descriptors may have been modified and __must not__ be reused.
 *
 * - - -
 *
 * \param pTransferDescriptor
 * Pointer to an I2C transfer descriptor. nullptr is not allowed.\n
 * Multiple transfer descriptors can be chained using the descriptor's `pNext` pointer to setup a scattered access
 * to multiple buffers or to setup multiple I2C transfers using repeated start conditions.\n
 * Note that the elements `pData` and `nBytes` of the descriptor(s) may be modified during the transfer(s).
 *
 * \param timeoutMS
 * Timeout in ms over all transfers.
 *
 * \retval true  OK.
 * \retval false No ACK on I2C bus received from slave.
 */

} // namespace StdIf
} // namespace gpcc

#endif // II2C_MASTER_HPP_202209062105
