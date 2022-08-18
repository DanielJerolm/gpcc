/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#ifndef CLIADAPTERBASE_HPP_201905062115
#define CLIADAPTERBASE_HPP_201905062115

#include "gpcc/src/cood/Object.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace gpcc {

namespace cli  {
  class CLI;
}

namespace cood {

class IObjectAccess;

/**
 * \ingroup GPCC_COOD_CLI
 * \brief This class offers a [CLI](@ref gpcc::cli::CLI) command that allows to access an
 *        [ObjectDictionary](@ref gpcc::cood::ObjectDictionary).\n
 *        This is an abstract base class with a small set of hooks that need to be provided by a user-defined
 *        sub-class.
 *
 * # CLI command
 * This class provides one CLI command. The command's name is configurable when the class is instantiated. The command
 * allows to perform the following operations on the object dictionary:
 * - enumerate objects
 * - query object's meta data
 * - read subindices
 * - write subindices
 * - read objects (complete access)
 * - write objects (complete access)
 *
 * # Intended way of subclassing
 * ## CLI command
 * Sub-classes have to invoke @ref RegisterCLICommand() and @ref UnregisterCLICommand(). In most cases the end of the
 * subclass' constructor and the begin of the subclass' destructor are suitable places to make the calls.
 *
 * ## Additional information, functionality and customization
 * To provide access to an object dictionary, this class requires up to three pieces of information:
 * - what access rights shall be used for read- and write-access
 * - how to convert an object's attributes to a human readable string
 * - how to display application-specific meta data (optional)
 *
 * The sub-class has to provide an implementation for up to four virtual hook-methods, which will provide the three
 * pieces of information mentioned before:
 * - @ref BeginAccessHook()
 * - @ref EndAccessHook()
 * - @ref AttributesToStringHook()
 * - @ref AppSpecificMetaDataToStringHook() (only required if the default implementation is not suitable)
 *
 * ## Locking the system state and the access rights
 * In some applications, the access rights provided by @ref BeginAccessHook() depend on some state variable that
 * could change during the access to the object dictionary. Such a state variable is e.g. the state of the
 * EtherCAT Slave State Machine in an application using an @ref gpcc::cood::ObjectDictionary in conjunction with
 * an EtherCAT Slave Stack's SDO server.
 *
 * @ref BeginAccessHook() and @ref EndAccessHook() will always be called in sequence by the same thread (CLI).
 * They can be used to acquire a lock for the system state (e.g. a [Mutex](@ref gpcc::osal::Mutex)) until the object
 * dictionary access has finished. Subclasses can ensure this way, that the access rights used to access the object
 * dictionary do not change during the access. The other way round by blocking @ref BeginAccessHook(), an access to the
 * object dictionary can be delayed until the system state and the access rights have been updated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class CLIAdapterBase
{
  public:
    CLIAdapterBase(void) = delete;
    CLIAdapterBase(CLIAdapterBase const &) = delete;
    CLIAdapterBase(CLIAdapterBase &&) = delete;
    virtual ~CLIAdapterBase(void) = default;

    CLIAdapterBase& operator=(CLIAdapterBase const &) = delete;
    CLIAdapterBase& operator=(CLIAdapterBase &&) = delete;

  protected:
    CLIAdapterBase(IObjectAccess & _od,
                   gpcc::cli::CLI & _cli,
                   std::string const & _cmdName,
                   uint8_t const _attributeStringMaxLength);

    void RegisterCLICommand(void);
    void UnregisterCLICommand(void) noexcept;

    virtual Object::attr_t BeginAccessHook(void) = 0;
    virtual void EndAccessHook(void) noexcept = 0;
    virtual std::string AttributesToStringHook(Object::attr_t const attributes) = 0;
    virtual std::string AppSpecificMetaDataToStringHook(std::vector<uint8_t> const & data);

  private:
    /// Interface used to access the object dictionary.
    IObjectAccess & od;

    /// CLI component where the CLI command is registered.
    gpcc::cli::CLI & cli;

    /// Name of the published CLI command.
    std::string const cmdName;

    /// Maximum length of any string that could be returned by @ref AttributesToStringHook().
    uint8_t const attributeStringMaxLength;


    void CLI_CommandHandler(std::string const & restOfLine, gpcc::cli::CLI & cli);

    void CLI_Enumerate(std::string const & restOfLine);
    void CLI_Info(std::string const & restOfLine);
    void CLI_Read(std::string const & restOfLine);
    void CLI_Write(std::string const & restOfLine);
    void CLI_CARead(std::string const & restOfLine);
    void CLI_CAWrite(std::string const & restOfLine);

    static uint_fast8_t DigitsInSubindex(uint8_t const si) noexcept;
};

/**
 * \fn gpcc::cood::Object::attr_t CLIAdapterBase::BeginAccessHook
 * \brief This is invoked before a read/write access to the object dictionary takes place.
 *
 * Subclasses implementing this method shall:
 * 1. If the access rights may change, then there should be a lock dedicated to the access rights.\n
 *    This method shall acquire that lock. This ensures, that the access rights are valid and constant during the
 *    access.
 * 2. Determine the access rights that shall be used for the read/write access.
 *
 * [EndAccessHook()](@ref gpcc::cood::CLIAdapterBase::EndAccessHook) is the counterpart to this. It will be invoked
 * after the read/write access has finished, either successful or not.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This will be invoked by the CLI thread only.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \return
 * Access rights that shall be used for the read/write access.
 */

/**
 * \fn void CLIAdapterBase::EndAccessHook
 * \brief This is invoked after a read/write access to the object dictionary has taken place.
 *
 * This is the counterpart to [BeginAccessHook()](@ref gpcc::cood::CLIAdapterBase::BeginAccessHook).
 *
 * Subclasses implementing this method shall unlock any locks acquired in the prior call to
 * [BeginAccessHook()](@ref gpcc::cood::CLIAdapterBase::BeginAccessHook).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This will be invoked by the CLI thread only.\n
 * [BeginAccessHook()](@ref gpcc::cood::CLIAdapterBase::BeginAccessHook) and this are invoked by _the same_ thread.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred thread cancellation disabled.
 */

/**
 * \fn std::string CLIAdapterBase::AttributesToStringHook
 * \brief Converts object attributes into a human-readable string.
 *
 * Depending on the application of the object dictionary (e.g. EtherCAT or CANopen), the
 * [object attributes](@ref gpcc::cood::Object::attr_ACCESS_RD) defined by GPCC have a different meaning or are not
 * applicable in EtherCAT or CANopen. There are also bits in [Object::attr_t](@ref gpcc::cood::Object::attr_t) that are
 * not defined by GPCC. Users can assign any custom meaning to them.
 *
 * This method shall convert the attributes into a human-readable string according to the conventions of the
 * application. For standard applications (e.g. EtherCAT or CANopen), subclasses may delegate the call to
 * [Object::AttributeToString()](@ref gpcc::cood::Object::AttributeToString) if they like. If custom attribute bits are
 * defined, then subclasses should implement this method on their own.
 *
 * The output of this method will be used to compose table-structured CLI output. The length of the returned string must
 * be constant and match the length passed to
 * [CLIAdapterBase::CLIAdapterBase()](@ref gpcc::cood::CLIAdapterBase::CLIAdapterBase), parameter
 * '_attributeStringMaxLength'. If necessary, short output shall be extended with space characters. This will ensure
 * that the rows of any table-structured CLI output are properly aligned.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This will be invoked by the CLI thread only.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param attributes
 * Attribute value that shall be converted into a string.
 *
 * \return
 * String object containing a human-readable textual representation of parameter 'attributes'.
 */

/**
 * \fn std::string CLIAdapterBase::AppSpecificMetaDataToStringHook
 * \brief Converts application-specific meta data into a human-readable string.
 *
 * Depending on the application of the object dictionary, application-specific meta data may be attached to some or
 * all subindices.
 *
 * Derived classes may implement this method to convert the application-specific meta data into a human-readable string
 * according to the custom structure, type, and format of the application-specific meta data. The default implementation
 * provided by base class [CLIAdapterBase](@ref gpcc::cood::CLIAdapterBase) will convert the application-specific
 * meta data into hexadecimal byte values.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This will be invoked by the CLI thread only.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param data
 * Unmodifiable reference to an std::vector<uint8_t> containing the application-specific meta data.
 *
 * \return
 * String object containing a human-readable string representation of parameter 'data'.
 */

} // namespace cood
} // namespace gpcc

#endif // CLIADAPTERBASE_HPP_201905062115
