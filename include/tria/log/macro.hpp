#pragma once
#include "tria/log/logger.hpp"
#include "tria/log/metadata.hpp"

// '__PRETTY_FUNCTION__' does not exist on msvc, use '__FUNCSIG__' instead.
#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define LOG(logger, lvl, txt, ...)                                                                 \
  do {                                                                                             \
    constexpr static auto meta = log::MetaData{lvl,                                                \
                                               std::string_view{txt},                              \
                                               std::string_view{__FILE__},                         \
                                               std::string_view{__PRETTY_FUNCTION__},              \
                                               __LINE__};                                          \
    auto* loggerPtr            = (logger);                                                         \
    if (loggerPtr) {                                                                               \
      loggerPtr->publish(log::Message{&meta, {__VA_ARGS__}});                                      \
    }                                                                                              \
  } while (false)

#if defined(NDEBUG)
/* Log a debug message.
 */
#define LOG_D(logger, txt, ...) (void)logger
#else
/* Log a debug message.
 */
#define LOG_D(logger, txt, ...) LOG(logger, log::Level::Debug, txt, __VA_ARGS__)
#endif

/* Log a info message.
 */
#define LOG_I(logger, txt, ...) LOG(logger, log::Level::Info, txt, __VA_ARGS__)

/* Log a warning message.
 */
#define LOG_W(logger, txt, ...) LOG(logger, log::Level::Warn, txt, __VA_ARGS__)

/* Log a error message.
 */
#define LOG_E(logger, txt, ...) LOG(logger, log::Level::Error, txt, __VA_ARGS__)
