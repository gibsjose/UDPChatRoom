//Example UDP client application

#include <sys/socket.h> //Socket features
#include <netinet/in.h> //Internet-specific features of sockets
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>   //timeval structure
#include <unistd.h>

int main(int argc, char *argv[]) {

    //Create a socket:
    //socket() system call creates a socket, returning a socket descriptor
    //  AF_INET specifies the address family for internet
    //  SOCK_DGRAM says we want UDP as our transport layer
    //  0 is a parameter used for some options for certain types of sockets, unused for INET sockets
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd < 0) {
        printf("Could not open socket\n");
        return -1;
    }

    //Create timeval struct
    struct timeval to;
    to.tv_sec = 5;
    to.tv_usec = 0;

    //Make socket only wait for 5 seconds w/ setsockopt
    //  socket descriptor
    //  socket level (internet sockets, local sockets, etc.)
    //  option we want (SO_RCVTIMEO = Receive timeout)
    //  timeout structure
    //  size of structure
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));

    //Specify the address for the socket
    //Create the socket address structure and populate it's fields
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;                            //Specify the family again (AF_INET = internet family)
    serveraddr.sin_port = htons(9876);                          //Specify the port on which to send data (16-bit) (# < 1024 is off-limits)
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");        //Specify the IP address of the server with which to communicate
                                                                //inet_addr() converts a string-address into the proper type

    //No need for 'connect()' on a UDP connection
        //Specify the socket descriptor, the server address structure, and the size of the socket address structure
        //int e = connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr_in));
        // if(e < 0) {
        //     printf("Could not connect to server\n");
        //     return -1;
        // }

    //Can now send/receive data over the socket...

    //Ask for some text
    printf("Enter a line... ");

    char s_line[1024];
    char r_line[1024];
    fgets(s_line, 1024, stdin);

    //sendto: Like 'send()': Sends the data over the socket, except does not require
    // a connection as in TCP:
    //  socket descriptor
    //  data (const void *)
    //  size of the data
    //  optional settings
    //  Server we want to send to
    //  Size of server structure

    //With UDP, whatever is in the send buffer is explicitly sent in a packet
    sendto(sockfd, s_line, strlen(s_line), 0, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    //recvfrom: Like 'recv()': Receive data over the socket:
    //  socket descriptor
    //  where to store data
    //  MAX_BYTES
    //  optional settings
    //  store the address of what we received from in serveraddr
    //  size of server address
    //Returns the number of bytes received...
    unsigned int len = sizeof(serveraddr);
    int n = recvfrom(sockfd, r_line, 1024, 0, (struct sockaddr *)&serveraddr, &len);  //Receive is BLOCKING: Will wait for SOME data, but not necessarily until MAX_BYTES

    if(n < 0) {
        printf("Error receiving bytes from server...\n");
        return -1;
    } else if(n == 0) {
        printf("No bytes were received from server...\n");
    } else {
        printf("Received %d bytes from server: %s\n", n, r_line);
    }

    //Close the socket
    close(sockfd);

    return 0;
}
