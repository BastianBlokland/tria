#include "tria/log/logger.hpp"
#include "tria/log/metadata.hpp"
#include "tria/pal/utils.hpp"
#include <condition_variable>
#include <stdexcept>
#include <thread>
#include <vector>

namespace tria::log {

class Logger::Impl final {
public:
  explicit Impl(std::vector<SinkPtr> sinks) : m_sinks{std::move(sinks)}, m_threadShutdown{false} {
    // Validate input sinks.
    for (const auto& sinkPtr : sinks) {
      if (!sinkPtr) {
        throw std::invalid_argument{"Null sink pointer is not supported"};
      }
    }

    // Start the logging thread.
    m_thread = std::thread(&Impl::logLoop, this);
  }

  ~Impl() {
    if (!m_thread.joinable()) {
      return;
    }
    {
      std::lock_guard<std::mutex> lk(m_mutex);
      m_threadShutdown = true;
    }
    m_logCondVar.notify_one();
    m_thread.join();
  }

  auto publish(Message msg) noexcept {
    {
      std::lock_guard<std::mutex> lk(m_mutex);
      m_msgsInput.push_back(std::move(msg));
    }
    m_logCondVar.notify_one();
  }

private:
  std::vector<SinkPtr> m_sinks;
  std::thread m_thread;
  bool m_threadShutdown;

  std::vector<Message> m_msgsInput;
  std::vector<Message> m_msgsProcess;

  std::mutex m_mutex;
  std::condition_variable m_logCondVar;

  auto logLoop() noexcept -> void {
    pal::setThreadName("log_thread");

    auto running = true;
    while (running) {

      // Wait for a message to be logged.
      {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_logCondVar.wait(lk, [this]() { return !m_msgsInput.empty() || m_threadShutdown; });

        // Move the messages into our process buffer.
        m_msgsInput.swap(m_msgsProcess);

        if (m_threadShutdown) {
          running = false;
        }
      }

      // Process all messages.
      for (const auto& msg : m_msgsProcess) {
        processMsg(msg);
      }
      m_msgsProcess.clear();
    }
  }

  auto processMsg(const Message& msg) noexcept -> void {
    for (const auto& sinkPtr : m_sinks) {
      if (isInMask(sinkPtr->getMask(), msg.getMeta()->getLevel())) {
        sinkPtr->write(msg);
      }
    }
  }
};

Logger::Logger(std::vector<SinkPtr> sinks) : m_impl{std::make_unique<Impl>(std::move(sinks))} {}

Logger::~Logger() = default;

auto Logger::publish(Message msg) noexcept -> void { m_impl->publish(std::move(msg)); }

} // namespace tria::log
