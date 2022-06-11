/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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

#ifndef SHAREDSTRING_HPP_202110291338
#define SHAREDSTRING_HPP_202110291338

#include <atomic>
#include <string>
#include <cstddef>

namespace gpcc   {
namespace string {

/**
 * \ingroup GPCC_STRING
 * \brief Wrapper for std::string objects that allows to share immutable std::string objects.
 *
 * The std::string class provided by STL does not apply copy-on-write and thus does not allow to share the underlying
 * memory. Applications that create many copies of an std::string object that is not intended to be modified may benefit
 * from using this class.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access to the same instance is safe.\n
 * Further multiple threads may access instances exclusively dedicated to each thread 100% independent from each other,
 * even if they share the same std::string object.
 */
class SharedString final
{
  public:
    SharedString(void) = delete;
    SharedString(char const * const pStr);
    SharedString(std::string const & s);
    SharedString(std::string && s);
    SharedString(SharedString const & other) noexcept;
    SharedString(SharedString && other) = delete;
    ~SharedString(void);

    SharedString& operator=(SharedString const & rhv) noexcept;
    SharedString& operator=(SharedString && rhv) noexcept;

    SharedString& operator=(char const * const rhv);
    SharedString& operator=(std::string const & rhv);
    SharedString& operator=(std::string && rhv);

    std::string const & GetStr(void) const noexcept;

  private:
    /// Container for the actual std::string-object.
    /// The container can be shared among multiple @ref SharedString instances.
    class Container final
    {
      public:
        Container(void) = delete;
        Container(char const * const pStr);
        Container(std::string const & _str);
        Container(std::string && _str) noexcept;
        Container(Container const &) = delete;
        Container(Container &&) = delete;
        ~Container(void);

        Container operator=(Container const &) = delete;
        Container operator=(Container &&) = delete;

        void IncRefCnt(void) noexcept;
        bool DecRefCnt(void) noexcept;

        std::string const & GetStr(void) const noexcept;

      private:
        /// Encapsulated string.
        std::string str;

        /// Reference count.
        std::atomic<size_t> refCnt;

        static char const * ValidateNotNullptr(char const * const p);
    };


    /// Shared container containing the shared std::string object.
    Container * pContainer;

    void DiscardContainer(void) noexcept;
};

} // namespace string
} // namespace gpcc

#endif // SHAREDSTRING_HPP_202110291338
