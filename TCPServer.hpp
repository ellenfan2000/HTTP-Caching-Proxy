#include <iostream> 
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h> 
#ifndef TCPServer_H
#define TCPServer_H
int server_init(std::string port);
int server_client_communicate(int sockfd);
#endif