/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#ifndef EXCEPTIONS_HPP_201809272122
#define EXCEPTIONS_HPP_201809272122

#include <gpcc/cood/data_types.hpp>
#include <stdexcept>
#include <string>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_EXCEPTIONS
 * \brief Exception thrown if a functionality does not support a particular @ref DataType.
 */
class DataTypeNotSupportedError : public std::logic_error
{
  public:
    /// The unsupported data type.
    DataType const dt;

    inline DataTypeNotSupportedError(DataType const _dt) : std::logic_error("Data type not supported."), dt(_dt) {};
    virtual ~DataTypeNotSupportedError(void) noexcept = default;
};

/**
 * \ingroup GPCC_COOD_EXCEPTIONS
 * \brief Exception thrown if a subindex is not existing or empty.
 */
class SubindexNotExistingError : public std::logic_error
{
  public:
    inline SubindexNotExistingError(void) : std::logic_error("Subindex is not existing or empty.") {};
    virtual ~SubindexNotExistingError(void) noexcept = default;
};

} // namespace cood
} // namespace gpcc

#endif // EXCEPTIONS_HPP_201809272122
