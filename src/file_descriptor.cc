#include "smplsocket.h"

#include <arpa/inet.h> //for htonl
#include <unistd.h>

#define READ_WINDOW 4096

smpl::File_Descriptor::~File_Descriptor(){
    close(_fd);
}

smpl::File_Descriptor::File_Descriptor(const int &fd){
    _fd = fd;
}

ssize_t smpl::File_Descriptor::_send(const std::string &msg) noexcept{
    std::unique_lock<std::mutex> lock(_write_lock);
    ssize_t msg_length = msg.length();
    uint32_t net_length = htonl(msg_length);


    const auto l = ::send(_fd, &net_length, 4, MSG_NOSIGNAL);
    if ( l != 4 ){
        return -1;
    }

    const auto s = ::send(_fd, msg.c_str(), msg_length, MSG_NOSIGNAL);
    if ( s != msg_length ){
        return -1;
    }
    return s;
}

ssize_t smpl::File_Descriptor::_recv(std::string &msg) noexcept{
    msg.clear();
    std::unique_lock<std::mutex> lock(_read_lock);

    uint32_t net_length;

    const auto r = ::recv(_fd, &net_length, 4, MSG_NOSIGNAL);
    if(r != 4){
        return -1;
    }

    uint32_t message_size = ntohl(net_length);

    while (msg.length() < message_size) {
        char buff[READ_WINDOW];
        size_t to_read = std::min((size_t)READ_WINDOW, message_size - msg.length());

        const auto ret = ::recv(_fd, buff, to_read, MSG_NOSIGNAL);
        if (ret < 0) {
            return -1;
        }
        else if (ret == 0){
            return msg.size();
        }
        else{
            msg.append(buff, ret);
        }
    }

    return msg.size();
}

//TODO: check file descriptor status here?
smpl::CHANNEL_STATUS smpl::File_Descriptor::wait() noexcept{
    std::unique_lock<std::mutex> lock(_read_lock);
    fd_set set;
    FD_SET(_fd, &set);

    const auto ret = select(_fd + 1, &set, nullptr, nullptr, nullptr);

    if(ret < 0){
        return smpl::CHANNEL_BLOCKED;
    }
    else{
        return smpl::CHANNEL_READY;
    }
}
