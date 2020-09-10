#ifndef _GUID_ce458d39_6037_41d8_9423_76182cacb67d_Producer_HPP_
#define _GUID_ce458d39_6037_41d8_9423_76182cacb67d_Producer_HPP_

#include "Consumer.hpp"
#include "common/concurrent/Actor.hpp"
#include "common/logger/Logger.hpp"

#include <memory>
#include <vector>

namespace my_app {

class Consumer;

class Producer : public common::concurrent::actor::Actor<Producer> {

  using Logger = common::logger::Logger;
  using ConsumerSharedPtr = std::shared_ptr<Consumer>;
  using IMessage = common::concurrent::actor::IMessage;
  using TimerSharedPtr = std::shared_ptr<common::concurrent::actor::Timer>;

public:
  struct Message {};
  void onMessage(const IMessage &) override;
  void onMessage(const TimerSharedPtr &) override;

  void onMessage(const Producer::Message &);
  void onMessage(const std::vector<ConsumerSharedPtr> &consumers);

private:
  void onStart() override;
  void onStop() override;

  Logger &mLogger = Logger::get("Producer");
  std::vector<ConsumerSharedPtr> m_consumers;
};
} // namespace my_app
#endif /*_GUID_ce458d39_6037_41d8_9423_76182cacb67d_Producer_HPP_*/