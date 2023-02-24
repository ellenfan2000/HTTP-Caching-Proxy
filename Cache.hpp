#include <iostream>
#include "ServerResponse.hpp"
#include "ClientRequest.hpp"
#include <vector>
#include<pthread.h>
#include <fstream>
#include <thread>         // std::thread
#include <mutex>  

class Cache{
	public:
      std::map<std::string, std::string> cache_map;
	  Cache(){
	    
	  }
	  bool in_cache(std::string request);
	  bool validate(std::ofstream&LogStream,std::mutex lock,int server_fd, ClientRequest* request,ServerResponse* response);
	  bool can_cache(std::string response);
	  bool check_expire(std::ofstream&LogStream,pthread_mutex_t lock,int id,std::string info,ServerResponse* response);
	  std::vector<std::string> get_cache_control(ServerResponse* response);
	  ServerResponse* check_request_save(std::ofstream&LogStream,pthread_mutex_t lock,int server_fd,ClientRequest* request);
	  void check_response_save(std::ofstream&LogStream, pthread_mutex_t lock,int server_fd,ClientRequest* request, ServerResponse* response);
}; 