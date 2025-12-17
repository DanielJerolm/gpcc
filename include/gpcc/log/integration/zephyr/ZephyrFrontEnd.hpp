/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR

#ifndef ZEPHYRFRONTEND_202512051239
#define ZEPHYRFRONTEND_202512051239

#if !defined(CONFIG_LOG)
   #error "Using GPCC's ZephyrFrontEnd requires setting CONFIG_LOG=y in your project configuration (e.g. in your prj.conf file)."
#endif

#include <gpcc/log/Logger.hpp>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log_output.h>
#include <atomic>
#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace log  {

class ILogFacility;

/**
 * \ingroup GPCC_LOG_INTEGRATION_ZEPHYR
 * \brief Front-end for GPCC's log system that receives log messages from a back-end registered in Zephyr's log system.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ZephyrFrontEnd final
{
  public:
    ZephyrFrontEnd(void) = delete;
    ZephyrFrontEnd(ILogFacility & logFacility, std::string const & logSourceName);
    ZephyrFrontEnd(ZephyrFrontEnd const &) = delete;
    ZephyrFrontEnd(ZephyrFrontEnd &&) = delete;
    ~ZephyrFrontEnd(void);

    ZephyrFrontEnd& operator=(ZephyrFrontEnd const &) = delete;
    ZephyrFrontEnd& operator=(ZephyrFrontEnd &&) = delete;

    void Dropped(size_t const n) noexcept;
    void Append(char const * const p, size_t const n) noexcept;
    void MessageComplete(uint8_t const level) noexcept;

  private:
    /// Log facility where this front-end is registered at.
    ILogFacility & logFacility_;

    /// Logger used to emit log messages from Zephyr into @ref logFacility_.
    Logger logger_;

    /// Characters dropping in from Zephyr's log system are accumulated here.
    std::string accu_;

    /// Indicates if an error ocurred during accumulaton of characters in @ref accu_.
    bool errorDuringAccumulation_;
};

} // namespace log
} // namespace gpcc


/**
 * \ingroup GPCC_LOG_INTEGRATION_ZEPHYR
 * \brief Structure of an static backend for Zephyr's log system.
 */
struct gpcc_log_stZephyrBackend
{
  /// Pointer to static log output object.
  struct log_output const * pOutput;

  /// Currently configured log message format.
  uint32_t currentLogFormat;

  /// Indicates if panic mode is enabled.
  std::atomic<bool> panicMode;

  /// Log messages from Zephyr's log system will be passed to the referenced @ref gpcc::log::ZephyrFrontEnd instance.
  /** Log messages will be ignored until this is set to a valid object. */
  std::atomic<gpcc::log::ZephyrFrontEnd*> pZFE;
};

extern struct log_backend_api const gpcc_log_ZephyrBackend_api;

extern "C" int gpcc_log_ZephyrBackend_CharOut(uint8_t *data, size_t length, void *ctx);


/**
 * \ingroup GPCC_LOG_INTEGRATION_ZEPHYR
 * \brief Creates a static backend for Zephyr's log system plus an output object for Zephyr's log system.
 *
 * The way the backend and the output object are defined is cruical. There are two alternative options.\n
 * If a different approch is used, then the backend might not be picked up by Zephyr's build process and no log
 * messages will be received.\n
 * The approaches are:
 * - Instantiate this macro in a C++ file that is build into an __object library__.\n
 *   The "app" target defined by Zephyr must link against that object library.
 * - Alternatively, you can instantiate this macro in a C++ file that is build directly into the "app" target defined
 *   by Zephyr.
 *
 * - - -
 *
 * \param ID
 * ID for the backend instance. Typically "1".
 */
#define DEFINE_GPCC_LOG_ZEPHYR_LOG_BACKEND(ID)                                                                      \
        static uint8_t gpcc_log_ZephyrBackend_buffer_##ID[128];                                                     \
        LOG_OUTPUT_DEFINE(gpcc_log_ZephyrBackend_Output_##ID,                                                       \
                          gpcc_log_ZephyrBackend_CharOut,                                                           \
                          gpcc_log_ZephyrBackend_buffer_##ID, sizeof(gpcc_log_ZephyrBackend_buffer_##ID));          \
                                                                                                                    \
        struct gpcc_log_stZephyrBackend gpcc_log_ZephyrBackend_##ID =                                               \
        {                                                                                                           \
          .pOutput          = &gpcc_log_ZephyrBackend_Output_##ID  ,                                                \
          .currentLogFormat = LOG_OUTPUT_TEXT,                                                                      \
          .panicMode        = false,                                                                                \
          .pZFE             = nullptr                                                                               \
        };                                                                                                          \
                                                                                                                    \
        LOG_BACKEND_DEFINE(gpcc_log_ZephyrBackendInst_##ID,                                                         \
                           gpcc_log_ZephyrBackend_api,                                                              \
                           true,                                                                                    \
                           (void*)&gpcc_log_ZephyrBackend_##ID);

/**
 * \ingroup GPCC_LOG_INTEGRATION_ZEPHYR
 * \brief Macro for accessing a static log backend instance.
 *
 * Use this in the C++-file only that contains the instantiation of the log backend
 * (@ref DEFINE_GPCC_LOG_ZEPHYR_LOG_BACKEND()).
 *
 * - - -
 *
 * \param ID
 * ID of the instance that shall be accessed.
 */
#define GPCC_LOG_ZEPHYR_LOG_BACKEND(ID) (gpcc_log_ZephyrBackend_##ID)

/**
 * \ingroup GPCC_LOG_INTEGRATION_ZEPHYR
 * \brief Connects an @ref gpcc::log::ZephyrFrontEnd instance to an static Zephyr log backend.
 *
 * Use this in the same C++-file that contains the instantiation of the Zephyr log backend
 * (@ref DEFINE_GPCC_LOG_ZEPHYR_LOG_BACKEND()).
 *
 * The best practice is to define a function that uses this macro to setup the connection.\n
 * Example:
 * ~~~{.cpp}
 * void ConnectZephyrLogBackend(gpcc::log::ZephyrFrontEnd& zfe) noexcept
 * {
 *   GPCC_LOG_ZEPHYR_LOG_BACKEND_CONNECT(1, zfe);
 * }
 * ~~~
 *
 * - - -
 *
 * \param ID
 * ID of the static Zephyr log backend.
 *
 * \param zfe
 * Reference to the @ref gpcc::log::ZephyrFrontEnd instance, that shall be linked to the backend.
 */
#define GPCC_LOG_ZEPHYR_LOG_BACKEND_CONNECT(ID, zfe) (GPCC_LOG_ZEPHYR_LOG_BACKEND(ID).pZFE = &zfe)

/**
 * \ingroup GPCC_LOG_INTEGRATION_ZEPHYR
 * \brief Disconnects an @ref gpcc::log::ZephyrFrontEnd instance from an static Zephyr log backend.
 *
 * Use this in the same C++-file that contains the instantiation of the Zephyr log backend
 * (@ref DEFINE_GPCC_LOG_ZEPHYR_LOG_BACKEND()).
 *
 * The best practice is to define a function that uses this macro to remove the connection.\n
 * Example:
 * ~~~{.cpp}
 * void DisconnectZephyrLogBackend(void) noexcept
 * {
 *   GPCC_LOG_ZEPHYR_LOG_BACKEND_DISCONNECT(1);
 * }
 * ~~~
 *
 * This has no effect, if the static log backend currently has no connection to any @ref gpcc::log::ZephyrFrontEnd
 * instance.
 *
 * This macro perfoms a flush of the Zephyr log system.
 *
 * - - -
 *
 * \param ID
 * ID of the static Zephyr log backend, whose connection to a @ref gpcc::log::ZephyrFrontEnd instance shall be removed.
 */
#define GPCC_LOG_ZEPHYR_LOG_BACKEND_DISCONNECT(ID)      \
        GPCC_LOG_ZEPHYR_LOG_BACKEND(ID).pZFE = nullptr; \
        log_flush();

#endif // #ifndef ZEPHYRFRONTEND_202512051239
#endif // #ifdef OS_ZEPHYR
