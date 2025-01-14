/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#ifndef UNIQUE_C_PTR_HPP_202501121951
#define UNIQUE_C_PTR_HPP_202501121951

#include <memory>
#include <stdlib.h>

namespace gpcc {
namespace raii {

namespace internal
{
  class DeleterUsingFree
  {
    public:
      template <typename T>
      void operator()(T* const p) const noexcept
      {
        std::free(const_cast<std::remove_const_t<T>*>(p));
      }
  };
}

/**
 * \ingroup GPCC_RAII
 * \brief Smart pointer managing ownership of storage that must be released via `free()`.
 *
 * If you have to invoke C APIs, then these APIs may allocate memory from the heap using `malloc()` and pass it back to
 * the caller. Typically ownership also moves to the caller, so the caller is responsible to finally release the memory.
 *
 * Memory allocated via `malloc()` and friends must be released via `free()`. `std::unique_ptr` cannot be used as a
 * container, because it finally passes the guarded memory to `delete`.
 *
 * However to allow for appliance of exception safe programming and RAII patterns, GPCC provides
 * `gpcc::raii::unique_c_ptr` as a specialization of `std::unique_ptr` that uses `free()` to release guarded memory.
 *
 * `gpcc::raii::unique_c_ptr` can be used to manage ownership of memory allocated via `malloc()`. The usage is the same
 * like `std::unique_ptr`:
 * ~~~{.cpp}
 * #include <gpcc/raii/unique_c_ptr.hpp>
 *
 * void SomeFunc(void)
 * {
 *   gpcc::raii::unique_c_ptr<uint32> spData(Library_written_in_C_CreateU32Data());
 *
 *   // spData can be used like an std::unique_ptr. E.g. get() can be used to obtain a raw pointer:
 *   Library_written_in_C_Sum(spData.get());
 *
 *   // When leaving the scope, spData will release the managed memory using "free".
 * }
 * ~~~
 * - - -
 *
 * \tparam T
 * Data type referenced by the pointer.\n
 * Example: Use `uint8_t` if `unique_c_ptr` shall behave like `uint8_t*`.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
template <typename T>
using unique_c_ptr = std::unique_ptr<T, internal::DeleterUsingFree>;

} // namespace raii
} // namespace gpcc

#endif // UNIQUE_C_PTR_HPP_202501121951
