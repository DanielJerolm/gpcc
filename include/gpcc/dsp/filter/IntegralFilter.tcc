/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
