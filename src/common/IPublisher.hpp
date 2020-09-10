#ifndef _GUID_797ecde4_9b35_445a_bbee_1fabb425714d_IPublisher_HPP_
#define _GUID_797ecde4_9b35_445a_bbee_1fabb425714d_IPublisher_HPP_

#include "ISubscriber.hpp"

#include <memory>

namespace common {

class ISubscriber;

class IPublisher {
protected:
  IPublisher();

public:
  IPublisher(const IPublisher &);
  IPublisher operator=(const IPublisher &);

  IPublisher(IPublisher &&);
  IPublisher operator=(IPublisher &&);

  friend bool operator==(const IPublisher &lhs, const IPublisher &rhs);
  friend bool operator!=(const IPublisher &lhs, const IPublisher &rhs) {
    return !(lhs == rhs);
  }

  virtual ~IPublisher();

  void notifyAll() const;

private:
  friend class ISubscriber;
  void subscribe(ISubscriber &subscriber);
  void unsubscribe(ISubscriber &subscriber);

  class Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace common

#endif /* _GUID_797ecde4_9b35_445a_bbee_1fabb425714d_IPublisher_HPP_ */
