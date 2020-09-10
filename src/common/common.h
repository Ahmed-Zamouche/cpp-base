
#ifndef _GUID_9201bc8a_1afa_4f17_8cfb_b3be5b7dc85f_Common_H_
#define _GUID_9201bc8a_1afa_4f17_8cfb_b3be5b7dc85f_Common_H_

#include <chrono>

namespace common {
template <typename F, typename... Args>
double measureRunTime(const F &func, Args &&... args) {

  auto t0 = std::chrono::high_resolution_clock::now();
  func(std::forward<Args>(args)...);
  auto t1 = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
};
} // namespace common
#endif /*_GUID_9201bc8a_1afa_4f17_8cfb_b3be5b7dc85f_Common_H_*/