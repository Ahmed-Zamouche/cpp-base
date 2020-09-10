#ifndef _GUID_700c48b4_d620_43ac_8b7f_7cd9d7168b5d_Actor_HPP_
#define _GUID_700c48b4_d620_43ac_8b7f_7cd9d7168b5d_Actor_HPP_

#include "Semaphore.hpp"
#include "SpinLock.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <thread>
#include <vector>

namespace common {
namespace concurrent {
namespace actor {

class Timer;
class AbstractActor;
using TimerSharedPtr = std::shared_ptr<Timer>;

template <typename Type> class MailBox {
public:
  MailBox() {}
  virtual ~MailBox(){};
  bool empty() const { return m_container.empty(); }
  Type &back() { return m_container.back(); }
  const Type &back() const { return m_container.back(); }
  Type &front() { return m_container.front(); }
  const Type &front() const { return m_container.front(); }
  void pop() { m_container.pop_back(); }
  void push(const Type &val) { m_container.push_back(val); }
  void push(Type &&val) { m_container.push_back(std::forward<Type>(val)); }
  std::size_t size() const { return m_container.size(); }

  template <typename Container> void move(Container &container) {
    std::move(m_container.begin(), m_container.end(),
              std::back_inserter(container));
  }

  template <typename Container> void swap(Container &container) {
    std::swap(m_container, container);
  }

private:
  std::vector<Type> m_container;
};

class IMessage {
public:
  IMessage() {}
  virtual ~IMessage() {}
  virtual void dispatch(AbstractActor *) = 0;
};

template <typename Runnable, typename T> class Message : public IMessage {
private:
  const T value;

public:
  T &getValue() { return value; }
  T const &getValue() const { return value; }
  Message(const T &val) : value(val) {}
  virtual ~Message() {}
  void dispatch(AbstractActor *actor) override {
    // static_cast<Runnable *>(actor)->onMessage(*this);
    static_cast<Runnable *>(actor)->onMessage(value);
  }
};

class AbstractActor {
protected:
  SpinLock m_lock;
  Semaphore m_sem;
  bool m_stop{false};
  MailBox<std::unique_ptr<IMessage>> m_mbox;

public:
  AbstractActor() {}
  virtual ~AbstractActor(){};

  virtual void onStart() = 0;
  virtual void onStop() = 0;

  void stop() { m_stop = true; }

  virtual void eventLoop() {

    m_sem.wait();

    std::vector<std::unique_ptr<IMessage>> messages;
    {
      const LockGuard<SpinLock> lockGuard{m_lock};
      m_mbox.swap(messages);
    }

    for (const auto &message : messages) {
      message->dispatch(this);
    }
  };

  int dispatcher() {
    onStart();
    while (!m_stop) {
      eventLoop();
    }
    onStop();
    return 0;
  }

  void post(std::unique_ptr<IMessage> msg) {
    {
      const LockGuard<SpinLock> lockGuard{m_lock};
      m_mbox.push(std::move(msg));
    }
    m_sem.notify();
  }
  virtual void onMessage(const IMessage &){};
  virtual void onMessage(const std::shared_ptr<Timer> &){};
};

template <typename Runnable> class Actor : public AbstractActor {
public:
  Actor() {}
  virtual ~Actor() { m_stop = true; };

  template <typename... Args>
  static std::shared_ptr<Runnable> create(Args &&... args) {
    auto runnable = std::make_shared<Runnable>(std::forward<Args>(args)...);
    std::thread(&Actor::dispatcher, runnable.get()).detach();
    return runnable;
  }

  template <typename... Args> static int run(Args &&... args) {
    auto runnable = std::make_shared<Runnable>(std::forward<Args>(args)...);
    return runnable->dispatcher();
  }

  template <typename T> void post(const T &msg) {
    std::unique_ptr<IMessage> msgPtr =
        std::make_unique<Message<Runnable, T>>(msg);
    AbstractActor::post(std::move(msgPtr));
  }
};

class Timer {
  using system_clock = std::chrono::system_clock;
  using milliseconds = std::chrono::milliseconds;

private:
  AbstractActor *m_actor;
  system_clock::time_point m_timeout;
  std::uint32_t m_interval;

  static std::list<std::shared_ptr<Timer>> m_timers;
  static bool m_stop;
  static SpinLock m_lock;
  static Semaphore m_sem;

public:
  Timer(AbstractActor *actor, std::uint32_t timeout, std::uint32_t interval)
      : m_actor(actor), m_timeout(system_clock::now() + milliseconds(timeout)),
        m_interval(interval) {}
  virtual ~Timer(){};

  static std::shared_ptr<Timer>
  create(AbstractActor *actor, std::uint32_t timeout, std::uint32_t interval) {

    static std::once_flag onceFlag;

    std::call_once(onceFlag, [&]() { std::thread(&Timer::run).detach(); });

    auto timer = std::make_shared<Timer>(actor, timeout, interval);

    // std::cerr << measureRunTime(insert, timer) << std::endl;
    insert(timer);

    m_sem.notify();

    return timer;
  }

private:
  static void insert(std::shared_ptr<Timer> &timer) {

    const LockGuard<SpinLock> lockGuard{m_lock};
    auto it = std::upper_bound(m_timers.begin(), m_timers.end(), timer,
                               [](const std::shared_ptr<Timer> &lhs,
                                  const std::shared_ptr<Timer> &rhs) {
                                 return lhs->m_timeout < rhs->m_timeout;
                               });
    m_timers.insert(it, timer);
  }

  static void run() {

    m_sem.wait();

    while (!m_stop) {

      std::shared_ptr<Timer> timer;
      {
        const LockGuard<SpinLock> lockGuard{m_lock};
        timer = m_timers.front();
      }
      if (system_clock::now() < timer->m_timeout) {
        if (m_sem.wait_until(timer->m_timeout)) {
          continue;
        }
      }

      {
        const LockGuard<SpinLock> lockGuard{m_lock};
        m_timers.pop_front();
      }

      {
        std::unique_ptr<IMessage> msgPtr =
            std::make_unique<Message<AbstractActor, std::shared_ptr<Timer>>>(
                timer);
        timer->m_actor->post(std::move(msgPtr));
      }
      if (timer->m_interval) {
        timer->m_timeout =
            system_clock::now() + milliseconds(timer->m_interval);
        // std::cerr << measureRunTime(insert, timer) << std::endl;
        insert(timer);
      }
    }
  }
};

} // namespace actor
} // namespace concurrent
} // namespace common

#endif /* _GUID_700c48b4_d620_43ac_8b7f_7cd9d7168b5d_Actor_HPP_ */
