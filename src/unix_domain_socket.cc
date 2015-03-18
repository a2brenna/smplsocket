#include "unix_domain_socket.h"
#include "error.h"

#include <netdb.h> //for struct addrinfo
#include <sys/un.h> //for sockaddr_un
#include <sys/socket.h> //for socket
#include <unistd.h> //for close

#define UNIX_MAX_PATH 108

Local_UDS::Local_UDS(const std::string &new_path){

    path = new_path;

    {
        struct addrinfo res;
        struct sockaddr_un address;

        const auto m = memset(&address, 0, sizeof(struct sockaddr_un));
        if( m != &address){
            throw smpl::Error("Failed to zero address structure");
        }

        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, path.c_str(), UNIX_MAX_PATH);
        address.sun_path[UNIX_MAX_PATH - 1] = '\0';

        res.ai_addr = (struct sockaddr *)&address;
        res.ai_addrlen = sizeof(struct sockaddr_un);

        sockfd =  socket(AF_UNIX, SOCK_STREAM, 0);
        if( sockfd < 0 ){
            throw smpl::Error("Failed to open socket");
        }

        const int yes = 1;
        const auto a = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        const auto b = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int));
        if ((a != 0) || (b != 0)) {
            throw smpl::Error("Failed to set socket option(s) SO_REUSEADDR or SO_REUSEPORT");
        }

        const int bind_result = bind(sockfd, res.ai_addr, res.ai_addrlen);
        if( bind_result < 0 ){
            throw smpl::Error("Failed to bind socket");
        }

    }

}

Local_UDS::~Local_UDS(){
    const int c = close(sockfd);
    if(c != 0){
        throw smpl::Error("Failed to cleanly close socket");
    }
}

smpl::Channel* Local_UDS::listen(){

    return NULL;
}

bool Local_UDS::check(){

    return false;
}

Remote_UDS::Remote_UDS(const std::string &new_path){
    path = new_path;
}

smpl::Channel* Remote_UDS::connect(){

    return NULL;
}
