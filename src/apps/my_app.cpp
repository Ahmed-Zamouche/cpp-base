#include "common/concurrent/Actor.hpp"
#include "common/logger/Logger.hpp"

#include "my_app/Consumer.hpp"
#include "my_app/Producer.hpp"

using namespace common;
using namespace common::logger;
using namespace common::concurrent::actor;

namespace my_app {

class Application : public Actor<Application> {

  using ConsumerSharedPtr = std::shared_ptr<Consumer>;

private:
  Logger &mLogger = Logger::get("MyApp");

  std::shared_ptr<Producer> m_producer;

  void onStart() override {
    mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();

    m_producer = Producer::create();

    std::vector<ConsumerSharedPtr> consumers{4};

    for (auto &consumer : consumers) {
      consumer = Consumer::create();
      consumer->post(m_producer);
    }
    m_producer->post(consumers);
    Timer::create(this, 10, 3000);
  }

  void onStop() override {
    mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  }

public:
  Application(int argc, char **argv) {
    (void)argc;
    (void)argv;
  }
  virtual ~Application() {}
  void onMessage(const IMessage &) override {
    mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  }

  void onMessage(const TimerSharedPtr &) override {
    mLogger(Level::INFO) << __PRETTY_FUNCTION__ << Endl();
  }
};

} // namespace my_app
int RunMyApp(int argc, char **argv) {

  return my_app::Application::run(argc, argv);
}
