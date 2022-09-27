/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef IMULTIRODACLICLIENT_HPP_202106121949
#define IMULTIRODACLICLIENT_HPP_202106121949

#include <cstdint>

namespace gpcc {
namespace cood {

class IRemoteObjectDictionaryAccess;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Interface for class @ref MultiRODACLIClientBase and derived classes.
 *
 * This interface allows users of classes derived from class @ref MultiRODACLIClientBase to register and unregister RODA
 * interfaces. Classes derived from @ref MultiRODACLIClientBase offer CLI access to object dictionaries accessible
 * through registered RODA interfaces.
 *
 * This interface allows to register multiple different RODA interfaces at the same time. The RODA interfaces are
 * distinguished by 32bit IDs. The IDs are assigned during registration of the RODA interfaces, and the IDs are expected
 * as arguments by the CLI commands.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IMultiRODACLIClient
{
  public:
    virtual void Register(IRemoteObjectDictionaryAccess & rodaItf, uint32_t const id) = 0;
    virtual void Unregister(uint32_t const id) = 0;

  protected:
    IMultiRODACLIClient(void) = default;
    IMultiRODACLIClient(IMultiRODACLIClient const &) = default;
    IMultiRODACLIClient(IMultiRODACLIClient &&) = default;
    virtual ~IMultiRODACLIClient(void) = default;

    IMultiRODACLIClient& operator=(IMultiRODACLIClient const &) = default;
    IMultiRODACLIClient& operator=(IMultiRODACLIClient &&) = default;
};

/**
 * \fn void IMultiRODACLIClient::Register(gpcc::cood::IRemoteObjectDictionaryAccess & rodaItf, uint32_t const id)
 * \brief Registers a RODA interface at the CLI client.
 *
 * \pre   The given ID is not yet used.
 *
 * \pre   There is no client registered at the given RODA interface yet.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rodaItf
 * Reference to the RODA interface that shall be registered.
 *
 * \param id
 * ID for referencing to `rodaItf`.\n
 * Multiple _different_ RODA interfaces can be registered at a @ref IMultiRODACLIClient interface using unique IDs. The
 * ID is used to distinguish them when CLI commands are entered.
 */

/**
 * \fn void IMultiRODACLIClient::Unregister(uint32_t const id)
 * \brief Unregisters a RODA interface that has previously been registered via
 *        [Register(...)](@ref gpcc::cood::IMultiRODACLIClient::Register).
 *
 * This has no effect, if there is no RODA interface with the given ID registered.
 *
 * \post  The RODA interface cannot be accessed via CLI any more.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param id
 * ID of the RODA interface that shall be unregistered.
 */

} // namespace cood
} // namespace gpcc

#endif // IMULTIRODACLICLIENT_HPP_202106121949
