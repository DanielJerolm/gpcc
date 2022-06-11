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

#ifndef SRC_GPCC_STDIF_I2C_II2C_MASTER_DRIVER_EXCEPTIONS_HPP_
#define SRC_GPCC_STDIF_I2C_II2C_MASTER_DRIVER_EXCEPTIONS_HPP_

#include <stdexcept>

namespace gpcc
{

namespace StdIf
{

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Exception thrown if an I2C bus related error occurs. Base class for more detailed exceptions.
 */
class I2CBusError : public std::runtime_error
{
  public:
    inline I2CBusError(std::string const & msg) : std::runtime_error(msg) {};
    inline I2CBusError(char const * const msg) : std::runtime_error(msg) {};
    virtual ~I2CBusError(void) noexcept = default;
};

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Exception thrown if the timeout for an I2C bus transfer has expired.
 */
class I2CBusTimeout : public I2CBusError
{
  public:
    inline I2CBusTimeout(void) : I2CBusError("I2C Bus Timeout") {}
    virtual ~I2CBusTimeout(void) noexcept = default;
};

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Exception thrown if an (single) I2C bus master looses arbitration.
 */
class I2CBusArbitrationLost : public I2CBusError
{
  public:
    inline I2CBusArbitrationLost(void) : I2CBusError("I2C Arbitration lost") {}
    virtual ~I2CBusArbitrationLost(void) noexcept = default;
};

/**
 * \ingroup GPCC_STDIF_I2C
 * \brief Exception thrown if an unspecified I2C bus error occurred.
 */
class I2CBusGeneralError : public I2CBusError
{
  public:
    inline I2CBusGeneralError(void) : I2CBusError("I2C Bus General Error") {}
    virtual ~I2CBusGeneralError(void) noexcept = default;
};

} // namespace StdIf
} // namespace gpcc

#endif // SRC_GPCC_STDIF_I2C_II2C_MASTER_DRIVER_EXCEPTIONS_HPP_
