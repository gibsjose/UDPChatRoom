//Example client application

#include "../defines.hpp"

#include <sys/socket.h> //Socket features
#include <netinet/in.h> //Internet-specific features of sockets
#include <arpa/inet.h>  //inet_addr()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <iostream>

bool send_get_ack(char * aBytes, size_t aLen, int aSocketFD, struct sockaddr * aDestAddr, socklen_t aDestAddrLen);
//Trim the first newline character from the string
void ntrim(char *str);

int main(int argc, char *argv[]) {

    char port_str[16];
    char address[64];
    unsigned int port;

    //Create a socket:
    //socket() system call creates a socket, returning a socket descriptor
    //  AF_INET specifies the address family for internet
    //  SOCK_DGRAM says we want UDP as our transport layer
    //  0 is a parameter used for some options for certain types of sockets, unused for INET sockets
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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

    //"Connect" to the server by sending it 'STX' and expect an 'ACK' back.
    char lSTX = STX_CHAR;
    if(!send_get_ack(&lSTX, 1, sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
        printf("Could not connect to server\n");
        return -1;
    }

    fd_set sockets;
    // Clear the fd set
    FD_ZERO(&sockets);

    //Add server socket and stdin to file desciptor set
    FD_SET(sockfd, &sockets);
    FD_SET(STDIN_FILENO, &sockets);    //Add stdin to fd set

    std::cout << "Chat room client" << std::endl;

    char * response = (char*)malloc(MAX_INPUT_SIZE);

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

      //If the socket is a member of the temp set, there is stuff to read from the socket.
      if(FD_ISSET(sockfd, &tmp_set)) {
        unsigned int len = sizeof(serveraddr);
        int n = recvfrom(
          sockfd,
          response,
          MAX_INPUT_SIZE,
          0,
          (struct sockaddr *)&serveraddr,
          &len
        );

        if(n <= 0) {
            //Server has potentially closed the connection (check for specific error value)
            printf("Error reading data from server\n");
            close(sockfd);
            free(response);
            return -1;
        }
        ntrim(response);
        printf("%s\n", response);
        memset(response, 0, MAX_INPUT_SIZE);
      }
      else if(FD_ISSET(STDIN_FILENO, &tmp_set))
      {
        //There is something to read from stdin, so read it and send it to the server.
        char r_line[MAX_INPUT_SIZE];
        read(STDIN_FILENO, r_line, MAX_INPUT_SIZE);
        ntrim(r_line);

        // Quit the program if /exit was typed.
        if(!strcmp(r_line, "/exit"))
        {
          //to disconnect, send the ETX character and expect an ACK back from the server
          char lETX = ETX_CHAR;
          if(send_get_ack(&lETX, 1, sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)))
          {
            std::cout << "Recieved an ACK from server; cleanly shutting down." << std::endl;
            free(response);
            close(sockfd);
            return 0;
          }
          else
          {
            //did not get an ACK back, so the server must be down
            std::cerr << "Server unreachable on disconnect.\n";
            free(response);
            close(sockfd);
            return -1;
          }
        }
        else
        {
          if(!send_get_ack(r_line, strlen(r_line), sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)))
          {
            std::cerr << "Server did not ACK packet--assuming it is down.\n";
            /*free(response);
            close(sockfd);
            return -1;*/
          }
        }
      }
      else
      {
        std::cout << "No file descriptor ready for reading..." << std::endl;
      }
    }
    close(sockfd);
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

bool send_get_ack(
  char * aBytes,
  size_t aLen,
  int aSocketFD,
  struct sockaddr * aDestAddr,
  socklen_t aDestAddrLen)
{
  //With UDP, whatever is in the send buffer is explicitly sent in a packet
  if(-1 == sendto(
                  aSocketFD,
                  aBytes,
                  aLen,
                  0,
                  aDestAddr,
                  aDestAddrLen
                  )
    )
  {
    perror(strerror(errno));
    return false;
  }

  char * lResponse = (char *)malloc(MAX_INPUT_SIZE);

  int n = recvfrom(
    aSocketFD,
    lResponse,
    MAX_INPUT_SIZE,
    0,
    (struct sockaddr *)&aDestAddrLen,
    &aDestAddrLen
  );

  if(n < 0)
  {
    perror(strerror(errno));
    free(lResponse);
    return false;
  }
  else if(lResponse[0] == ACK_CHAR) //Check if ACK char.
  {
    free(lResponse);
    return true;
  }
  else
  {
    std::cout << "DEBUG: Raw response (should be just the ACK char): " << std::string(lResponse) << std::endl;
    free(lResponse);
    return false;
  }

  //satisfy the compiler--this should never be executed.
  return false;
}
