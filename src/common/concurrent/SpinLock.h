#ifndef _GUID_61490d1a_68e7_414a_938f_482060433885_SpinLock_H_
#define _GUID_61490d1a_68e7_414a_938f_482060433885_SpinLock_H_

#include <atomic>

namespace common {
namespace concurrent {
class SpinLock {
private:
  std::atomic<bool> mLock{false};

public:
  inline void lock() {
    while (std::atomic_exchange_explicit(&mLock, true,
                                         std::memory_order_acquire)) {
    }
  }

  inline void unlock() {
    std::atomic_store_explicit(&mLock, false, std::memory_order_release);
  }
};

template <typename LockType> class LockGuard {
private:
  const LockType &mLock;

public:
  explicit LockGuard(const LockType &spinLock) : mLock(spinLock) {
    const_cast<LockType &>(mLock).lock();
  }
  virtual ~LockGuard() { const_cast<LockType &>(mLock).unlock(); }
};
} // namespace concurrent
} // namespace common

#endif /* _GUID_61490d1a_68e7_414a_938f_482060433885_SpinLock_H_ */
