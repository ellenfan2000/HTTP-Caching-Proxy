#include <map>
#include <string>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include "SocketUtils.hpp"

class Cache{
private:
    std::map<http::request<http::dynamic_body>, http::response<http::dynamic_body> > cache_map;
public:
    Cache(){}
	bool isInCache(http::request<http::dynamic_body> * request){
        return false;
    }
	bool validate(http::request<http::dynamic_body> * request);
	http::response<http::dynamic_body> * getCache(http::request<http::dynamic_body> * request);
	bool canCache(http::request<http::dynamic_body> * request);
	bool checkExpire();
	std::vector<std::string> getCacheControl();
	std::string save();

	// bool in_cache(std::string request);
	//   bool validate(std::ofstream&LogStream,std::mutex lock,int server_fd, ClientRequest* request,ServerResponse* response);
	//   bool can_cache(std::string response);
	//   bool check_expire(std::ofstream&LogStream,pthread_mutex_t lock,int id,std::string info,ServerResponse* response);
	//   std::vector<std::string> get_cache_control(ServerResponse* response);
	//   ServerResponse* check_request_save(std::ofstream&LogStream,pthread_mutex_t lock,int server_fd,ClientRequest* request);
	//   void check_response_save(std::ofstream&LogStream, pthread_mutex_t lock,int server_fd,ClientRequest* request, ServerResponse* response);
}; 