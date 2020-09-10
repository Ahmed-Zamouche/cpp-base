#ifndef _GUID_14656a5a_8325_4acd_ae5f_ab683b42214b_Consumer_HPP_
#define _GUID_14656a5a_8325_4acd_ae5f_ab683b42214b_Consumer_HPP_

#include "Producer.hpp"
#include "common/concurrent/Actor.hpp"
#include "common/logger/Logger.hpp"

#include <memory>

namespace my_app {

class Producer;

class Consumer : public common::concurrent::actor::Actor<Consumer> {

  using Logger = common::logger::Logger;
  using ProducerSharedPtr = std::shared_ptr<Producer>;
  using IMessage = common::concurrent::actor::IMessage;
  using TimerSharedPtr = std::shared_ptr<common::concurrent::actor::Timer>;

public:
  struct Message {};
  void onMessage(const IMessage &) override;
  void onMessage(const TimerSharedPtr &) override;

  void onMessage(const Consumer::Message &);
  void onMessage(const ProducerSharedPtr &);

private:
  void onStart() override;
  void onStop() override;

  Logger &mLogger = Logger::get("Consumer");
  ProducerSharedPtr m_producer;
};
} // namespace my_app
#endif /*_GUID_14656a5a_8325_4acd_ae5f_ab683b42214b_Consumer_HPP_*/