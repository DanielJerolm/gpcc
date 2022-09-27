/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
