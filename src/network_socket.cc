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
    //TODO: check validity of ip
    port = new_port;
    //TODO: check range on port

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

bool smpl::Local_UDP::_initialize(const std::string &new_ip, const int &new_port){
    _ip = new_ip;
    //TODO: check validity of ip
    _port = new_port;
    //TODO: check range on port

    std::stringstream s;
    s << _port;
    const std::string port_string = s.str();

    struct addrinfo *r = nullptr;
    Tripwire t(std::bind(safe_freeaddrinfo, r));

    const int addrinfo_status = getaddrinfo(_ip.c_str(), port_string.c_str(), nullptr, &r);
    if (addrinfo_status != 0) {
        return false;
    }
    if ( r == nullptr ){
        return false;
    }

    _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd < 0) {
        return false;
    }

    const int yes = 1;
    const auto sa = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    const auto sb = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int));
    if ((sa != 0) || (sb != 0)) {
        return false;
    }

    bool bound = false;
    for(auto s = r; s != nullptr; s = s->ai_next){
        const int b = bind(_sockfd, s->ai_addr, s->ai_addrlen);
        if (b == 0) {
            bound = true;
            break;
        }
        else{
            continue;
        }
    }
    if( bound ){
        return true;
    }
    else{
        return false;
    }

}

bool smpl::Remote_UDP::_initialize(const std::string &new_ip, const int &new_port){
    _ip = new_ip;
    //check validity of ip
    _port = new_port;
    //check range on port

    std::stringstream s;
    s << _port;
    const std::string port_string = s.str();

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *r = nullptr;
    Tripwire t(std::bind(safe_freeaddrinfo, r));

    const auto addrinfo_status = getaddrinfo(_ip.c_str(), port_string.c_str(), &hints, &r);
    if (addrinfo_status != 0) {
        return false;
    }
    if ( r == nullptr ){
        return false;
    }

    _sockfd = -1;
    for(auto s = r; s != nullptr; s = s->ai_next){
        int _s;
        _s = socket(AF_INET, SOCK_DGRAM, 0);
        if (_s < 0) {
            return false;
        }

        const int c = ::connect(_s , s->ai_addr, s->ai_addrlen);
        if (c < 0) {
            close(_s);
        }
        else{
            _sockfd = _s;
            break;
        }
    }

    if(_sockfd < 0){
        return false;
    }
    else{
        return true;
    }

}

std::string smpl::Local_UDP::recv() noexcept{
    std::unique_lock<std::mutex> lock(_lock);
    std::string msg;
    msg.clear();

    uint32_t net_length;
    const auto r = ::recv(_sockfd, &net_length, 4, MSG_NOSIGNAL);
    (void)r;
    uint32_t message_size = ntohl(net_length);

    msg.resize(message_size);
    char *buff = &msg[0];
    size_t read = 0;

    while (read < message_size) {
        const size_t to_read = message_size - read;

        const auto ret = ::recv(_sockfd, (buff + read), to_read, MSG_NOSIGNAL);

        if (ret <= 0){
            break;
        }
        else{
            read = read + ret;
        }
    }

    return msg;
}

void smpl::Remote_UDP::send(const std::string &msg) noexcept{
    //TODO: de-dup this and File_Descriptor code
    std::unique_lock<std::mutex> lock(_lock);
    ssize_t msg_length = msg.length();
    uint32_t net_length = htonl(msg_length);

    const auto l = ::send(_sockfd, &net_length, 4, MSG_NOSIGNAL);
    (void)l;
    const auto s = ::send(_sockfd, msg.c_str(), msg_length, MSG_NOSIGNAL);
    (void)s;
    return;
}
