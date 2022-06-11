/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

namespace gpcc {
namespace dsp {
namespace filter {

/**
 * \brief Constructor. Integrator at zero, output false.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
template <class T, T MAX, T LOWERTHR, T UPPERTHR>
IntegralFilter<T, MAX, LOWERTHR, UPPERTHR>::IntegralFilter(void) noexcept
: integrator(0U)
, output(0U)
{
  // compile-time checks on template parameters
  static_assert(MAX      >  0U,             "Check constraints on template parameters!");
  static_assert(LOWERTHR <= UPPERTHR + 1U,  "Check constraints on template parameters!");
  static_assert(LOWERTHR <= MAX,            "Check constraints on template parameters!");
  static_assert(UPPERTHR <  MAX,            "Check constraints on template parameters!");
}

/**
 * \brief Clears the filter. The integrator is set to zero, output is cleared (false).
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
template <class T, T MAX, T LOWERTHR, T UPPERTHR>
void IntegralFilter<T, MAX, LOWERTHR, UPPERTHR>::Clear(void) noexcept
{
  integrator = 0U;
  output     = 0U;
}

/**
 * \brief Samples the filter.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param input
 * State of the input signal.
 * \return
 * Output signal.
 */
template <class T, T MAX, T LOWERTHR, T UPPERTHR>
bool IntegralFilter<T, MAX, LOWERTHR, UPPERTHR>::Sample(bool const input) noexcept
{
  if (input)
  {
    if (integrator != MAX)
    {
      integrator++;
      if (integrator > UPPERTHR)
        output = 1U;
    }
  }
  else
  {
    if (integrator != 0U)
    {
      integrator--;
      if (integrator < LOWERTHR)
        output = 0U;
    }
  }

  return (output != 0U);
}

} // namespace filter
} // namespace dsp
} // namespace gpcc
