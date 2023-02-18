
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
 	int request_line_end=request_info.find_first_of("\r\n");
 	std::string first_line=request_info.substr(0,request_line_end);
 	parseFirstLine(first_line);
 	parseHeader(request_info.substr(request_line_end+1,request_info.length()-first_line.length()+2));
 }
 void parseFirstLine(std::string firstLine);
 void parseHeader(std::string header);
};
#endif
