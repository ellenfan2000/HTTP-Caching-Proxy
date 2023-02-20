#include <iostream> 
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h> 
#include "TCPServer.hpp"
int server_init(std::string port){
   int status;
   struct addrinfo server_info;
   struct addrinfo*server_info_list;
   memset(&server_info,0,sizeof(server_info));
   server_info.ai_family = AF_UNSPEC;
   server_info.ai_socktype = SOCK_STREAM;
   server_info.ai_flags = AI_PASSIVE;
   if ((status = getaddrinfo(NULL, port.c_str(), &server_info, &server_info_list)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
   }
   int server_fd=socket(server_info_list->ai_family,server_info_list->ai_socktype,server_info_list->ai_protocol);
   int one;
   status=setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(int));
   if(status<0){
   	 perror("setting socket error!!");
   	 exit(1);
   }
   bind(server_fd,server_info_list->ai_addr,server_info_list->ai_addrlen);
   int listen_yes=listen(server_fd,200);
   if(listen_yes==-1){
   	 perror("listen error!!");
   	 exit(1);
   }
   freeaddrinfo(server_info_list);
   return server_fd;
}
int server_client_communicate(int sockfd){
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int client_socket_fd=accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
	return client_socket_fd;
}
