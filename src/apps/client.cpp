/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <atomic>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "protocols/service.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class MyClient {
public:
  MyClient(std::shared_ptr<Channel> channel)
      : stub_(protocol::MyService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string sendRequest(const std::string &name) {

    std::cout << __PRETTY_FUNCTION__ << std::endl;
    // Data we are sending to the server.
    protocol::Request request;
    request.set_id(MyClient::ids++);
    request.set_name(name);

    // Container for the data we expect from the server.
    protocol::Response response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->action(&context, request, &response);

    std::cout << MyClient::ids << ": sent request: {"
              << request.ShortDebugString() << '}' << std::endl;

    // Act upon its status.
    if (status.ok()) {
      return '{' + response.ShortDebugString() + '}';
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<protocol::MyService::Stub> stub_;
  static std::atomic<int32_t> ids;
};

std::atomic<int32_t> MyClient::ids(0);

int RunClient(int argc, char **argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  std::cout << "unset http_proxy; unset https_proxy;" << std::endl;
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        target_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --target="
                  << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  } else {
    target_str = "localhost:50051";
  }
  MyClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string msg("ping");
  std::string response = client.sendRequest(msg);
  std::cout << "received: " << response << std::endl;

  return 0;
}
