//Example UDP server application

#include <sys/socket.h> //Socket features
#include <netinet/in.h> //Internet-specific features of sockets
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {

    //Open socket
    //SOCK_DGRAM specifies a UDP socket (stands for Datagram)
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //Create server/client address structures:
    //Server needs to know both the address of itself and the client
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9876);
    serveraddr.sin_addr.s_addr = INADDR_ANY;    //Specifies the address on which the server will listen to for clients...
                                                //INADDR_ANY specifies to listen on ANY address, but with a fixed port

    //Bind: Similar to 'connect()': Associating the socket with the address, without contacting the address
    //Bind, on the server side, tells all incoming data on the server address to go to the specified socket
    bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    //No need to 'listen' on a UDP server: There are no connections

    //Handle clients
    while(1) {
        unsigned int len = sizeof(clientaddr);

        //Receive on the client socket
        char r_line[1024];
        memset(&r_line, 0, 1024);

        memset(r_line, 0, 1024);

        unsigned int n = recvfrom(sockfd, r_line, 5000, 0, (struct sockaddr *)&clientaddr, &len);

        //Display data received
        printf("Received %d bytes from client: %s\n", n, r_line);

        //Echo data back to client
        sendto(sockfd, r_line, strlen(r_line), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));

        //No need to close UDP socket since there is only one for all clients
    }
}
