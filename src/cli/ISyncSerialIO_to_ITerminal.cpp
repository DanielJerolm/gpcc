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

#include "ISyncSerialIO_to_ITerminal.hpp"
#include <stdexcept>

namespace gpcc {
namespace cli  {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _ssio
 * Reference to an [ISyncSerialIO](@ref gpcc::StdIf::ISyncSerialIO) interface that shall be used to talk
 * to the terminal.
 */
ISyncSerialIO_to_ITerminal::ISyncSerialIO_to_ITerminal(gpcc::StdIf::ISyncSerialIO & _ssio)
: ITerminal()
, ssio(_ssio)
{
}

// <-- ITerminal
/// \copydoc ITerminal::Read
size_t ISyncSerialIO_to_ITerminal::Read(char * pBuffer, size_t bufferSize, uint16_t timeout_ms)
{
  if (bufferSize == 0U)
    throw std::invalid_argument("ISyncSerialIO_to_ITerminal::Read: bufferSize is zero");

  return ssio.RxSync(pBuffer, bufferSize, nullptr, timeout_ms);
}

/// \copydoc ITerminal::Flush
void ISyncSerialIO_to_ITerminal::Flush(void)
{
  ssio.FlushRxBuffer();
}

/// \copydoc ITerminal::Write
void ISyncSerialIO_to_ITerminal::Write(char const * pBuffer, size_t s)
{
  ssio.TxSync(pBuffer, s);
}
// --> ITerminal

} // namespace cli
} // namespace gpcc
