/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ISYNCSERIALIO_TO_ITERMINAL_HPP_201712292136
#define ISYNCSERIALIO_TO_ITERMINAL_HPP_201712292136

#include <gpcc/cli/ITerminal.hpp>
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
