/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019 Daniel Jerolm

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

#ifndef ISYNCSERIALIO_HPP_201712292059
#define ISYNCSERIALIO_HPP_201712292059

#include <cstdint>
#include <cstddef>

namespace gpcc  {
namespace StdIf {

/**
 * \ingroup GPCC_STDIF_SERIAL
 * \brief Interface for device drivers offering simple synchronous serial IO, e.g. UART devices.
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
    virtual ~ISyncSerialIO(void) = default;
};

/**
 * \fn virtual void ISyncSerialIO::TxSync
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
 * - Transmission may be incomplete (not all `size` bytes may have been transmitted)
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Transmission may be incomplete (not all `size` bytes may have been transmitted)
 *
 * - - -
 *
 * \param pData
 * Pointer to a buffer containing the data that shall be transmitted.
 * \param size
 * Number of bytes that shall be transmitted.\n
 * Zero is allowed.
 */

/**
 * \fn virtual size_t ISyncSerialIO::RxSync
 * \brief Receives data synchronously.
 *
 * This method blocks until either `size` bytes have been received or a timeout condition occurs.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of bytes might have been read from the UART and written into the buffer referenced by `pData`.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of bytes might have been read from the UART and written into the buffer referenced by `pData`.
 *
 * - - -
 *
 * \param pData
 * Pointer to a buffer into which the received data shall be written.
 * \param size
 * Maximum number of bytes that shall be received.\n
 * The size of the block of memory referenced by `pData` must be equal to this or larger than this.\n
 * If this is zero, then this method returns immediately returning zero.
 * \param pOverflow
 * Pointer to a boolean that will be set to true if an overflow occurred in the receiving path
 * of the UART hardware or inside the driver since the last call to this method.\n
 * This may be nullptr if this information is not interesting.
 * \param timeout_ms
 * Timeout in milliseconds.\n
 * This method either returns after reception of `size` bytes or after `timeout_ms` milliseconds have passed.\n
 * The timeout starts with the entry to this method.\n
 * The timeout is _restarted_ with each received character.\n
 * _Special values:_\n
 * 0  = no timeout (check for available data, then return immediately)\n
 * -1 = infinite timeout
 * \return
 * Number of bytes received.
 */

/**
 * \fn virtual void ISyncSerialIO::FlushRxBuffer
 * \brief Flushes all buffers in the receive path.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Flush may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Flush may be incomplete
 */

} // namespace StdIf
} // namespace gpcc

#endif // ISYNCSERIALIO_HPP_201712292059
