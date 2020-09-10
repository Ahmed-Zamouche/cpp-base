#ifndef _GUID_55f33a02_e2e3_4383_9815_6173faa07c3d_ISubscriber_HPP_
#define _GUID_55f33a02_e2e3_4383_9815_6173faa07c3d_ISubscriber_HPP_

#include "IPublisher.hpp"

#include <memory>

namespace common {

class IPublisher;

class ISubscriber {
public:
  ISubscriber();
  ISubscriber(IPublisher &publisher);

  ISubscriber(const ISubscriber &);

  ISubscriber(ISubscriber &&);

  friend bool operator==(const ISubscriber &lhs, const ISubscriber &rhs);
  friend bool operator!=(const ISubscriber &lhs, const ISubscriber &rhs) {
    return !(lhs == rhs);
  }

  virtual ~ISubscriber();

  void addPublisher(IPublisher &publisher);
  void removePublisher(IPublisher &publisher);
  virtual void update(const IPublisher &publisher) = 0;

private:
  friend class IPublisher;
  void enroll(IPublisher &publisher);
  void terminate(IPublisher &publisher);

private:
  class Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace common

#endif /* _GUID_55f33a02_e2e3_4383_9815_6173faa07c3d_ISubscriber_HPP_ */
