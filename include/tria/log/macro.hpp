#pragma once
#include "tria/log/logger.hpp"
#include "tria/log/metadata.hpp"

// If 'SRC_PATH_LENGTH' is defined we strip that part of the path of.
#if defined(SRC_PATH_LENGTH)
#define __FILENAME__ (static_cast<const char*>(__FILE__) + SRC_PATH_LENGTH)
#else
#define __FILENAME__ __FILE__
#endif

// '__PRETTY_FUNCTION__' does not exist on msvc, use '__FUNCSIG__' instead.
#if defined(_MSC_VER)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define LOG(logger, lvl, txt, ...)                                                                 \
  do {                                                                                             \
    constexpr static auto meta = log::MetaData{lvl,                                                \
                                               std::string_view{txt},                              \
                                               std::string_view{__FILENAME__},                     \
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
