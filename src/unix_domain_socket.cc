#include "smplsocket.h"

#include <netdb.h> //for struct addrinfo
#include <sys/un.h> //for sockaddr_un
#include <sys/socket.h> //for socket
#include <unistd.h> //for close

#define UNIX_MAX_PATH 108

int smpl::open_unix_domain_socket(const std::string &new_path){
    struct addrinfo res;
    struct sockaddr_un address;

    const auto m = memset(&address, 0, sizeof(struct sockaddr_un));
    if( m != &address){
        return -1;
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, new_path.c_str(), UNIX_MAX_PATH);
    address.sun_path[UNIX_MAX_PATH - 1] = '\0';

    res.ai_addr = (struct sockaddr *)&address;
    res.ai_addrlen = sizeof(struct sockaddr_un);

    const int sockfd =  socket(AF_UNIX, SOCK_STREAM, 0);
    if( sockfd < 0 ){
        return -1;
    }

    const int yes = 1;
    const auto a = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    const auto b = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int));
    if ((a != 0) || (b != 0)) {
        return -1;
    }

    const int bind_result = bind(sockfd, res.ai_addr, res.ai_addrlen);
    if( bind_result < 0 ){
        return -1;
    }

    const int l = ::listen(sockfd, SOMAXCONN);
    if( l < 0 ){
        return -1;
    }

    return sockfd;
}

bool smpl::Local_UDS::_initialize(const std::string &new_path) noexcept{
    path = new_path;
    if ( path.size() > UNIX_MAX_PATH ){
        return false;
    }

    sockfd = open_unix_domain_socket(new_path);

    if(sockfd == -1){
        return false;
    }
    else{
        return true;
    }
}

smpl::Local_UDS::~Local_UDS(){
    close(sockfd);
    remove(path.c_str());
}

smpl::Channel* smpl::Local_UDS::_listen() noexcept{

    const int a = accept(sockfd, nullptr, nullptr);
    if( a < 0 ){
        return nullptr;
    }

    const auto fd = new File_Descriptor(a);

    if( fd == nullptr ){
        return nullptr;
    }

    return fd;
}

smpl::ADDRESS_STATUS smpl::Local_UDS::check() noexcept{
    return smpl::ADDRESS_ERROR;
}

bool smpl::Remote_UDS::_initialize(const std::string &new_path) noexcept{
    path = new_path;
    if ( new_path.size() > UNIX_MAX_PATH ){
        return false;
    }
    else{
        path = new_path;
        return true;
    }
}

smpl::Channel* smpl::Remote_UDS::_connect() noexcept{
    const int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if( sockfd < 0 ){
        return nullptr;
    }

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, path.c_str(), UNIX_MAX_PATH);
    const auto len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    const auto c = ::connect(sockfd, (struct sockaddr *)&remote, len);
    if( c != 0 ){
        return nullptr;
    }

    const auto fd = new File_Descriptor(sockfd);
    if( fd == nullptr ){
        return nullptr;
    }

    return fd;
}
