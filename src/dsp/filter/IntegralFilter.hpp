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

#ifndef INTEGRALFILTER_HPP_201701232118
#define INTEGRALFILTER_HPP_201701232118

#include <cstdint>

namespace gpcc {
namespace dsp {
namespace filter {

/**
 * \ingroup GPCC_DSP_FILTER
 * \brief Integral filter for binary digital signals.
 *
 * _Implicit capabilities: default-construction, copy-construction, copy-assignment, move-construction, move-assignment_
 *
 *  # Applications
 *  This type of filter removes noise (e.g. contact chattering) from a digital signal.\n
 *  It can also be used to debounce digital signals.
 *
 *  # Operating Principle
 *  The filter is comprised of an integrator that either increments or decrements depending on the state of the
 *  binary input signal. The integrator is limited to the value range [0;MAX].
 *
 *  When the integrator's value exceeds (-> grater than) UPPERTHR, then the filter's output is asserted.\n
 *  When the integrator's value falls below (-> less than) LOWERTHR, then the filter's output is deasserted.\n
 *  LOWERTHR and UPPERTHR can be different values in order to implement a hysteresis.
 *
 *  Last but not least, T is the data type for the integrator's counter. Choose
 *  an unsigned integer type into which MAX, LOWERTHR, and UPPERTHR fit.
 *
 *  # Usage
 *  The input signal of the filter is intended to be sampled cyclically at a fixed and constant sampling rate.
 *  Each sample of the input signal should be passed to the filter for processing. With each sample passed to the
 *  filter, a "filtered" sample is retrieved.
 *
 *  Example:
 *  ~~~{.cpp}
 *  IntegralFilter<uint8_t, 100, 20, 80> filter;
 *
 *  void Sample(void)
 *  {
 *    // read input signal
 *    bool const gpio_input = ReadGPIO_XY();
 *
 *    // feed it through the filter
 *    bool const filtered_gpio_input = filter.Sample(gpio_input);
 *
 *    // do something with the filtered signal
 *    // ...
 *  }
 *
 *  void main(void)
 *  {
 *    while (true)
 *    {
 *      // wait for some kind of cyclic event, e.g. a timer that produces a tick each 10ms
 *      WaitForEvent();
 *
 *      Sample();
 *    }
 *  }
 *  ~~~
 *
 *  \tparam T
 *  Data type for the integrator's counter.\n
 *  Choose an unsigned integer type into which MAX, LOWERTHR, and UPPERTHR fit.\n
 *  Usually it is advantageous to choose the smallest data type that fits.
 *  \tparam MAX
 *  Upper limit for the integrator. Must be larger than zero.\n
 *  If the filter's input is `0` (false), then the integrator decrements, but not below zero.\n
 *  If the filter's input is `1` (true), then the integrator increments, but does not exceed MAX.\n
 *  \tparam LOWERTHR
 *  The filter's output is cleared to `0` (false), if the integrator's value is less than this after decrementing.\n
 *  This must be equal to or less than MAX and less than UPPERTHR + 1.
 *  \tparam UPPERTHR
 *  The filter's output is set to `1` (true), if the integrator's value is larger than this after incrementing.\n
 *  This must be less than MAX and equal to or larger than LOWERTHR - 1.
 *
 * ---
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
template <class T, T MAX, T LOWERTHR, T UPPERTHR>
class IntegralFilter final
{
  public:
    IntegralFilter(void) noexcept;

    void Clear(void) noexcept;
    bool Sample(bool const input) noexcept;

  private:
    /// Integrator's counter.
    /** This is always within [0; MAX]. */
    T integrator;

    /// Current state of the output.
    /** 0 = deasserted / false / `0`\n
        1 = asserted / true / `1` */
    uint8_t output;
};

} // namespace filter
} // namespace dsp
} // namespace gpcc

#include "IntegralFilter.tcc"

#endif // INTEGRALFILTER_HPP_201701232118
