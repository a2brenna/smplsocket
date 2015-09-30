#include <iostream>
#include <smpl.h>
#include <smplsocket.h>
#include <memory>
#include <chrono>
#include <string>

int main(int argc, char *argv[]){
    std::unique_ptr<smpl::Remote_Address> uds(new smpl::Remote_UDS("/tmp/smplsocket_test.sock"));
    std::unique_ptr<smpl::Channel> channel(uds->connect());

    const std::string message(1000000000, 'a');
    const auto start = std::chrono::high_resolution_clock::now();
    channel->send(message);
    const std::string received_message = channel->recv();
    const auto end = std::chrono::high_resolution_clock::now();
    const auto elapsed_time = end - start;

    std::cout << received_message.size() << " " << elapsed_time.count() << std::endl;

    return 0;
}
