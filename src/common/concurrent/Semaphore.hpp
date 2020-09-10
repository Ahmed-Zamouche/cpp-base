#ifndef _GUID_1e1f57e4_b928_4386_ad22_2f90aa23e31f_Semaphore_HPP_
#define _GUID_1e1f57e4_b928_4386_ad22_2f90aa23e31f_Semaphore_HPP_

#include "SpinLock.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace common {
namespace concurrent {
class Semaphore {
public:
  Semaphore(int count = 0) : m_count(count) {}
  virtual ~Semaphore() {}

  inline void notify() {

    {
      // const std::lock_guard<std::mutex> lock(m_mutex);
      const LockGuard<SpinLock> lock(m_lock);
      m_count++;
    }

    m_cv.notify_one();
  }
  inline void wait() {

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [&]() -> bool { return m_count; });
    }

    {
      const LockGuard<SpinLock> lock(m_lock);
      m_count--;
    }
  }
  inline bool
  wait_until(const std::chrono::system_clock::time_point &time_point) {

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait_until(lock, time_point, [&]() -> bool { return m_count; });
      if (m_cv.wait_until(lock, time_point,
                          [&]() -> bool { return m_count; })) {
        const LockGuard<SpinLock> lock(m_lock);
        m_count--;
        return true;
      }
    }

    return true;
  }

  inline bool try_wait() {
    const LockGuard<SpinLock> lock(m_lock);
    if (m_count) {
      --m_count;
      return true;
    }
    return false;
  }

private:
  std::mutex m_mutex;
  SpinLock m_lock;
  std::condition_variable m_cv;
  int m_count{0};
};
} // namespace concurrent
} // namespace common

#endif /* _GUID_1e1f57e4_b928_4386_ad22_2f90aa23e31f_Semaphore_HPP_ */
