/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
