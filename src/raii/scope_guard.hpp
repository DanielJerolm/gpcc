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

#ifndef SCOPE_GUARD_HPP_201612311341
#define SCOPE_GUARD_HPP_201612311341

#include "gpcc/src/osal/Panic.hpp"

namespace gpcc {
namespace raii {

/**
 * \ingroup GPCC_RAII
 * \brief Executes a given lambda when leaving the current scope.
 *
 * # Description
 * This class is intended to be used to execute code performing roll-back or clean-up operations when execution leaves
 * the scope in which this class has been instantiated. The clean-up code is executed by this class' destructor
 * (RAII-pattern).
 *
 * Usually the current scope is left due to normal program flow, but it can also be left due to an exception or due to
 * thread cancellation (usually implemented using a special type of exception).
 *
 * If execution of the roll-back code is no longer required, then a call to `Dismiss()` will suppress invocation of the
 * lambda when the scope in which this has been instantiated is left.
 *
 * # Usage
 * _Class @ref ScopeGuard cannot be instantiated directly._
 *
 * One has to use either the function `make_ScopeGuard(...)` or the macro `ON_SCOPE_EXIT(name)`, which in turn uses the
 * helper class @ref ScopeGuardCreator to create a @ref ScopeGuard instance. Usually you will use the macros
 * `ON_SCOPE_EXIT(name)` and `ON_SCOPE_EXIT_DISMISS(name)`.
 *
 * ## ON_SCOPE_EXIT and ON_SCOPE_DISMISS
 * The macro `ON_SCOPE_EXIT(name) { code };` creates an @ref ScopeGuard instance identified by `name` on the stack in
 * the current scope. When the scope is left, then `code` will be executed, except `ON_SCOPE_EXIT_DISMISS(name)` is
 * invoked before leaving the scope.
 *
 * Note that `name` is optional and can be omitted if there is only one scope guard in a scope and if there is no
 * further scope guard in a nested scope.
 *
 * Example:
 * ~~~{.cpp}
 * void AddItem(void* pItem)
 * {
 *   // First add item to some kind of database. If this fails due to an exception or if the thread is cancelled, then
 *   // the item is not added to the data base (says doc of AddToDatabase(...)) and AddToLocalItemCollection(...) will
 *   // never be executed. Fine.
 *   AddToDatabase(pItem);
 *
 *   // If AddToLocalItemCollection(...) fails in the next step, then we have to remove the item from the data base, so
 *   // be prepared for exceptions or thread cancellation.
 *   ON_SCOPE_EXIT() { RemoveFromDataBase(pItem); };
 *
 *   // may throw
 *   AddToLocalItemCollection(pItem);
 *
 *   // We are still here, AddToLocalItemCollection(...) has succeeded. The rollback is no longer needed.
 *   ON_SCOPE_DISMISS();
 * }
 * ~~~
 *
 * As the example above shows, you do not need to provide a name for the scope guard if you only have one scope guard in
 * your current scope and no scope guard in a nested scope. If you have multiple ON_SCOPE_EXIT statements, then you have
 * to specify names:
 * ~~~{.cpp}
 * work1();
 * ON_SCOPE_EXIT(undo1) { cleanup1(); };
 *
 * work2();
 * ON_SCOPE_EXIT(undo2) { cleanup2(); };
 *
 * do_stuff_that_can_fail();
 *
 * ON_SCOPE_EXIT_DISMISS(undo2);
 * ON_SCOPE_EXIT_DISMISS(undo1);
 * ~~~
 *
 * ## make_ScopeGuard(...)
 * As an alternative to using the ON_SCOPE_EXIT() macro, you can also create a @ref ScopeGuard instance via
 * make_ScopeGuard(...).\n
 * Example:
 * ~~~{.cpp}
 * auto guard = make_ScopeGuard([&]() { cleanupStuff(); });
 * // ...some operations that might throw
 *
 * // if the cleanup is no longer needed, then you can invoke Dismiss():
 * guard.Dismiss();
 * ~~~
 *
 * _However, you should prefer ON_SCOPE_EXIT() and ON_SCOPE_DISMISS()._
 *
 * # Roll-back lambda requirements
 * The roll-back code must meet the following requirements:
 * - No uncaught exceptions must leave the roll-back code.
 * - Thread cancellation must not occur inside the roll-back code.\n
 *   To achieve this, either thread cancellation must be disabled, or the roll-back code must not contain any thread
 *   cancellation point, or your software design must ensure that there is no thread cancellation request pending while
 *   the roll-back code is executed.
 *
 * __If any requirement is violated, then the application will be terminated via Panic().__
 *
 * # Performance
 * In theory, GPCC's scope guard should not introduce any performance penalty.
 *
 * When compiling for x64 with gcc using -O3 or -Os, no performance penalty can be observed. However there is a
 * significant performance penalty when compiling for x64 with gcc and -O0.
 *
 * Curious guys can run the unit test cases "gpcc_raii_scope_guard_Tests.Performance_WithoutDismiss" and
 * "gpcc_raii_scope_guard_Tests.Performance_WithDismiss" multiple times for their specific target CPU architecture and
 * compiler using different optimization levels.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Each instance of class @ref ScopeGuard is intended to be used by one thread only.
 */
template<typename TLambda>
class ScopeGuard final
{
    template<typename AnyTLambda>
    friend auto make_ScopeGuard(AnyTLambda &&) noexcept -> ScopeGuard<AnyTLambda>;

    friend class ScopeGuardCreator;

  public:
    ScopeGuard(void) = delete;
    ScopeGuard(ScopeGuard const &) = delete;
    inline ScopeGuard(ScopeGuard && other) noexcept : lambda(std::move(other.lambda)), dismissed(other.dismissed) { other.dismissed = true; }
    inline ~ScopeGuard(void) { if (!dismissed) { try { lambda(); } catch (...) { gpcc::osal::Panic("gpcc::ScopeGuard: Rollback threw"); } } }

    ScopeGuard& operator=(ScopeGuard const &) = delete;
    ScopeGuard& operator=(ScopeGuard &&) = delete;

    inline void Dismiss(void) noexcept { dismissed = true; }

  private:
    TLambda lambda;   ///<The roll-back lambda.
    bool dismissed;   ///<Flag indicating if execution of the lambda is no longer necessary.

    inline explicit ScopeGuard(TLambda && _lambda) noexcept : lambda(std::move(_lambda)), dismissed(false) {}
};

/**
 * \fn ScopeGuard::ScopeGuard(ScopeGuard&& other)
 * \brief Move constructor.
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
 * The new @ref ScopeGuard instance is created from this. The scope guard referenced by this will be "dismissed", that
 * means it will not execute the roll-back lambda when it is destroyed. The responsibility to execute the roll-back
 * lambda moves to the new @ref ScopeGuard instance. If this scope guard is already "dismissed", then the new
 * constructed one will be dismissed too.
 */

/**
 * \fn ScopeGuard::~ScopeGuard()
 * \brief Destructor. This executes the roll-back code if the scope guard is not "dismissed".
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Depends on rollback-lambda.\n
 * _Thread cancellation must not occur during execution of the roll-back lambda._
 */

/**
 * \fn ScopeGuard::Dismiss()
 * \brief Dismisses the scope guard. The roll-back code will not be executed when the scope guard is released.
 *
 * This has no effect, if the scope guard is already dismissed.
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

/**
 * \fn ScopeGuard::ScopeGuard(TLambda && _lambda)
 * \brief Constructor. Private, can only be invoked by friends.
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
 * \param _lambda
 * The roll-back lambda that shall be executed when the ScopeGuard is destroyed. The referenced lambda will be moved
 * into the new @ref ScopeGuard instance.
 */

/**
 * \ingroup GPCC_RAII
 * \brief Helper function used to create an instance of class @ref ScopeGuard.
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
 * \param lambda
 * Lambda expression containing the clean-up/roll-back code to be executed by the @ref ScopeGuard instance when the
 * current scope is left. The referenced lambda will be moved into the new @ref ScopeGuard instance.
 *
 * \return
 * An instance of class @ref ScopeGuard. The @ref ScopeGuard instance uses its destructor to invoke the given lambda
 * expression when the current scope is left. This requires, that @ref ScopeGuard instances are only created on the
 * stack.\n
 * Example:
 * ~~~{.cpp}
 * void SomeFunction(void)
 * {
 *   auto guard = make_ScopeGuard([&]() { cleanupStuff(); });
 *   // ...
 * }
 * ~~~
 */
template<typename TLambda>
auto make_ScopeGuard(TLambda && lambda) noexcept -> ScopeGuard<TLambda>
{
  return ScopeGuard<TLambda>(std::move(lambda));
}

/**
 * \ingroup GPCC_RAII
 * \brief Helper class used to create instances of class @ref ScopeGuard inside the ON_SCOPE_EXIT() macro.
 *
 * You should not use this directly. Use the ON_SCOPE_EXIT() macro instead as described in the details of class
 * @ref ScopeGuard.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Each instance of class @ref ScopeGuardCreator is intended to be used by one thread only.
 */
class ScopeGuardCreator
{
  public:
    ScopeGuardCreator(void) noexcept = default;
    ScopeGuardCreator(ScopeGuardCreator const &) = delete;
    ScopeGuardCreator(ScopeGuardCreator&&) = delete;

    ScopeGuardCreator& operator=(ScopeGuardCreator const &) = delete;
    ScopeGuardCreator& operator=(ScopeGuardCreator&&) = delete;

    template<typename TLambda>
    inline ScopeGuard<TLambda> operator<< (TLambda && lambda) noexcept { return ScopeGuard<TLambda>(std::move(lambda)); }
};

/**
 * \ingroup GPCC_RAII
 * \brief Creates a [ScopeGuard](@ref gpcc::raii::ScopeGuard) instance `name` that executes the given code when the
 *        current scope is left.
 *
 * Example:
 * ~~~{.cpp}
 * work1();
 * ON_SCOPE_EXIT(undo1) { cleanup1(); };
 *
 * work2();
 * ON_SCOPE_EXIT(undo2) { cleanup2(); };
 *
 * commit();
 *
 * ON_SCOPE_EXIT_DISMISS(undo2);
 * ON_SCOPE_EXIT_DISMISS(undo1);
 * ~~~
 */
#define ON_SCOPE_EXIT(name) auto scopeGuard_##name = gpcc::raii::ScopeGuardCreator() << [&]()

/**
 * \ingroup GPCC_RAII
 * \brief Dismisses a scope guard previously created via @ref ON_SCOPE_EXIT(name).
 *
 * Example:
 * ~~~{.cpp}
 * work1();
 * ON_SCOPE_EXIT(undo1) { cleanup1(); };
 *
 * work2();
 * ON_SCOPE_EXIT(undo2) { cleanup2(); };
 *
 * commit();
 *
 * ON_SCOPE_EXIT_DISMISS(undo2);
 * ON_SCOPE_EXIT_DISMISS(undo1);
 * ~~~
 */
#define ON_SCOPE_EXIT_DISMISS(name) scopeGuard_##name.Dismiss()

} // namespace raii
} // namespace gpcc

#endif /* SCOPE_GUARD_HPP_201612311341 */
