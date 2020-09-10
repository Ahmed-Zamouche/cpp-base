#include "ISubscriber.hpp"

#include "concurrent/SpinLock.h"
#include "logger/Logger.hpp"

#include <set>

namespace common {

using namespace concurrent;

class ISubscriber::Impl {
public:
  Impl() {}
  SpinLock mLock;
  struct PublisherRefCompare {
    bool operator()(const std::reference_wrapper<IPublisher> &lhs,
                    const std::reference_wrapper<IPublisher> &rhs) const {
      return (&lhs.get() < &rhs.get());
    }
  };

  std::set<std::reference_wrapper<IPublisher>, PublisherRefCompare> mPublishers;
};

ISubscriber::ISubscriber() : pImpl(std::make_unique<Impl>()) {}

ISubscriber::ISubscriber(IPublisher &publisher) : ISubscriber() {
  pImpl->mPublishers.insert(publisher);
  publisher.subscribe(*this);
}

ISubscriber::ISubscriber(const ISubscriber &o)
    : pImpl(std::make_unique<Impl>()) {
  pImpl->mPublishers = o.pImpl->mPublishers;
  for (const auto &publisher : pImpl->mPublishers) {
    publisher.get().subscribe(*this);
  }
}

ISubscriber::ISubscriber(ISubscriber &&o) : pImpl(std::make_unique<Impl>()) {
  pImpl->mPublishers.swap(o.pImpl->mPublishers);
  for (const auto &publisher : pImpl->mPublishers) {
    publisher.get().unsubscribe(o);
    publisher.get().subscribe(*this);
  }
}

bool operator==(const ISubscriber &lhs, const ISubscriber &rhs) {
  if (&lhs == &rhs) {
    return true;
  } else if (lhs.pImpl->mPublishers.size() != rhs.pImpl->mPublishers.size()) {
    return false;
  } else {
    for (const auto &publisher : lhs.pImpl->mPublishers) {
      if (rhs.pImpl->mPublishers.find(publisher.get()) ==
          rhs.pImpl->mPublishers.end()) {
        return false;
      }
    }
  }

  return true;
}

ISubscriber::~ISubscriber() {
  for (const auto &publisher : pImpl->mPublishers) {
    publisher.get().unsubscribe(*this);
  }
}

void ISubscriber::terminate(IPublisher &publisher) {
  const LockGuard<SpinLock> lockGuard{pImpl->mLock};
  pImpl->mPublishers.erase(publisher);
}
void ISubscriber::enroll(IPublisher &publisher) {
  const LockGuard<SpinLock> lockGuard{pImpl->mLock};
  pImpl->mPublishers.insert(publisher);
}

void ISubscriber::addPublisher(IPublisher &publisher) {
  {
    const LockGuard<SpinLock> lockGuard{pImpl->mLock};
    pImpl->mPublishers.insert(publisher);
  }
  publisher.subscribe(*this);
}

void ISubscriber::removePublisher(IPublisher &publisher) {
  {
    const LockGuard<SpinLock> lockGuard{pImpl->mLock};
    pImpl->mPublishers.erase(publisher);
  }
  publisher.unsubscribe(*this);
}

} // namespace common
