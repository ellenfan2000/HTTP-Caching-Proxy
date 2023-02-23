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
    // std::string request_line;
    // std::string response_info;
public:
	// Cache(std::string request_line, std::string response_info){
	// 	this->request_line=request_line;
	// 	this->response_info=response_info;
	// }
    Cache(){}
	bool isInCache(http::request<http::dynamic_body> * request){
        return false;
    }
	bool validate(http::request<http::dynamic_body> * request);
	http::response<http::dynamic_body> * getCache(http::request<http::dynamic_body> * request);
	bool canCache(http::request<http::dynamic_body> * request);
	std::string trySave();
}; 