#include <iostream>
#include <smpl.h>
#include <memory>
#include <smplsocket.h>
#include <string>
#include <chrono>

int main(int argc, char *argv[]){
    std::cout << "Starting test_server..." << std::endl;

    std::unique_ptr<smpl::Local_Address> uds(new smpl::Local_UDS("/tmp/smplsocket_test.sock"));
    std::unique_ptr<smpl::Channel> channel(uds->listen());

    const std::string message = channel->recv();
    channel->send(message);

    return 0;
}
