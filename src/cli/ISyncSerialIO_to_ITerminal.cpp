/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/cli/ISyncSerialIO_to_ITerminal.hpp>
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
 * Reference to an [ISyncSerialIO](@ref gpcc::stdif::ISyncSerialIO) interface that shall be used to talk
 * to the terminal.
 */
ISyncSerialIO_to_ITerminal::ISyncSerialIO_to_ITerminal(gpcc::stdif::ISyncSerialIO & _ssio)
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
