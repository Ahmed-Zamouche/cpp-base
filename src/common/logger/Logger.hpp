#ifndef _GUID_a27d802f_8f62_44d9_9f17_e7957284e22d_ILogger_HPP_
#define _GUID_a27d802f_8f62_44d9_9f17_e7957284e22d_ILogger_HPP_

#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>

namespace common {
namespace logger {
enum class Level {
  TRACE = 0,
  DEBUG,
  INFO,
  NOTICE,
  WARN,
  ERROR,
  FATAL,
};

class Endl {};

class Logger;

class LoggerBase {

public:
  static LoggerBase *getInstance();

private:
  LoggerBase();
  virtual ~LoggerBase();

  struct Impl;
  std::unique_ptr<Impl> pImpl;

  friend class Logger;
};

class Logger {

public:
  Logger operator()(Level level = Level::INFO);

  Logger(const Logger &o);

  Logger &operator=(const Logger &o);

  Logger(Logger &&o);

  Logger &operator=(Logger &&o);

  virtual ~Logger();

  template <typename T> Logger &operator<<(const T &t) {
    mOss << t;
    return *this;
  }

  Logger &operator<<(std::ios_base &f(std::ios_base &)) {
    mOss << f;
    return *this;
  }

  Logger &operator<<(std::ostream &f(std::ostream &)) {
    mOss << f;
    return *this;
  }

  Logger &operator<<(const Endl &);

  bool operator==(const Logger &o) const;

  std::size_t hash() const;

  static Logger &get(const std::string &name);

protected:
private:
  explicit Logger(const std::string &name, Level level = Level::INFO);

  struct Impl;
  std::unique_ptr<Impl> pImpl;
  std::ostringstream mOss;
};

} // namespace logger
} // namespace common

#endif /* _GUID_a27d802f_8f62_44d9_9f17_e7957284e22d_ILogger_HPP_ */
