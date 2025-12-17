/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR
#if defined(CONFIG_LOG)

#include <gpcc/log/integration/zephyr/ZephyrFrontEnd.hpp>
#include <gpcc/log/logfacilities/ILogFacility.hpp>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend_std.h>

#if !defined(CONFIG_LOG_MODE_IMMEDIATE)
  #warning "You should set CONFIG_LOG_MODE_IMMEDIATE=y in your project configuration (e.g. in your prj.conf file).\n" \
           "Otherwise Zephyr's log system will queue log messages, and log messages from Zephyr cannot be correlated\n" \
           "to log messages directly issued to GPCC's log system."
#endif

#if !defined(CONFIG_LOG_OUTPUT)
  #error "Using GPCC's ZephyrFrontEnd requires setting CONFIG_LOG_OUTPUT=y in your project configuration (e.g. in your prj.conf file)."
#endif


namespace gpcc {
namespace log  {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param logFacility
 * The new @ref ZephyrFrontEnd instance will register at the referenced log facility.
 *
 * \param logSourceName
 * Desired name for the log source registered in GPCC's log system, that will emit log messages received from Zephyr's
 * log system.
 */
ZephyrFrontEnd::ZephyrFrontEnd(ILogFacility & logFacility, std::string const & logSourceName)
: logFacility_(logFacility)
, logger_(logSourceName)
, accu_()
, errorDuringAccumulation_(false)
{
  logger_.SetLogLevel(gpcc::log::LogLevel::DebugOrAbove);
  logFacility_.Register(logger_);
}

/**
 * \brief Destructor.
 *
 * \pre   No Zephyr log backend is connect to this.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ZephyrFrontEnd::~ZephyrFrontEnd(void)
{
  logFacility_.Unregister(logger_);
}

/**
 * \brief This is invoked by the Zephyr backend to report if Zephyr has dropped one or more log messages.
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
 * \param n
 * Number of droppd log messages.
 */
void ZephyrFrontEnd::Dropped(size_t const n) noexcept
{
  if (n != 0U)
    logger_.LogV(gpcc::log::LogType::Warning, "Zephyr dropped %zu messages", n);
}

/**
 * \brief This is invoked by the Zephyr backend to append characters to the next emitted log message.
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
 * \param p
 * Pointer to a buffer containing @p n characters.
 *
 * \param n
 * Number of characters in the buffer referenced by @p p.
 */
void ZephyrFrontEnd::Append(char const * const p, size_t const n) noexcept
{
  if (errorDuringAccumulation_)
    return;

  try
  {
    accu_.append(p, n);
  }
  catch (std::bad_alloc const &)
  {
    errorDuringAccumulation_ = true;
  }
}

/**
 * \brief This is invoked by the Zephyr backend to finish a log message and emit it.
 *
 * This will emit a log message into GPCC's log system containing all characters previously added via @ref Append().
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
 * \param level
 * Log level. Any value from Zephyr's log level definitions (LOG_LEVEL_*).
 */
void ZephyrFrontEnd::MessageComplete(uint8_t const level) noexcept
{
  if (!errorDuringAccumulation_)
  {
    // remove any '\r'
    auto it = accu_.begin();
    while (it != accu_.end())
    {
      if (*it == '\r')
        it = accu_.erase(it);
      else
        ++it;
    }

    // remove any trailing '\n'
    while ((!accu_.empty()) && (accu_.back() == '\n'))
      (void)accu_.pop_back();

    if (!accu_.empty())
    {
      // translate Zephyr's log message type to GPCC's log message type.
      gpcc::log::LogType type = gpcc::log::LogType::Error;
      switch (level)
      {
        case LOG_LEVEL_DBG: type = gpcc::log::LogType::Debug;   break;
        case LOG_LEVEL_INF: type = gpcc::log::LogType::Info;    break;
        case LOG_LEVEL_WRN: type = gpcc::log::LogType::Warning; break;
        case LOG_LEVEL_ERR: type = gpcc::log::LogType::Error;   break;
      }

      // emit log message
      logger_.Log(type, accu_);
      accu_.clear();
    }
  }
  else
  {
    logger_.LogFailed();
    errorDuringAccumulation_ = false;
    accu_.clear();
  }
}

} // namespace log
} // namespace gpcc


int gpcc_log_ZephyrBackend_CharOut(uint8_t *data, size_t length, void *ctx)
{
  auto const pZB = static_cast<struct gpcc_log_stZephyrBackend*>(ctx);
  gpcc::log::ZephyrFrontEnd* const pZFE = pZB->pZFE;

  pZFE->Append(reinterpret_cast<char const*>(data), length);

  return length;
}

namespace
{
  extern "C"
  {
    void c_api_Process(const struct log_backend *const backend, union log_msg_generic *msg)
    {
      auto const pZB = static_cast<struct gpcc_log_stZephyrBackend*>(backend->cb->ctx);

      // ignore all log messages in panic mode
      if (!pZB->panicMode)
      {
        gpcc::log::ZephyrFrontEnd* const pZFE = pZB->pZFE;

        // ZephyrFrontEnd instance already connected?
        if (pZFE != nullptr)
        {
          uint32_t const flags = log_backend_std_get_flags();
          log_format_func_t const log_output_func = log_format_func_t_get(pZB->currentLogFormat);

          log_output_func(pZB->pOutput, &msg->log, flags);
          pZFE->MessageComplete(msg->log.hdr.desc.level);
        }
      }
    }

    void c_api_Dropped(const struct log_backend *const backend, uint32_t cnt)
    {
      auto const pZB = static_cast<struct gpcc_log_stZephyrBackend*>(backend->cb->ctx);

      // ignore potential calls in panic mode
      if (!pZB->panicMode)
      {
        gpcc::log::ZephyrFrontEnd* const pZFE = pZB->pZFE;

        // ZephyrFrontEnd instance already connected?
        if (pZFE != nullptr)
          pZFE->Dropped(cnt);
      }
    }

    void c_api_Panic(const struct log_backend *const backend)
    {
      auto const pZB = static_cast<struct gpcc_log_stZephyrBackend*>(backend->cb->ctx);

      // switch to panic mode
      pZB->panicMode = true;
      log_backend_std_panic(pZB->pOutput);
    }

    void c_api_Init(const struct log_backend *const backend)
    {
      auto const pZB = static_cast<struct gpcc_log_stZephyrBackend*>(backend->cb->ctx);

      log_output_ctx_set(pZB->pOutput, static_cast<void*>(pZB));
    }

    int c_api_FormatSet(const struct log_backend *const backend, uint32_t log_type)
    {
      auto const pZB = static_cast<struct gpcc_log_stZephyrBackend*>(backend->cb->ctx);

      pZB->currentLogFormat = log_type;
      return 0;
    }
  }
}

struct log_backend_api const gpcc_log_ZephyrBackend_api =
{
  .process    = c_api_Process,
  .dropped    = c_api_Dropped,
  .panic      = c_api_Panic,
  .init       = c_api_Init,
  .is_ready   = nullptr,
  .format_set = c_api_FormatSet,
  .notify     = nullptr
};

#endif // CONFIG_LOG
#endif // #ifdef OS_ZEPHYR
