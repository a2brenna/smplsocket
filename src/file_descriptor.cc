#include "smplsocket.h"

#include <arpa/inet.h> //for htonl
#include <unistd.h>
#include <memory>
#include <cassert>

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

ssize_t smpl::File_Descriptor::_send(const char *msg, const size_t &msg_size) noexcept{
    std::unique_lock<std::mutex> lock(_write_lock);
    uint32_t net_length = htonl(msg_size);

    const auto l = ::send(_fd, &net_length, 4, MSG_NOSIGNAL);
    if ( l != 4 ){
        return -1;
    }

    const auto s = ::send(_fd, msg, msg_size, MSG_NOSIGNAL);
    if ( s != msg_size){
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
    msg.resize(message_size);
    char *buff = &msg[0];
    size_t read = 0;

    while (read < message_size) {
        const size_t to_read = message_size - read;

        const auto ret = ::recv(_fd, (buff + read), to_read, MSG_NOSIGNAL);

        if (ret == 0){
            return read;
        }
        else if(ret < 0){
            return ret;
        }
        else{
            read = read + ret;
        }
    }

    return read;
}

ssize_t smpl::File_Descriptor::_recv(char *buffer, const size_t &len) noexcept{
    std::unique_lock<std::mutex> lock(_read_lock);

    uint32_t net_length;

    const auto r = ::recv(_fd, &net_length, 4, MSG_NOSIGNAL);
    if(r != 4){
        return -1;
    }

    uint32_t message_size = ntohl(net_length);
    if(message_size > len){
        return -1;
    }
    size_t read = 0;

    while (read < message_size) {
        const size_t to_read = message_size - read;

        assert( (read + to_read) <= len);
        const auto ret = ::recv(_fd, (buffer + read), to_read, MSG_NOSIGNAL);

        if (ret == 0){
            return read;
        }
        else if(ret < 0){
            return ret;
        }
        else{
            read = read + ret;
        }
    }

    return read;
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
