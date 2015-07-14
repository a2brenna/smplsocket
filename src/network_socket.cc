#include "smplsocket.h"

#include <netdb.h> //for struct addrinfo
#include <sys/un.h> //for sockaddr_un
#include <sys/socket.h> //for socket
#include <unistd.h> //for close
#include <memory> //for unique_ptr
#include "tripwire.h" //for safe use of freeaddrinfo()

#include <sstream>

//I'm unsure what happens if you call freeaddrinfo(nullptr), hence this
//function.
void safe_freeaddrinfo(struct addrinfo *r){
    if(r){
        freeaddrinfo(r);
    }
}

bool smpl::Local_Port::_initialize(const std::string &new_ip, const int &new_port) noexcept{

    ip = new_ip;
    //check validity of ip
    port = new_port;
    //check range on port

    std::stringstream s;
    s << port;
    const std::string port_string = s.str();

    struct addrinfo *r = nullptr;
    Tripwire t(std::bind(safe_freeaddrinfo, r));

    const int addrinfo_status = getaddrinfo(ip.c_str(), port_string.c_str(), nullptr, &r);
    if (addrinfo_status != 0) {
        return false;
    }
    if ( r == nullptr ){
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }

    const int yes = 1;
    const auto sa = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    const auto sb = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int));
    if ((sa != 0) || (sb != 0)) {
        return false;
    }

    bool bound = false;
    for(auto s = r; s != nullptr; s = s->ai_next){
        const int b = bind(sockfd, s->ai_addr, s->ai_addrlen);
        if (b == 0) {
            bound = true;
            break;
        }
        else{
            continue;
        }
    }
    if( !bound ){
        return false;
    }

    const int l = ::listen(sockfd, SOMAXCONN);
    if (l < 0) {
        return false;
    }
    return true;
}

smpl::Local_Port::~Local_Port(){
    close(sockfd);
}

smpl::Channel* smpl::Local_Port::_listen() noexcept{

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

smpl::ADDRESS_STATUS smpl::Local_Port::check() noexcept{
    return smpl::ADDRESS_ERROR;
}

bool smpl::Remote_Port::_initialize(const std::string &new_ip, const int &new_port) noexcept{
    ip = new_ip;
    //check validity of ip
    port = new_port;
    //check range on port
    return true;
}

smpl::Channel* smpl::Remote_Port::_connect() noexcept{
    std::stringstream s;
    s << port;
    const std::string port_string = s.str();

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *r = nullptr;
    Tripwire t(std::bind(safe_freeaddrinfo, r));

    const auto addrinfo_status = getaddrinfo(ip.c_str(), port_string.c_str(), &hints, &r);
    if (addrinfo_status != 0) {
        return nullptr;
    }
    if ( r == nullptr ){
        return nullptr;
    }

    int sockfd = -1;
    for(auto s = r; s != nullptr; s = s->ai_next){
        int _s;
        _s = socket(AF_INET, SOCK_STREAM, 0);
        if (_s < 0) {
            return nullptr;
        }

        const int c = ::connect(_s , s->ai_addr, s->ai_addrlen);
        if (c < 0) {
            close(_s);
        }
        else{
            sockfd = _s;
            break;
        }
    }
    if(sockfd < 0){
        return nullptr;
    }

    smpl::File_Descriptor *new_fd = nullptr;
    try{
        new_fd = new smpl::File_Descriptor(sockfd);
    }
    catch(...){
        return nullptr;
    }
    return new_fd;

}
