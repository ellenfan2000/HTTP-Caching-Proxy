
#include<string>
#ifndef ClientRequest_hpp
#define ClientRequest_hpp
class ClientRequest{
public:
  std::string request_method;
  std::string host_name;
  std::string port;
  std::string line_one; 
  int ID;
  std::string IP;
public:
 ClientRequest(std::string request_info,int ID,std::string IP){
 	this->ID=ID;
  this->IP=IP;
 	int request_line_end=request_info.find_first_of("\r\n");
 	std::string first_line=request_info.substr(0,request_line_end);
 	parseFirstLine(first_line);
 	parseHeader(request_info.substr(request_line_end+2,request_info.length()-first_line.length()-2));
 }
 void parseFirstLine(std::string firstLine);
 void parseHeader(std::string header);
};
#endif
