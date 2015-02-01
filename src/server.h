
//C and networking stuff
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

//16 bytes
struct directions{
    int diterror;
    int status;
};


class TCPServer{

    public:
    
    TCPServer();
    void InitiateSocket(char *port);
    bool ListenOnHost();
    bool ListenOnClient();
    void AcceptConnection();
    int Receive();
    void Send(struct directions direc);
    void CloseConnection();

    ~TCPServer();

    int m_connected;

    private:

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN];
	int rv;

    fd_set fdset, send_set;

    // get sockaddr, IPv4 or IPv6:
    void *get_in_addr(struct sockaddr *sa);
};

extern struct directions direc;
