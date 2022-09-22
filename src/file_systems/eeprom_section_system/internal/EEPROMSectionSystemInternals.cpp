/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "EEPROMSectionSystemInternals.hpp"

namespace gpcc
{
namespace file_systems
{
namespace eeprom_section_system
{
namespace internal
{

uint8_t CalcHash(char const * s) noexcept
/**
 * \brief Calculates a hash value for a null-terminated c-string.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param s
 * Pointer to a null-terminated c-string for which a hash shall be calculated.
 * \return
 * The hash calculated for the string `s`.
 */
{
  uint_fast8_t hash = 0;
  char c;
  while ((c = *s++) != 0)
  {
    hash = hash + static_cast<unsigned char>(c);
  }
  return static_cast<uint8_t>(hash);
}

} // namespace internal
} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc
