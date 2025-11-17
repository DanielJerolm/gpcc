/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
*/

#ifndef ISYNCSERIALIO_HPP_201712292059
#define ISYNCSERIALIO_HPP_201712292059

#include <cstddef>
#include <cstdint>

namespace gpcc  {
namespace stdif {

/**
 * \ingroup GPCC_STDIF_SERIAL
 * \brief Interface for device drivers offering simple synchronous serial I/O, e.g. UART peripherals.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ISyncSerialIO
{
  public:
    virtual void TxSync(void const * pData, size_t size) = 0;
    virtual size_t RxSync(void* pData, size_t size, bool* const pOverflow, int32_t const timeout_ms) = 0;
    virtual void FlushRxBuffer(void) = 0;

  protected:
    ISyncSerialIO(void) = default;
    ISyncSerialIO(ISyncSerialIO const &) = delete;
    ISyncSerialIO(ISyncSerialIO &&) = default;
    virtual ~ISyncSerialIO(void) = default;

    ISyncSerialIO& operator=(ISyncSerialIO const &) = delete;
    ISyncSerialIO& operator=(ISyncSerialIO &&) = delete;
};

/**
 * \fn void ISyncSerialIO::TxSync(void const * pData, size_t size)
 * \brief Transmits data synchronously.
 *
 * This method blocks until transmission has finished.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Transmission may be incomplete (not all @p size bytes may have been transmitted).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Transmission may be incomplete (not all @p size bytes may have been transmitted).
 *
 * - - -
 *
 * \param pData
 * Pointer to a buffer containing the data that shall be transmitted.
 *
 * \param size
 * Number of bytes that shall be transmitted.\n
 * Zero is allowed.
 */

/**
 * \fn size_t ISyncSerialIO::RxSync(void* pData, size_t size, bool* const pOverflow, int32_t const timeout_ms)
 * \brief Receives data synchronously.
 *
 * This method blocks until either @p size bytes have been received or a timeout condition occurs.
 *
 * There are two independent timeouts:
 * - Receive timeout (@p timeout_ms)\n
 *   This timeout starts immediately when entering this method and expires if a total of @p size bytes is not received
 *   within the given timespan.
 * - Inter-character timeout.\n
 *   The inter-character timeout is started upon reception of a byte and restarted upon reception of each further byte.
 *   - The inter-character timeout is only present, if it is supported by the driver offering this interface.
 *   - The inter-character timeout must be configured at the driver. It can neither be configured nor enabled and
 *     disabled through the @ref ISyncSerialIO interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of bytes may have been read from the device
 * - Undefined data may have been written into the buffer referenced by @p pData
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of bytes may have been read from the device
 * - Undefined data may have been written into the buffer referenced by @p pData
 *
 * - - -
 *
 * \param pData
 * Pointer to a buffer into which the received data shall be written.
 *
 * \param size
 * Maximum number of bytes that shall be received.\n
 * The size of the block of memory referenced by @p pData must be equal to or larger than this.\n
 * If this is zero, then this method returns immediately returning zero.
 *
 * \param pOverflow
 * Pointer to a boolean that will be set to `true` if an overflow occurred in the receiving path of the hardware or
 * inside the driver since the last call to this method.\n
 * This may be `nullptr` if this information is not interesting.
 *
 * \param timeout_ms
 * Receive timeout in milliseconds.\n
 * If this timeout expires, then this method will return even though @p size bytes of data have not yet been received.\n
 * The timeout starts when entering this method. The timeout is not restarted with each received character.\n
 * __Special values:__\n
 * 0  = no timeout (check for available data, then return immediately)\n
 * -1 = infinite timeout
 *
 * \return
 * Number of bytes received.\n
 * A value less than @p size indicates a receive timeout condition or an inter-character timeout condition.
 */

/**
 * \fn void ISyncSerialIO::FlushRxBuffer(void)
 * \brief Flushes all buffers in the receive path of the hardware and the driver.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Flush may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Flush may be incomplete.
 */

} // namespace stdif
} // namespace gpcc

#endif // ISYNCSERIALIO_HPP_201712292059
