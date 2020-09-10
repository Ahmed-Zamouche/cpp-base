#include "Logger.hpp"

#include "common/Assert.h"
#include "common/concurrent/Semaphore.hpp"
#include "common/concurrent/SpinLock.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <queue>
#include <thread>
#include <unordered_map>

#if __has_include(<format>)
#include <format>
using std::format;
#else
#include <fmt/format.h>
using fmt::format;
#endif

#ifndef _WIN32
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace common {
namespace logger {

static inline std::string toString(const Level &value) {
  switch (value) {
  case Level::TRACE:
    return "TRACE";
  case Level::DEBUG:
    return "DEBUG";
  case Level::INFO:
    return "INFO";
  case Level::NOTICE:
    return "NOTICE";
  case Level::WARN:
    return "WARN";
  case Level::ERROR:
    return "ERROR";
  case Level::FATAL:
    return "FATAL";
  default:
    ASSERT(false, "enum value out of range");
    break;
  }
  return "unknown enum value";
}

static inline std::chrono::milliseconds getTimestamp() {
  using namespace std::chrono;
  milliseconds ms =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  return ms;
}

static inline auto getThreadId() {
#ifndef _WIN32
  return static_cast<long long int>(syscall(__NR_gettid));
#else
  return std::this_thread::get_id();
#endif
}

using namespace concurrent;

/* LoggerBase:Impl */
struct LoggerBase::Impl {

  Impl() { std::thread(&LoggerBase::Impl::run, this).detach(); }
  ~Impl() {}

  void pushLogEntry(std::string &&logEntry) {
    {
      const LockGuard<SpinLock> lockGuard{mLock};
      mLogEntries.push(std::move(logEntry));
    }
    mSem.notify();
  }

  SpinLock mLock;
  Semaphore mSem;
  const Level mLevel{Level::INFO};
  std::ostream &mOstream{std::cout};
  std::queue<std::string> mLogEntries;

  void run() {
    while (true) {

      mSem.wait();

      std::string mLogEntry;
      {
        const LockGuard<SpinLock> lockGuard{mLock};
        if (mLogEntries.empty()) {
          continue;
        }
        mLogEntry = std::move(mLogEntries.front());
        mLogEntries.pop();
      }

      mOstream << mLogEntry;
    }
  }

  static LoggerBase *mInstance;
};

LoggerBase *LoggerBase::Impl::mInstance{nullptr};

/* LoggerBase */

LoggerBase::LoggerBase() : pImpl(std::make_unique<Impl>()) {}

LoggerBase::~LoggerBase() { pImpl->mOstream.flush(); }

LoggerBase *LoggerBase::getInstance() {

  static std::once_flag onceFlag;
  std::call_once(onceFlag,
                 [&]() { LoggerBase::Impl::mInstance = new LoggerBase(); });
  return LoggerBase::Impl::mInstance;
}

/* Logger::Impl */
struct LoggerHash {
public:
  std::size_t operator()(const Logger &logger) const { return logger.hash(); }
};

struct LoggerEqualTo {
public:
  bool operator()(const Logger &lhs, const Logger &rhs) const {
    return lhs == rhs;
  }
};

struct Logger::Impl {
  Impl(const std::string &name, Level level)
      : mName(name), mLevel(level), mBase(LoggerBase::getInstance()) {}

  std::string mName;
  Level mLevel;
  LoggerBase *mBase;
  static std::unordered_map<std::string, Logger> mLoggers;
};

std::unordered_map<std::string, Logger> Logger::Impl::mLoggers{
    {"Main", Logger("Main")}};

/* Logger */
Logger::Logger(const std::string &name, Level level)
    : pImpl(std::make_unique<Impl>(name, level)) {}

Logger::~Logger() {}

Logger::Logger(const Logger &o)
    : pImpl(std::make_unique<Impl>(o.pImpl->mName, o.pImpl->mLevel)) {}

Logger &Logger::operator=(const Logger &o) {
  if (this != &o) {
    pImpl->mName = o.pImpl->mName;
    pImpl->mLevel = o.pImpl->mLevel;
  }
  return *this;
}

Logger::Logger(Logger &&o)
    : pImpl(std::move(o.pImpl)), mOss(std::move(o.mOss)) {}

Logger &Logger::operator=(Logger &&o) {
  if (this != &o) {
    pImpl = std::move(o.pImpl);
    mOss = std::move(o.mOss);
  }
  return *this;
}

Logger Logger::operator()(Level level) { return Logger(pImpl->mName, level); }

Logger &Logger::operator<<(const Endl &) {

  if (pImpl->mLevel >= pImpl->mBase->pImpl->mLevel) {
    auto timestamp = getTimestamp();
    auto threadId = getThreadId();

    auto logEntry =
        format("[{:010d}.{:04d}][{: <6}] {: <}: {: <}: {}\n",
               timestamp.count() / 1000, timestamp.count() % 1000,
               toString(pImpl->mLevel), threadId, pImpl->mName, mOss.str());

    pImpl->mBase->pImpl->pushLogEntry(std::move(logEntry));
  }

  mOss.str("");
  mOss.clear();

  return *this;
}

bool Logger::operator==(const Logger &o) const {
  return (pImpl->mName == o.pImpl->mName);
}

std::size_t Logger::hash() const {
  return std::hash<std::string>{}(pImpl->mName);
}

Logger &Logger::get(const std::string &name) {

  auto it = Logger::Impl::mLoggers.find(name);

  if (it == Logger::Impl::mLoggers.end()) {
    Logger::Impl::mLoggers.insert({name, Logger(name)});
    it = Logger::Impl::mLoggers.find(name);
  }
  return it->second;
}

} // namespace logger
} // namespace common
