#include "TCPRequestChannel.h"

using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {

    
    if (_ip_address == "") {
        // server side
        // set up variables
        struct sockaddr_in server;

        //socket - make socket - socket(int domain, int type, protocol)
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        //AD_INET = IPv4
        //SOCK_STREAM = TCP


        // provide necessary machine info for sockaddr_in
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(atoi(_port_no.c_str()));

        // address family, IPv4
        // IPv4 address, use current IPv4 address (INADDR_ANY)
        // connection port
        // convert short from host byte order to network byte order


        //bind 0 assign address to socket - bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
        bind(sockfd, (struct sockaddr *) &server, sizeof(server));

        //listen - listen for client - listen(int sockfd, int backlog)
        listen(sockfd, 1024);

        //accept - accept connection
        //written in a separate method
        
    }
    else {
        // client side
        
        struct sockaddr_in server_info;
        server_info.sin_family = AF_INET;
        server_info.sin_port = htons(atoi(_port_no.c_str()));

        //socket - make socket - socket(int domain, int type, protocol)
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        //generate server's info based on parameters
        // address family, IPv4
        //connection port
        //convery short from host byte order to network byte order
        //convert ip address c-string to binary represenation for sin_addr
        inet_pton(AF_INET, _ip_address.c_str(), &server_info.sin_addr);

        //connect - connect to listening socket - connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
        connect(sockfd, (struct sockaddr *) &server_info, sizeof(server_info));
    }

}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    //assign an existing socket to object's socket file descriptor
    this->sockfd = _sockfd;
}

TCPRequestChannel::~TCPRequestChannel () {
    //close socket - close(this->sockfd)
    close(this->sockfd);
}

int TCPRequestChannel::accept_conn () {
    //accept - accept connection - 
    //socket file dexcriptor for accepted connection
    //accept connection - accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    //return socket file descriptor
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int new_sockfd = accept(sockfd, (struct sockaddr *) &addr, &addrlen);

    if (new_sockfd < 0) {
        perror("accept failed");
    }

    return new_sockfd;
}

int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    //read from socket - read(int sockfd, void *buf, size_t count)
    //return number of bytes read
    return read(this->sockfd, msgbuf, msgsize);
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    //write to socket - write(int sockfd, const void *buf, size_t count)
    //return number of bytes written
    return write(this->sockfd, msgbuf, msgsize);
}
