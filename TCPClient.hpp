#include<sys/socket.h>
#include "netdb.h"
#include <unistd.h>
#include <iostream>
#ifndef TCPClient_H
#define TCPClient_H
int client_init(std::string host,std::string port);
#endif