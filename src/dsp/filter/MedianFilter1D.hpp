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

#ifndef MEDIANFILTER1D_HPP_201701242016
#define MEDIANFILTER1D_HPP_201701242016

#include <cstdint>

namespace gpcc   {
namespace dsp    {
namespace filter {

/**
 * \ingroup GPCC_DSP_FILTER
 * \brief 1-dimensional median filter for small and fixed window sizes.
 *
 * # Initialization
 * If the filter has just been created or if @ref Clear() has just been invoked, then the filter is not initialized.
 * In this case, @ref Sample() will initialize all storage elements of the filter with parameter `value`.\n
 * The first value passed to @ref Sample() after creation or invocation of @ref Clear() therefore has much more weight
 * than subsequent values.\n
 * After WINDOWSIZE calls to @ref Sample(), the filter will behave normally and all samples have the same weight.
 *
 * - - -
 *
 * \tparam T
 * Data type of the processed data.
 *
 * \tparam WINDOWSIZE
 * Size of the median filter's window. This must be an odd value in the range [3;255].
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
template <class T, uint_fast8_t WINDOWSIZE>
class MedianFilter1D final
{
  public:
    MedianFilter1D(void) noexcept;

    void Clear(void) noexcept;
    T Sample(T const value) noexcept;

  private:
    /// Array representing the window of the median filter.
    /** The samples inside the window are sorted in ascending order. */
    T window[WINDOWSIZE];

    /// Array with the age of each window element.
    /** Beside the age, this field is used to indicate if the filer is initialized or not:\n
        Usually all elements in age are different, but if the filter is uninitialized, then
        age[0] and age[1] are equal. */
    uint8_t age[WINDOWSIZE];
};

} // namespace filter
} // namespace dsp
} // namespace gpcc

#include "MedianFilter1D.tcc"

#endif // MEDIANFILTER1D_HPP_201701242016
