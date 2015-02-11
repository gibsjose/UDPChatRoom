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
#include "../defines.hpp"


#define PORT 8888   //Default server port

void getAddressAndPort(struct sockaddr_in *s, char *addr, size_t addr_size, uint16_t *port);

int main(int argc, char *argv[]) {
  std::map<std::pair<unsigned short int, char * >,sockaddr_in> map;
  char ack = 6;

  //Open socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  //Create server/client address structures:
  //Server needs to know both the address of itself and the client
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(PORT);
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
    if(r_line[0] == STX_CHAR) {
      sendto(sockfd, &ack, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
      char * ip;
      ip = inet_ntoa(clientaddr.sin_addr);
      //Add new client to map
      map[std::pair<unsigned short int, char *>(clientaddr.sin_port, ip)] = clientaddr;

      std::cout << "<---> New client connected from " << ip << ":" << clientaddr.sin_port << std::endl;
    }
    //Checking for ascii ETX to remove client
    else if(r_line[0] == ETX_CHAR) {
      char * ip;
      ip = inet_ntoa(clientaddr.sin_addr);
      //remove client from map
      map.erase(std::pair<unsigned short int, char *>(clientaddr.sin_port, ip));
      sendto(sockfd, &ack, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
      std::cout << "Sent ack to confirm shut down from " << ip << ":" << clientaddr.sin_port << std::endl;
    }
    else {
      //Display data received
      std::cout << "Received " << n << " bytes from client: " << r_line << std::endl;
      for(auto a = map.begin(); a != map.end(); ++a) {
        //send ack to client to confirm message was recieved
        if(a->first.first == clientaddr.sin_port && a->first.second == inet_ntoa(clientaddr.sin_addr)) {
          sendto(sockfd, &ack, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
        }
        //send the message to everyone but the person who sent it
        else {
          sendto(sockfd, r_line, strlen(r_line), 0, (struct sockaddr *)&a->second, sizeof(a->second));
        }
      }
    }
  }
}
