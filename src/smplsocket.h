#ifndef __SMPLSOCKET_H___
#define __SMPLSOCKET_H___

#include "smpl.h"
#include <mutex>

//TODO:Investigate how POSIX fds are reused and see if we can make guarantees
//about whether or not the underlying fd has been changed here... maybe some
//sort of global locked registry? Prayer?
//

const size_t CONFIG_MAX_UDP = 512;

namespace smpl{

class File_Descriptor : public smpl::Channel {
private:
        int _fd;
        std::mutex _read_lock;
        std::mutex _write_lock;
        virtual ssize_t _send(const std::string &msg) noexcept;
        virtual ssize_t _send(const char *msg, const size_t &msg_size) noexcept;
        virtual ssize_t _recv(std::string &msg) noexcept;
        virtual ssize_t _recv(char *buffer, const size_t &buffer_len) noexcept;

    public:

        virtual ~File_Descriptor();
        File_Descriptor(const int &fd);

        virtual CHANNEL_STATUS wait() noexcept;

};

class Local_Port: public smpl::Local_Address {

    private:
        std::string ip;
        int port = -1;
        int sockfd = -1;
        bool _initialize(const std::string &new_ip, const int &new_port) noexcept;
        virtual smpl::Channel* _listen() noexcept;

    public:

        Local_Port(const std::string &new_ip, const int &new_port){
            if(_initialize(new_ip, new_port)){
                return;
            }
            else{
                throw Bad_Address();
            }
        };

        virtual ~Local_Port();
        virtual ADDRESS_STATUS check() noexcept;

};

class Remote_Port : public smpl::Remote_Address {

    private:
        std::string ip;
        int port = -1;
        bool _initialize(const std::string &new_ip, const int &new_port) noexcept;
        virtual smpl::Channel* _connect() noexcept;

    public:

        Remote_Port(const std::string &new_ip, const int &new_port){
            if(_initialize(new_ip, new_port)){
                return;
            }
            else{
                throw Bad_Address();
            }
        };

};

class Local_UDS : public smpl::Local_Address {

    private:
        std::string path;
        int sockfd = -1;
        bool _initialize(const std::string &new_path) noexcept;
        virtual smpl::Channel* _listen() noexcept;

    public:

        Local_UDS(const std::string &new_path){
            if(_initialize(new_path)){
                return;
            }
            else{
                throw Bad_Address();
            }
        };

        virtual ~Local_UDS();
        virtual ADDRESS_STATUS check() noexcept;

};

class Remote_UDS : public smpl::Remote_Address {

    private:
        std::string path;
        bool _initialize(const std::string &new_path) noexcept;
        virtual smpl::Channel* _connect() noexcept;

    public:

        Remote_UDS(const std::string &new_path){
            if(_initialize(new_path)){
                return;
            }
            else{
                throw Bad_Address();
            }
        };

};

class Remote_UDP : public smpl::Remote_Postbox {

    private:
        std::string _ip;
        int _port;
        int _sockfd;
        std::mutex _lock;

        bool _initialize(const std::string &new_ip, const int &new_port);

    public:

        Remote_UDP(const std::string &new_ip, const int &new_port){
            if(_initialize(new_ip, new_port)){
                return;
            }
            else{
                //throw Bad_Address();
            }
        };

        virtual ~Remote_UDP() noexcept;

        virtual void send(const std::string &m) noexcept;

};

class Local_UDP : public smpl::Local_Postbox{

    private:
        std::string _ip;
        int _port;
        int _sockfd;
        std::mutex _lock;

        bool _initialize(const std::string &new_ip, const int &new_port);

    public:

        Local_UDP(const std::string &new_ip, const int &new_port){
            if(_initialize(new_ip, new_port)){
                return;
            }
            else{
                //throw Bad_Address();
            }
        };
        Local_UDP(const std::string &new_ip, const int &new_port, const size_t &max_msg_size) :
            Local_UDP(new_ip, new_port)
        {
        };

        virtual ~Local_UDP() noexcept;

        virtual std::string recv() noexcept;

};

}

#endif
