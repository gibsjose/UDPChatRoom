//Example server application

//This server will handle ALL clients that connect to it
//This is done by using select(), which checks if a file descriptor actually
//has data to read on it before blocking on the socket

#include <sys/socket.h> //Socket features
#include <netinet/in.h> //Internet-specific features of sockets
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define PORT 8888   //Default server port
#define LEN 1024    //Arbitrary string length

void getAddressAndPort(struct sockaddr_in *s, char *addr, size_t addr_size, uint16_t *port);

int main(int argc, char *argv[]) {

    //Open socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Set of file descriptors
    fd_set sockets;

    //Initialize fd set to be empty
    FD_ZERO(&sockets);

    //Create server/client address structures:
    // Server needs to know both the address of itself and the client
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;    //Specifies the address on which the server will listen to for clients...
                                                //INADDR_ANY specifies to listen on ANY address, but with a fixed port

    //Bind: Similar to 'connect()': Associating the socket with the address, without contacting the address
    //Bind, on the server side, tells all incoming data on the server address to go to the specified socket
    bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    //Listen:
    //  socket descriptor
    //  size of backlog of connections: Drop connections after N UNHANDLED connections
    listen(sockfd, 10);

    //Add the socket to the set
    FD_SET(sockfd, &sockets);

    //Length of socket
    unsigned int len = sizeof(clientaddr);

    //Handle connections
    while(1) {

        //Select call checks which socket has data to read
        //  Select call is destructive in terms of its inputs: Must make a copy of parameters to send
        fd_set tmp_set = sockets;

        //Select: Filters the set and checks if we can read from any of them, only leaving the FDs that
        //are available to read from
        //  0: maximum integer that must be selectable
        //  1: address of the set of descriptors to check for READ availability
        //  2: address of the set of descriptors to check for WRITE availability
        //  3: address of the set of descriptors to check for ERROR availability
        //  4: timeout: How long select call should block until at least one of the FDs is available to read/write
        //  RET: Number of descriptors available in the set
        select(FD_SETSIZE, &tmp_set, NULL, NULL, NULL);

        //Loop over set
        for(int i = 0; i < FD_SETSIZE; i++) {

            //FD_ISSET checks if the file descriptor at the index is a member of the set
            if(FD_ISSET(i, &tmp_set)) {

                //Two cases of sockets:
                //  1) Sockets to read data on
                //  2) Socket I was using to listen for new clients

                //Case 2: Socket I was using to listen for new clients
                if(i == sockfd) {
                    //Accept connections to the socket, returning a CLIENT socket descriptor, used ONLY for communicating with that
                    //specific client
                    //Accept is BLOCKING: It will wait until at least 1 client tries to connect
                    //The clientaddr structure is filled by accept, filling the client IP address and port number
                    int clientsocket = accept(sockfd, (struct sockaddr *) &clientaddr, &len);

                    char addr[LEN];
                    uint16_t prt = 0;
                    getAddressAndPort(&clientaddr, addr, LEN, &prt);
                    printf("<---> New client connected (%s:%u)\n\n", addr, prt);

                    //Add the new client socket to the set
                    FD_SET(clientsocket, &sockets);
                }

                //Not a new connection
                else {

                    //Receive data on the client socket
                    char r_line[LEN];

                    //Clear the receive buffer
                    memset(r_line, 0, LEN);

                    //Receive data on the client socket
                    int n = recv(i, r_line, LEN, 0);

                    //Client wishes to close connection (sent "/exit")
                    if(!strcmp(r_line, "/exit") || (n == 0)) {
                        //Close the client
                        close(i);

                        //Remove the client socket from the set
                        FD_CLR(i, &sockets);

                        //Print to the log
                        char addr[LEN];
                        uint16_t prt = 0;
                        getAddressAndPort(&clientaddr, addr, LEN, &prt);
                        printf("\n<-x-> Client disconnected (%s:%u)\n", addr, prt);
                    }

                    //Client sent text: Relay to all clients
                    else {

                        //Display data received
                        printf("<--- Received %d bytes from client (%d): %s\n", n, i, r_line);

                        //Echo data back to all clients
                        for(int j = 0; j < FD_SETSIZE; j++) {

                            //Only send to client sockets that are open
                            if(FD_ISSET(j, &sockets) && (j != sockfd) && (j != i)) {

                                printf("---> Sending %d bytes to client (%d): %s\n", (int)strlen(r_line), j, r_line);

                                //Echo message to client
                                send(j, r_line, strlen(r_line), 0);
                            }
                        }
                    }
                }
            }
        }
    }
}

void getAddressAndPort(struct sockaddr_in *s, char *addr, size_t addr_size, uint16_t *port) {

    struct in_addr t_in_addr;
    t_in_addr = inet_makeaddr(ntohl((unsigned long)s->sin_addr.s_addr), 0);
    inet_ntop(AF_INET, (const void * restrict)&t_in_addr, addr, addr_size);

    printf("\nPORT: \n");
    printf("raw: %u\n", s->sin_port);
    printf("ntohs: %u\n", ntohs(s->sin_port));
    printf("htons: %u\n", htons(s->sin_port));

    *port = ntohs(s->sin_port);
}
