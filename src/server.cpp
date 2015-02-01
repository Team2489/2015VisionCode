//C++ interface studd
#include <iostream>
#include <string>

//server include
#include "server.h"

using namespace std;

#define BACKLOG 10	 // how many pending connections queue will hold

//#define DEBUG 1

#if 1
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
#endif

   // get sockaddr, IPv4 or IPv6:
void *TCPServer::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

    
TCPServer::TCPServer(){


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    new_fd = -1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("server: socket");
    }
}

void TCPServer::InitiateSocket(char *port) {

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

    p = servinfo;

    int yes = 1;
#if 1
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof(int)) == -1) {
        perror("setsockopt");
    }
#endif
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        perror("server: bind");
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
    }

    signal(SIGPIPE, SIG_IGN);
}



bool TCPServer::ListenOnHost(){

    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    //if (new_fd < 0) timeout = NULL;

    if (select(sockfd+1, &fdset, NULL, NULL, &timeout) == -1){
        perror("select");
    }

    return FD_ISSET(sockfd, &fdset);
}

void TCPServer::AcceptConnection(){
   
    CloseConnection();
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd < 0) perror("accept");

    int flags = fcntl(new_fd, F_GETFL);
    fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);

    if (new_fd > -1) 
        printf("server: got connection from %s\n", s);

}

bool TCPServer::ListenOnClient(){

    //Escape if new_fd closed
    if (new_fd < 0) return false;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250000;

    FD_ZERO(&send_set);
    FD_SET(new_fd, &send_set);
    if (select(new_fd+1, &send_set, NULL, NULL, &timeout) == -1) perror("Remote listen");

    return FD_ISSET(new_fd, &send_set);
}

int TCPServer::Receive(){
#ifdef DEBUG
    cout << "new_fd is = " << new_fd << endl;
#endif
    if (new_fd < 0) return -1;

    int buffer;
    int numbytes;
#ifdef DEBUG
    cout << "Receiving!" << endl;
#endif
    if ((numbytes = recv(new_fd, &buffer, sizeof(buffer), 0)) == -1){
        CloseConnection();
        perror("recv");
    }
#ifdef DEBUG
    cout << "Finished receiving" << endl;
#endif /* DEBUG */
    return buffer;

}

void TCPServer::Send(struct directions direc){
#ifdef DEBUG
    cout << "Entering send, new_fd = " << new_fd << endl;
#endif
    //Skip if connection closed
    if (new_fd < 0) return; 

#ifdef DEBUG
    cout << "Sending!" << endl;
#endif
    
    FD_ZERO(&send_set);
    FD_SET(new_fd, &send_set);
    select(new_fd+1, NULL, &send_set, NULL, NULL);

    if (FD_ISSET(new_fd, &send_set)){

        if (send(new_fd, &direc, sizeof(struct directions), 0) == -1){
            CloseConnection();
            perror("send");
        }
    }

#ifdef DEBUG
    cout << "Sent!" << endl;
#endif
}

void TCPServer::CloseConnection(){

    close(new_fd);
    new_fd = -1;

}

TCPServer::~TCPServer(){
    close(new_fd);
    close(sockfd);
}

