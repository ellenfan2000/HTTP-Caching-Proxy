#include<stdio.h> 
#include<stdlib.h>
#include<string>
#ifndef ClientRequest_hpp
#define ClientRequest_hpp
class ClientRequest{
std::string request_method;
std::string request_url;
std::string http_version;
std::string host_name;
std::string port;
public:
 ClientRequest(std::string request_info){
 }
};
#endif