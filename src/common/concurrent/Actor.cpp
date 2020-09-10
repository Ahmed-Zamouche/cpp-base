#include "Actor.hpp"
namespace common {
namespace concurrent {
namespace actor {
bool Timer::m_stop{false};
SpinLock Timer::m_lock;
Semaphore Timer::m_sem;
std::list<std::shared_ptr<Timer>> Timer::m_timers;
} // namespace actor
} // namespace concurrent
} // namespace common
