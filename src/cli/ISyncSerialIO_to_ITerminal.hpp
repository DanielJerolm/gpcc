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

#ifndef ISYNCSERIALIO_TO_ITERMINAL_HPP_201712292136
#define ISYNCSERIALIO_TO_ITERMINAL_HPP_201712292136

#include "ITerminal.hpp"
#include "gpcc/src/StdIf/serial/ISyncSerialIO.hpp"

namespace gpcc {
namespace cli  {

/**
 * \ingroup GPCC_CLI
 * \brief Adapter providing an @ref ITerminal interface from an [ISyncSerialIO](@ref gpcc::StdIf::ISyncSerialIO) interface.
 *
 * This adapter is intended to be used to connect an @ref CLI component to a terminal via a serial connection,
 * e.g. a UART.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ISyncSerialIO_to_ITerminal final : public ITerminal
{
  public:
    explicit ISyncSerialIO_to_ITerminal(gpcc::StdIf::ISyncSerialIO & _ssio);
    ISyncSerialIO_to_ITerminal(ISyncSerialIO_to_ITerminal const &) = delete;
    ISyncSerialIO_to_ITerminal(ISyncSerialIO_to_ITerminal &&) = delete;
    ~ISyncSerialIO_to_ITerminal(void) = default;

    ISyncSerialIO_to_ITerminal& operator=(ISyncSerialIO_to_ITerminal const &) = delete;
    ISyncSerialIO_to_ITerminal& operator=(ISyncSerialIO_to_ITerminal &&) = delete;

  private:
    /// Reference to the ISyncSerialIO interface.
    gpcc::StdIf::ISyncSerialIO & ssio;

    // <-- ITerminal
    size_t Read(char * pBuffer, size_t bufferSize, uint16_t timeout_ms) override;
    void Flush(void) override;

    void Write(char const * pBuffer, size_t s) override;
    // --> ITerminal
};

} // namespace cli
} // namespace gpcc

#endif // ISYNCSERIALIO_TO_ITERMINAL_HPP_201712292136
