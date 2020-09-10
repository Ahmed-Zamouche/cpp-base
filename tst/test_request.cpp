#include "protocols/request.pb.h"

#include <fstream>

#include <google/protobuf/message.h>
#include <gtest/gtest.h>

using namespace google::protobuf;

TEST(RequestTest, RequestId_NotSet_DefaultValue) {
  // Arrange
  protocol::Request request;
  // Act
  // Assert
  ASSERT_TRUE(request.id() == 0);
}

TEST(RequestTest, RequestId_Set_SetValue) {
  // Arrange
  protocol::Request response;
  // Act
  response.set_id(1);
  // Assert
  ASSERT_TRUE(response.id() == 1);
}

TEST(RequestTest, RequestSerialize_AllRequiredFiledSet_Success) {
  // Arrange
  protocol::Request request;
  request.set_id(1);
  request.set_name("request");
  size_t size = request.ByteSizeLong();
  auto data = std::unique_ptr<uint8_t>(new uint8_t[size]);
  // Act
  bool retVal = request.SerializeToArray(data.get(), size);
  // Assert
  ASSERT_TRUE(retVal);
}
