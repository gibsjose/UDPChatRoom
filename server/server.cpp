//Example server application

//This server will handle ALL clients that connect to it
//This is done by using select(), which checks if a file descriptor actually
//has data to read on it before blocking on the socket

#include <sys/socket.h> //Socket features
#include <sys/types.h>
#include <netinet/in.h> //Internet-specific features of sockets
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <map>


#define PORT 8888   //Default server port
#define LEN 1024    //Arbitrary string length

void getAddressAndPort(struct sockaddr_in *s, char *addr, size_t addr_size, uint16_t *port);

int main(int argc, char *argv[]) {
    std::map<std::pair<unsigned short int, char * >,sockaddr_in> map;

    //Used for ack to the client
    char ack = 6;

    //Open socket
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

    //Handle connections
    while(1) {
      unsigned int len = sizeof(clientaddr);

      //Receive on the client socket
      char r_line[1024];
      memset(&r_line, 0, 1024);

      memset(r_line, 0, 1024);

      unsigned int n = recvfrom(sockfd, r_line, 5000, 0, (struct sockaddr *)&clientaddr, &len);

      //Check to see if the message recieved is the ascii STX and send back
      //printf("Received %d bytes from client: %s\n", n, r_line);

      if(r_line[0] == 2) {
        sendto(sockfd, &ack, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        //sendto(sockfd, r_line, strlen(r_line), 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        //Need to convert this to standard IP
        char * ip;
        ip = inet_ntoa(clientaddr.sin_addr);

        map[std::pair<unsigned short int, char *>(clientaddr.sin_port, ip)] = clientaddr;
        //map.insert(std::pair<std::pair<unsigned short int, char *>, sockaddr_in>(new std::pair<unsigned short int, char *>(clientaddr.sin_port, ip), clientaddr);

        printf("<---> New client connected (%s)\n\n", ip);
      }
      else if(r_line[0] == 3) {
        //Checking for ascii ETX to finish connection
        char * ip;
        ip = inet_ntoa(clientaddr.sin_addr);
        map.erase(std::pair<unsigned short int, char *>(clientaddr.sin_port, ip));
        sendto(sockfd, &ack, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        printf("Sent ack to confirm shut down from %s", ip);
        std::cout << std::endl;
      }
      else {
        sendto(sockfd, &ack, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        //Display data received
        printf("Received %d bytes from client: %s\n", n, r_line);

        for(auto a = map.begin(); a != map.end(); ++a) {
          if(a->first.first == clientaddr.sin_port) {

          }
          //printf("Going through loops");
          else {
            sendto(sockfd, r_line, strlen(r_line), 0, (struct sockaddr *)&a->second, sizeof(a->second));
          }
        }
      }
      for(auto a = map.begin(); a!= map.end(); ++a) {
        //printf("These are the ports in the map %hu", a->first);
      }
    }
}
