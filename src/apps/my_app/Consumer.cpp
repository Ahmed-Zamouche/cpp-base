#include "Consumer.hpp"

#include <memory>

namespace my_app {

using namespace common;
using namespace common::logger;
using namespace common::concurrent::actor;

std::shared_ptr<Producer> m_producer;

void Consumer::onStart() {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  Timer::create(this, 1000, 3000);
}

void Consumer::onStop() {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
}

void Consumer::onMessage(const IMessage &) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
}

void Consumer::onMessage(const TimerSharedPtr &) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  m_producer->post(Producer::Message());
}

void Consumer::onMessage(const Consumer::Message &) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
}

void Consumer::onMessage(const ProducerSharedPtr &producer) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  m_producer = producer;
  post(Message());
}

} // namespace my_app
