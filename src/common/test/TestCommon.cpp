
#include "common/IPersistent.hpp"
#include "common/IPublisher.hpp"
#include "common/ISubscriber.hpp"
#include "common/logger/ILoggable.hpp"
#include "common/logger/Logger.hpp"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <thread>
#include <typeinfo>

#include <google/protobuf/message.h>
#include <gtest/gtest.h>

#include <gmock/gmock.h>

using namespace google::protobuf;

using namespace common;
using namespace common::logger;

class MockSubscriber : public ISubscriber {
public:
  MOCK_METHOD1(update, void(const IPublisher &));
};

class TestPublisher : public IPublisher {};
class TestSubscriber : public ISubscriber {
public:
  void update(const IPublisher &) {}
};

template <typename T> class Setting : public IPublisher {
private:
  const std::string mName;
  T mValue;

public:
  explicit Setting(const std::string &name, const T &value = T())
      : mName(name), mValue(value){};
  virtual ~Setting(){};

  T getValue() const { return mValue; }
  void setValue(T value) {
    if (value != mValue) {
      mValue = value;
      notifyAll();
    }
  }

  std::string getName() const { return mName; };
};

class User : public ISubscriber, public IPersistent {
private:
  Logger &mLogger = Logger::get("User");

public:
  User() {}
  template <typename T> User(Setting<T> &setting) : ISubscriber(setting) {}
  virtual ~User() {}

  template <typename T> void update(const Setting<T> *setting) {
    mLogger(Level::NOTICE) << "user {this: " << this << ",  setting : { name "
                           << setting->getName()
                           << ", value : " << setting->getValue() << "}}"
                           << Endl();
  }

  void update(const IPublisher &publisher) override {

    if (typeid(publisher) == typeid(Setting<int>)) {
      update(dynamic_cast<const Setting<int> *>(&publisher));

    } else if (typeid(publisher) == typeid(Setting<std::string>)) {
      update(dynamic_cast<const Setting<std::string> *>(&publisher));
    }
  }

  void persist() const override{

  };
};

static int test_main() {

  Logger &mainLogger = Logger::get("Main");

  mainLogger() << "hello world" << Endl();

  Setting<int> intSetting("intSetting", 0);

  User user0(intSetting);
  intSetting.setValue(1);
  {
    Setting<std::string> strSetting("strSetting", std::string("zero"));
    User user1(intSetting);
    user0.addPublisher(strSetting);
    user1.addPublisher(strSetting);
    intSetting.setValue(2);
    strSetting.setValue("one");
    intSetting.setValue(3);
    strSetting.setValue("two");
  }
  intSetting.setValue(3);
  {
    std::srand(std::time(nullptr));

    std::function<void(unsigned int)> lambda =
        [&intSetting](unsigned int i) -> void {
      while (i--) {
        intSetting.setValue(std::rand());
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
      }
    };

    std::thread thread_obj0(lambda, 5);
    std::thread thread_obj1(lambda, 5);

    thread_obj1.join();
    thread_obj0.join();
  }
  mainLogger(Level::WARN) << "goodbye world" << Endl();

  return 0;
}

TEST(TestCommom, DISABLED_TestNull_ScenarioNull_ExpectNull) {
  // Arrange
  test_main();
  // Act
  // Assert
  ASSERT_TRUE(0 == 0);
}

TEST(TestPublisher, TestCopy_CopyrPublisher_CopyEqualOriginal) {
  TestPublisher publisher;

  TestSubscriber subscriber0;
  TestSubscriber subscriber1;

  subscriber0.addPublisher(publisher);
  subscriber1.addPublisher(publisher);

  TestPublisher publisherCopy{publisher};

  TestPublisher publisherAssingOpr;
  publisherAssingOpr = publisher;

  ASSERT_EQ(publisher, publisherCopy);
  ASSERT_EQ(publisher, publisherAssingOpr);
  ASSERT_EQ(publisherCopy, publisherAssingOpr);

  ASSERT_EQ(publisher, publisher);
}

TEST(TestPublisher, TestMove_MovePublisher_NewEqualOld) {
  TestPublisher publisher0;
  TestPublisher publisher1;

  TestSubscriber subscriber0;
  TestSubscriber subscriber1;

  subscriber0.addPublisher(publisher0);
  subscriber1.addPublisher(publisher0);
  TestPublisher publisherMove{std::move(publisher0)};
  ASSERT_NE(publisher0, publisherMove);
  ASSERT_EQ(publisher0, publisher1);

  subscriber0.addPublisher(publisher1);
  subscriber1.addPublisher(publisher1);
  TestPublisher publisherMoveOpr;
  publisherMoveOpr = std::move(publisher1);
  ASSERT_NE(publisher1, publisherMoveOpr);
  ASSERT_EQ(publisher0, publisher1);
  ASSERT_EQ(publisherMove, publisherMoveOpr);
}

TEST(TestPublisher, TestCopy_CopyrSubscriber_CopyEqualOriginal) {
  TestSubscriber subscriber;

  TestPublisher publisher;
  subscriber.addPublisher(publisher);
  TestSubscriber subscriberCopy{subscriber};
  ASSERT_EQ(subscriber, subscriberCopy);
  ASSERT_EQ(subscriber, subscriber);
}

TEST(TestSubscriber, TestMove_MoveSubscriber_NewEqualOld) {
  TestSubscriber subscriber;

  TestPublisher publisher0;
  TestPublisher publisher1;

  subscriber.addPublisher(publisher0);
  TestSubscriber subscriberMove{std::move(subscriber)};
  ASSERT_NE(subscriber, subscriberMove);

  subscriber.addPublisher(publisher0);
  ASSERT_EQ(subscriber, subscriberMove);
}

TEST(TestSubscriberPublisher,
     TestNotifyAllUpdate_NotifyPublisherSubscriber_SubscriberUpdated) {
  TestPublisher publisher;
  MockSubscriber subscriber;
  EXPECT_CALL(subscriber, update(::testing::Ref(publisher))).Times(2);
  subscriber.addPublisher(publisher);
  publisher.notifyAll();
  subscriber.addPublisher(publisher);
  publisher.notifyAll();
  subscriber.removePublisher(publisher);
  publisher.notifyAll();
  subscriber.removePublisher(publisher);
  publisher.notifyAll();
}
