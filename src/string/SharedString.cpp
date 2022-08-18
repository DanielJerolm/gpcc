/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "SharedString.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace gpcc   {
namespace string {

/**
 * \brief Constructor. Creates a @ref SharedString instance by copying a null-terminated c-string.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pStr
 * Pointer to a null-terminated c-string. A deep copy of the string will be created.\n
 * nullptr is not allowed.
 */
SharedString::SharedString(char const * const pStr)
: pContainer(new Container(pStr))
{
}

/**
 * \brief Constructor. Creates a @ref SharedString instance by copying an std::string object.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to the std::string object that shall be copied.
 */
SharedString::SharedString(std::string const & s)
: pContainer(new Container(s))
{
}

/**
 * \brief Constructor. Creates a @ref SharedString instance by moving an std::string object into the new
 *        @ref SharedString instance.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Universal reference to the std::string object that shall be moved into the new @ref SharedString instance.
 */
SharedString::SharedString(std::string && s)
: pContainer(new Container(std::move(s)))
{
}

/**
 * \brief Copy constructor. Creates a @ref SharedString instance by copying an existing @ref SharedString object.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * Unmodifiable reference to the @ref SharedString object that shall be copied.
 */
SharedString::SharedString(SharedString const & other) noexcept
: pContainer(other.pContainer)
{
  pContainer->IncRefCnt();
}

/**
 * \brief Destructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
SharedString::~SharedString(void)
{
  DiscardContainer();
}

/**
 * \brief Copy assignment operator.\n
 *        Copy assigns the content of an existing @ref SharedString object to this instance.
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
 * \param rhv
 * Existing @ref SharedString object whose content shall be copy-assigned to this instance.
 *
 * \return
 * Reference to this.
 */
SharedString& SharedString::operator=(SharedString const & rhv) noexcept
{
  if (&rhv != this)
  {
    rhv.pContainer->IncRefCnt();

    DiscardContainer();
    pContainer = rhv.pContainer;
  }

  return *this;
}

/**
 * \brief Move assignment operator.\n
 *        Move assigns the content of an existing @ref SharedString object to this instance.
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
 * \param rhv
 * Existing @ref SharedString object whose content shall be move-assigned to this instance.\n
 * The referenced object will be left in a valid, but undefined state.
 *
 * \return
 * Reference to this.
 */
SharedString& SharedString::operator=(SharedString && rhv) noexcept
{
  if (&rhv != this)
  {
    std::swap(pContainer, rhv.pContainer);
  }

  return *this;
}

/**
 * \brief Copy assignment operator.\n
 *        Copy assigns a null-terminated c-string to this object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Pointer to a null-terminated c-string that shall be assigned to this @ref SharedString instance. A deep copy of the
 * string will be created.\n
 * nullptr is not allowed.
 *
 * \return
 * Reference to this.
 */
SharedString& SharedString::operator=(char const * const rhv)
{
  Container* const pNewContainer = new Container(rhv);

  DiscardContainer();
  pContainer = pNewContainer;

  return *this;
}

/**
 * \brief Copy assignment operator.\n
 *        Copy assigns an std::string to this object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Unmodifiable reference to an std::string object whose content shall be copy-assigned to this object.
 *
 * \return
 * Reference to this.
 */
SharedString& SharedString::operator=(std::string const & rhv)
{
  Container* const pNewContainer = new Container(rhv);

  DiscardContainer();
  pContainer = pNewContainer;

  return *this;
}

/**
 * \brief Move assignment operator.\n
 *        Move assigns an std::string to this object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Universal reference to an std::string object whose content shall be move-assigned to this object.
 *
 * \return
 * Reference to this.
 */
SharedString& SharedString::operator=(std::string && rhv)
{
  Container* const pNewContainer = new Container(std::move(rhv));

  DiscardContainer();
  pContainer = pNewContainer;

  return *this;
}

/**
 * \brief Offers read-only access to the shared std::string object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Unmodifiable reference to the encapsulated shared std::string object.
 */
std::string const & SharedString::GetStr(void) const noexcept
{
  return pContainer->GetStr();
}

/**
 * \brief Discards the container referenced by @ref pContainer properly.
 *
 * \post  @ref pContainer is undefined afterwards.
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
 */
void SharedString::DiscardContainer(void) noexcept
{
  if (pContainer->DecRefCnt())
    delete pContainer;
}

/**
 * \brief Constructor.\n
 *        Creates a container initialized with a copy of a null-terminated c-string and reference count one.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pStr
 * Pointer to a null-terminated c-string. A deep copy will be created.\n
 * nullptr is not allowed.
 */
SharedString::Container::Container(char const * const pStr)
: str(ValidateNotNullptr(pStr))
, refCnt(1U)
{
}

/**
 * \brief Constructor.\n
 *        Creates a container initialized with a copy of an std::string object and reference count one.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _str
 * Unmodifiable reference to an std::string object.
 */
SharedString::Container::Container(std::string const & _str)
: str(_str)
, refCnt(1U)
{
}

/**
 * \brief Constructor.\n
 *        Creates a container initialized with an std::string object using a move operation and reference count one.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _str
 * Universal reference to an std::string object.
 */
SharedString::Container::Container(std::string && _str) noexcept
: str(std::move(_str))
, refCnt(1U)
{
}

/**
 * \brief Destructor.
 *
 * \pre   The reference count of the container must be zero.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
SharedString::Container::~Container(void)
{
  if (refCnt != 0U)
    PANIC();
}

/**
 * \brief Increments the reference count.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void SharedString::Container::IncRefCnt(void) noexcept
{
  // Wrap-around/overflow of refCnt is not tested, because it is not possible.
  // Rationale:
  // refCnt represents the number of SharedString objects referencing to this Container instance. The size of a
  // SharedString instance is at least 32 or 64 bit. size_t has the same size. Even if size_t would be a signed type,
  // the number of Container instances that could be counted by refCnt exceeds the number of Container instances that
  // could actually be created in the available memory (max. 2^32 or 2^64).

  static_assert(sizeof(size_t) == sizeof(void*), "Assumption about impossible wrap-around of 'refCnt' (see above) "
                                                 "requires that 'size_t' has the same size as a pointer on the target "
                                                 "platform.");

  ++refCnt;
}

/**
 * \brief Decrements the reference count and tests for zero.
 *
 * \pre   The reference count must not be zero (not tested).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   New reference count is zero.
 * \retval false  New reference count is not zero.
 */
bool SharedString::Container::DecRefCnt(void) noexcept
{
  return (--refCnt == 0U);
}

/**
 * \brief Retrieves an unmodifiable reference to the encapsulated std::string object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Unmodifiable reference to the encapsulated std::string object.
 */
std::string const & SharedString::Container::GetStr(void) const noexcept
{
  return str;
}

/**
 * \brief Throws if `p` is nullptr, otherwise returns `p`.
 *
 * This is intended to be used by constructors.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   `p` is nullptr.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param p
 * Pointer that shall be tested for non-nullptr.
 *
 * \return
 * Value of `p`.
 */
char const * SharedString::Container::ValidateNotNullptr(char const * const p)
{
  if (p == nullptr)
    throw std::invalid_argument("SharedString::Container::ValidateNotNullptr: nullptr");

  return p;
}

} // namespace string
} // namespace gpcc
