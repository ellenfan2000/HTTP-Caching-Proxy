#include<sys/socket.h>
#include "netdb.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
int client_init(std::string host, std::string port){
	int status;
  int socket_fd;
  struct addrinfo host_addrinfo;
  struct addrinfo *host_addrinfo_list;
  memset(&host_addrinfo, 0, sizeof(host_addrinfo));
  host_addrinfo.ai_family   = AF_UNSPEC;
  host_addrinfo.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(host.c_str(), port.c_str(), &host_addrinfo, &host_addrinfo_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << host << "," << port << ")" << std::endl;
    return -1;
  } //if

  socket_fd = socket(host_addrinfo_list->ai_family, 
		     host_addrinfo_list->ai_socktype, 
		     host_addrinfo_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << host << "," << port << ")" << std::endl;
    return -1;
  } //if
  
  std::cout << "Connecting to " << host << " on port " << port << "..." << std::endl;
  
  status = connect(socket_fd, host_addrinfo_list->ai_addr, host_addrinfo_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << host << "," << port << ")" << std::endl;
    return -1;
  } //if
  freeaddrinfo(host_addrinfo_list);

  return socket_fd;
}
int main(){
	return 0;
}
