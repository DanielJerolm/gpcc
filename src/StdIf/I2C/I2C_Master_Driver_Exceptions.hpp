/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
