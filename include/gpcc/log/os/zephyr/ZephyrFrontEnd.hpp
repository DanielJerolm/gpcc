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

#include <gpcc/log/Logger.hpp>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_output.h>
#include <atomic>
#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace log  {

class ILogFacility;

/**
 * \ingroup GPCC_LOG_ZEPHYR
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
 * \ingroup GPCC_LOG_ZEPHYR
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
 * \ingroup GPCC_LOG_ZEPHYR
 * \brief Creates a static backend plus output object for Zephyr's log system.
 *
 * Instantiate this macro in a C++ file build into an __object library_.\n
 * The "app" target defined by Zephyr must link against the object library.\n
 * If this is implemented different, then the backend may not be picked up by Zephyrs build process and you fill not
 * receive any log messages.
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
 * \ingroup GPCC_LOG_ZEPHYR
 * \brief Macro for accesing a static log backend instance with given @p ID.
 *
 * Use this in the C++-file only that contains the instantiation of the log backend
 * (@ref DEFINE_GPCC_LOG_ZEPHYR_LOG_BACKEND()).
 */
#define GPCC_LOG_ZEPHYR_LOG_BACKEND(ID) (gpcc_log_ZephyrBackend_##ID)

/**
 * \ingroup GPCC_LOG_ZEPHYR
 * \brief Connects a [ZephyrFrontEnd](@ref gpcc::log::ZephyrFrontEnd) instance to the static log backend with
 *        given @p ID.
 *
 * Use this in the C++-file only that contains the instantiation of the log backend
 * (@ref DEFINE_GPCC_LOG_ZEPHYR_LOG_BACKEND()).
 *
 * The best practice is to define a function that uses this macro to make the connection. Example:
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
 * Reference to the [ZephyrFrontEnd](@ref gpcc::log::ZephyrFrontEnd) instance.
 */
#define GPCC_LOG_ZEPHYR_LOG_BACKEND_CONNECT(ID, zfe) (GPCC_LOG_ZEPHYR_LOG_BACKEND(ID).pZFE = &zfe)

#endif // #ifndef ZEPHYRFRONTEND_202512051239
#endif // #ifdef OS_ZEPHYR
