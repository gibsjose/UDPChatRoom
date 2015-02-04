//Example client application

#include <sys/socket.h> //Socket features
#include <netinet/in.h> //Internet-specific features of sockets
#include <arpa/inet.h>  //inet_addr()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/select.h>


#define MAX_INPUT_SIZE (1024)

//Trim the first newline character from the string
void ntrim(char *str);

int main(int argc, char *argv[]) {

    char port_str[16];
    char address[64];
    unsigned int port;

    //Create a socket:
    //socket() system call creates a socket, returning a socket descriptor
    //  AF_INET specifies the address family for internet
    //  SOCK_STREAM says we want TCP as our transport layer, by requesting stream-oriented communication
    //  0 is a parameter used for some options for certain types of sockets, unused for INET sockets
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    fd_set sockets;
    // Clear the fd set
    FD_ZERO(&sockets);

    if(sockfd < 0) {
        printf("Could not open socket\n");
        return -1;
    }

    //Prompt user for server address and port
    printf("Server address: ");
    fgets(address, 64, stdin);
    ntrim(address);

    printf("Server port: ");
    fgets(port_str, 16, stdin);
    ntrim(port_str);
    port = atoi(port_str);

    printf("===========================================\n");
    printf("Connecting to %s:%d\n", address, port);
    printf("===========================================\n\n");

    //inet_addr() converts a string-address into the proper type
    //Specify the address for the socket
    //Create the socket address structure and populate it's fields
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;                            //Specify the family again (AF_INET = internet family)
    serveraddr.sin_port = htons(port);                          //Specify the port on which to send data (16-bit) (# < 1024 is off-limits)
    serveraddr.sin_addr.s_addr = inet_addr(address);        //Specify the IP address of the server with which to communicate


    //Open TCP "Connection"
    //Specify the socket descriptor, the server address structure, and the size of the socket address structure
    int e = connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr_in));

    if(e < 0) {
        printf("Could not connect to server\n");
        return -1;
    }

    //Add server socket and stdin to file desciptor set
    FD_SET(sockfd, &sockets);
    FD_SET(0, &sockets);    //Add stdin to fd set

    //Can now send/receive data over the socket...

    //Ask for some text
    printf("Chat room client\n");
    char * response = malloc(MAX_INPUT_SIZE);

    while(1)
    {
        // Temp file descriptor set
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

        //Iterate over the set
        for (int i = 0; i < FD_SETSIZE; i++) {
            //If the set is member of the temp set
            if(FD_ISSET(i, &tmp_set)) {

                // If socket is the server connection, read the data
                if(i == sockfd) {
                    int n = recv(sockfd, response, MAX_INPUT_SIZE, 0);

                    //Error
                    if(n <= 0) {
                        //Server has potentially closed the connection (check for specific error value)
                        printf("Error reading data from server\n");
                        return -1;
                    }
                    ntrim(response);
                    printf("%s\n", response);
                    memset(response, 0, MAX_INPUT_SIZE);

                }
                else {
                    char r_line[MAX_INPUT_SIZE];
                    read(i, r_line, MAX_INPUT_SIZE);
                    //Send the data over the socket:
                    //  socket descriptor
                    //  data (const void *)
                    //  size of the data
                    //  optional settings
                    ntrim(r_line);
                    send(sockfd, r_line, strlen(r_line), 0);

                    // Quit the program if /quit input
                    if(!strcmp(r_line, "/exit"))
                    {
                        free(response);
                        close(sockfd);
                        return 0;
                    }
                }
            }
        }
    }

    free(response);
    return 0;
}

//Trim the first newline character from the string
void ntrim(char *str) {
    for(int i = 0; i < strlen(str); i++) {
        if(str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}
