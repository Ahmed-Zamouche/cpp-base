#include "IPublisher.hpp"

#include "concurrent/SpinLock.h"
#include "logger/Logger.hpp"

#include <set>

namespace common {

using namespace concurrent;

class IPublisher::Impl {
public:
  Impl() {}

  SpinLock mLock;

  struct SubscriberRefCompare {
    bool operator()(const std::reference_wrapper<ISubscriber> &lhs,
                    const std::reference_wrapper<ISubscriber> &rhs) const {
      return (&lhs.get() < &rhs.get());
    }
  };

  std::set<std::reference_wrapper<ISubscriber>, SubscriberRefCompare>
      mSubscribers;
};

IPublisher::IPublisher() : pImpl(std::make_unique<Impl>()) {}

IPublisher::IPublisher(const IPublisher &o) : pImpl(std::make_unique<Impl>()) {
  pImpl->mSubscribers = o.pImpl->mSubscribers;
  for (const auto &subscriber : pImpl->mSubscribers) {
    subscriber.get().enroll(*this);
  }
}

IPublisher IPublisher::operator=(const IPublisher &o) {
  if (this != &o) {
    for (const auto &subscriber : pImpl->mSubscribers) {
      subscriber.get().terminate(*this);
    }
    pImpl = std::make_unique<Impl>();

    pImpl->mSubscribers = o.pImpl->mSubscribers;

    for (const auto &subscriber : pImpl->mSubscribers) {
      subscriber.get().enroll(*this);
    }
  }
  return *this;
}

IPublisher::IPublisher(IPublisher &&o) : pImpl(std::make_unique<Impl>()) {
  pImpl->mSubscribers = std::move(o.pImpl->mSubscribers);
  for (const auto &subscriber : pImpl->mSubscribers) {
    subscriber.get().terminate(o);
    subscriber.get().enroll(*this);
  }
}

IPublisher IPublisher::operator=(IPublisher &&o) {
  if (this != &o) {
    for (const auto &subscriber : pImpl->mSubscribers) {
      subscriber.get().terminate(*this);
    }
    pImpl = std::make_unique<Impl>();
    pImpl->mSubscribers.swap(o.pImpl->mSubscribers);
    for (const auto &subscriber : o.pImpl->mSubscribers) {
      subscriber.get().terminate(o);
      subscriber.get().enroll(*this);
    }
  }
  return *this;
}

bool operator==(const IPublisher &lhs, const IPublisher &rhs) {
  if (&lhs == &rhs) {
    return true;
  } else if (lhs.pImpl->mSubscribers.size() != rhs.pImpl->mSubscribers.size()) {
    return false;
  } else {
    for (const auto &subscriber : lhs.pImpl->mSubscribers) {
      if (rhs.pImpl->mSubscribers.find(subscriber.get()) ==
          rhs.pImpl->mSubscribers.end()) {
        return false;
      }
    }
  }
  return true;
}

IPublisher::~IPublisher() {
  for (const auto &subscriber : pImpl->mSubscribers) {
    subscriber.get().terminate(*this);
  }
}

void IPublisher::subscribe(ISubscriber &subscriber) {
  const LockGuard<SpinLock> lockGuard{pImpl->mLock};
  pImpl->mSubscribers.insert(subscriber);
}

void IPublisher::unsubscribe(ISubscriber &subscriber) {
  const LockGuard<SpinLock> lockGuard{pImpl->mLock};
  pImpl->mSubscribers.erase(subscriber);
}

void IPublisher::notifyAll() const {
  const LockGuard<SpinLock> lockGuard{pImpl->mLock};
  for (const auto &subscriber : pImpl->mSubscribers) {
    subscriber.get().update(*this);
  }
}

} // namespace common
