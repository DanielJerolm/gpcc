/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2022 Daniel Jerolm

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

namespace gpcc   {
namespace dsp    {
namespace filter {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
template <class T, uint_fast8_t WINDOWSIZE>
MedianFilter1D<T, WINDOWSIZE>::MedianFilter1D(void) noexcept
: window()
, age()
{
  static_assert(WINDOWSIZE % 2U == 1U, "WINDOWSIZE must be odd");
  static_assert(WINDOWSIZE >= 3U,      "WINDOWSIZE must be [3;255]");
}

/**
 * \brief Clears the filter.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
template <class T, uint_fast8_t WINDOWSIZE>
void MedianFilter1D<T, WINDOWSIZE>::Clear(void) noexcept
{
  age[0] = age[1];
}

/**
 * \brief Samples the filter.
 *
 * Note:\n
 * If the filter has just been created or if @ref Clear() has just been invoked, then the filter is not initialized.
 * In this case, @ref Sample() will initialize all storage elements of the filter with parameter `value`.\n
 * The first value passed to @ref Sample() after creation or invocation of @ref Clear() therefore has much more
 * weight than subsequent values.\n
 * After WINDOWSIZE calls to @ref Sample(), the filter will behave normally and all samples have the same weight.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * Input sample.
 *
 * \return
 * Output sample.
 */
template <class T, uint_fast8_t WINDOWSIZE>
T MedianFilter1D<T, WINDOWSIZE>::Sample(T const value) noexcept
{
  // filter already initialized?
  if (age[0] != age[1])
  {
    // (filter is initialized)

    // ------------------------------------------------------------
    // Step 1: Update age[] and look for oldest element.
    //         The oldest element will be the one that is replaced.
    // ------------------------------------------------------------
    int_fast16_t insertIdx = -1;
    for (int_fast16_t i = 0; i < static_cast<int_fast16_t>(WINDOWSIZE); i++)
    {
      if (age[i] != WINDOWSIZE-1U)
        age[i]++;
      else
        insertIdx = i;
    }

    // -----------------------------------------------------------------------
    // Step 2: Insert "value" into "window" and update the sorting of "window"
    // -----------------------------------------------------------------------
    while ((insertIdx > 0) && (window[insertIdx-1] > value))
    {
      window[insertIdx] = window[insertIdx-1];
      age[insertIdx]    = age[insertIdx-1];
      insertIdx--;
    }
    while ((insertIdx < static_cast<int_fast16_t>(WINDOWSIZE-1U)) && (window[insertIdx+1] < value))
    {
      window[insertIdx] = window[insertIdx+1];
      age[insertIdx]    = age[insertIdx+1];
      insertIdx++;
    }

    window[insertIdx] = value;
    age[insertIdx]    = 0U;

    // --------------------------------------------------------
    // Step 3: pick up the result from the center of the window
    // --------------------------------------------------------
    return window[WINDOWSIZE / 2U];
  } // if (age[0] != age[1])
  else
  {
    // (filter is not yet initialized)

    // initialize the window
    for (uint_fast16_t i = 0; i < WINDOWSIZE; i++)
    {
      window[i] = value;
      age[i]    = i;
    }

    return value;
  } // if (age[0] != age[1])... else...
}

} // namespace filter
} // namespace dsp
} // namespace gpcc
