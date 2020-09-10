
#include "Producer.hpp"

#include <memory>

namespace my_app {

using namespace common;
using namespace common::logger;
using namespace common::concurrent::actor;

void Producer::onStart() {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  Timer::create(this, 100, 2000);
}

void Producer::onStop() {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
}

void Producer::onMessage(const IMessage &) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
}

void Producer::onMessage(const TimerSharedPtr &) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  for (const auto &consumer : m_consumers) {
    consumer->post(Consumer::Message());
  }
}

void Producer::onMessage(const Producer::Message &) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  // m_consumer->stop();
}

void Producer::onMessage(const std::vector<ConsumerSharedPtr> &consumers) {
  mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  m_consumers = consumers;
  post(Message());
}

} // namespace my_app
