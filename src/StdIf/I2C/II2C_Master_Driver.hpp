/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#ifndef SRC_GPCC_STDIF_I2C_II2C_MASTER_DRIVER_HPP_
#define SRC_GPCC_STDIF_I2C_II2C_MASTER_DRIVER_HPP_

#include <cstdint>
#include <cstddef>

namespace gpcc
{
namespace StdIf
{

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Common interface for I2C Master Drivers.
 *
 * Features/restrictions:
 * - This interface supports I2C master operation only.
 * - This interface supports single master operation on an I2C bus only.
 * - Drivers which implement this interface must recover the I2C bus after any error.
 */
class II2C_Master_Driver
{
  public:
    // (forward declaration)
    struct stI2CTransferDescriptor_t;

    /// I2C transfer descriptor.
    /** This structure describes one I2C transfer.\n
        Multiple I2C transfers can be chained in two ways:
        a) to make up a scattered read or write from/into the processor's memory
        b) to chain multiple I2C transactions using repeated start conditions

        In case of a), the two descriptors must assign the same I2C device (address) and
        the direction of the transfer must be the same. In case of a) a chained transfer
        must incorporate at least one byte of data (`nBytes` > 0).

        In case of b), the transactions are completely independent. The only restriction is
        that a read transfer must incorporate at least one byte of data.

        Transfers are chained using the descriptor's `pNext` pointer. The descriptor's
        `scattered` flag determines whether strategy a) or b) shall be applied.

        The `pNext` pointer of the last transfer descriptor must be nullptr to indicate the
        end of the transfer.

        Note that the elements `data` and `nBytes` are modified by the I2C master driver
        during a transfer. The other elements are guaranteed to be not modified. */
    typedef struct stI2CTransferDescriptor_t
    {
      /// I2C address of the device that shall be accessed.
      uint8_t address;

      /// Flag indicating the direction of the transfer.
      /** true  = write\n
          false = read */
      bool writeNotRead;

      /// Pointer to the data buffer.
      /** Note: During the transfer, this is incremented after each transferred byte.\n
          nullptr is allowed, if `nBytes` is zero. */
      uint8_t* data;

      /// Number of bytes that shall be transferred.
      /** Note: During the transfer, this is decremented after each transferred byte or each
          chunk of transferred bytes. */
      size_t nBytes;

      /// Pointer to the next transfer descriptor.
      /** nullptr indicates that this is the last transfer.
          If this is not nullptr, then the I2C driver will process the descriptor referenced by this
          after the current transfer has finished. `scattered` selects the strategy of processing
          the next descriptor:
          scattered = true : current I2C transfer is simply continued (scattered access to processor's RAM)\n
          scattered = false: a repeated start condition is created on the I2C bus */
      stI2CTransferDescriptor_t* pNext;

      /// Scattered-flag.
      /** This is only valid, if `pNext` is not nullptr.
          true  = The next descriptor is part of a scattered read/write. No repeated start condition will be created.\n
          false = A repeated start condition shall be created on the I2C bus before processing the next descriptor. */
      bool scattered;
    } stI2CTransferDescriptor_t;

    virtual ~II2C_Master_Driver(void) = default;

    virtual void LockBus(void) = 0;
    virtual uint32_t CalcMaxTransferTime(size_t nBytes, size_t nTransfers) const = 0;
    virtual bool WriteSync(uint8_t address, uint8_t const * data, size_t nBytes, uint32_t timeoutMS) = 0;
    virtual bool ReadSync(uint8_t address, uint8_t* data, size_t nBytes, uint32_t timeoutMS) = 0;
    virtual bool TransferSync(stI2CTransferDescriptor_t* pTransferDescriptor, uint32_t timeoutMS) = 0;
    virtual void UnlockBus(void) = 0;
};

/**
 * \fn void II2C_Master_Driver::LockBus(void)
 * \brief  Locks the I2C bus mutex.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */

/**
 * \fn uint32_t II2C_Master_Driver::CalcMaxTransferTime(size_t nBytes, size_t nTransfers) const
 * \brief Calculates the maximum time required to carry out a transfer (e.g. for setting up timeout values).
 *
 * __Thread safety:__\n
 * The I2C bus mutex must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master_Driver::LockBus()) and
 * [UnlockBus()](@ref gpcc::StdIf::II2C_Master_Driver::UnlockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param nBytes
 * Number of bytes that shall be transferred.
 * \param nTransfers
 * Number of transfers used to transfer the nBytes byte of data.
 * \return
 * Return value documentation.
 * Maximum time in ms required to transfer `nBytes` byte of data.\n
 * _This time does not include any potential delay introduced by the I2C slave due to_
 * _clock stretching._
 */

/**
 * \fn bool II2C_Master_Driver::WriteSync(uint8_t address, uint8_t const * data, size_t nBytes, uint32_t timeoutMS)
 * \brief Performs a synchronous write access to the I2C bus.
 *
 * This method blocks, until the transfer has finished or an error occurs.
 *
 * ---
 *
 * __Thread safety:__\n
 * The I2C bus mutex must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master_Driver::LockBus()) and
 * [UnlockBus()](@ref gpcc::StdIf::II2C_Master_Driver::UnlockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the transfer may be incomplete
 * - the I2C bus has been recovered
 *
 * Be aware of [I2CBusError](@ref gpcc::StdIf::I2CBusError) and exceptions derived from that.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the transfer may be incomplete
 * - stop condition will be generated
 *
 * ---
 *
 * \param address
 * I2C address of the device that shall be accessed.
 * \param data
 * Pointer to the data that shall be written.
 * \param nBytes
 * Number of bytes that shall be written.
 * \param timeoutMS
 * Timeout in ms for the whole transfer.
 * \return
 * true  = OK\n
 * false = No ACK on I2C bus received from slave.
 */

/**
 * \fn bool II2C_Master_Driver::ReadSync(uint8_t address, uint8_t* data, size_t nBytes, uint32_t timeoutMS)
 * \brief Brief documentation.
 *
 * This method blocks, until the transfer has finished or an error occurs.
 *
 * ---
 *
 * __Thread safety:__\n
 * The I2C bus mutex must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master_Driver::LockBus()) and
 * [UnlockBus()](@ref gpcc::StdIf::II2C_Master_Driver::UnlockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the transfer may be incomplete
 * - the I2C bus has been recovered
 *
 * Be aware of [I2CBusError](@ref gpcc::StdIf::I2CBusError) and exceptions derived from that.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the transfer may be incomplete
 * - stop condition will be generated
 *
 * ---
 *
 * \param address
 * I2C address of the device that shall be accessed.
 * \param data
 * Pointer to a buffer into which the data that has been read shall be written.
 * \param nBytes
 * Number of bytes that shall be read.
 * \param timeoutMS
 * Timeout in ms for the whole transfer.
 * \return
 * true  = OK\n
 * false = No ACK on I2C bus received from slave.
 */

/**
 * \fn bool II2C_Master_Driver::TransferSync(stI2CTransferDescriptor_t* pTransferDescriptor, uint32_t timeoutMS)
 * \brief Performs a single transfer or a series of transfers on the I2C bus.
 *
 * This method blocks, until all transfers have finished or an error occurs.
 *
 * ---
 *
 * __Thread safety:__\n
 * The I2C bus mutex must be locked when this method is executed.\n
 * Use [LockBus()](@ref gpcc::StdIf::II2C_Master_Driver::LockBus()) and
 * [UnlockBus()](@ref gpcc::StdIf::II2C_Master_Driver::UnlockBus()) to accomplish this.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the transfer may be incomplete
 * - the I2C bus has been recovered
 *
 * Be aware of [I2CBusError](@ref gpcc::StdIf::I2CBusError) and exceptions derived from that.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the transfer may be incomplete
 * - stop condition will be generated
 *
 * ---
 *
 * \param pTransferDescriptor
 * Pointer to a I2C transfer descriptor. nullptr is not allowed.\n
 * Multiple I2C transfers can be chained by using the descriptor's `pNext` pointer.\n
 * Note that the elements `data` and `nBytes` of the descriptor(s) are modified during the transfer(s).
 * \param timeoutMS
 * Timeout in ms over all transfers.
 * \return
 * true  = OK\n
 * false = No ACK on I2C bus received from slave.
 */

/**
 * \fn void II2C_Master_Driver::UnlockBus(void)
 * \brief Unlocks the I2C bus mutex.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */

} // namespace StdIf
} // namespace gpcc

#endif /* SRC_GPCC_STDIF_I2C_II2C_MASTER_DRIVER_HPP_ */
