#include <iostream>
#include "ClientRequest.hpp"
void ClientRequest::parseFirstLine(std::string firstLine){
 this->line_one=firstLine;
 int first_blank_position=firstLine.find_first_of(" ");
 std::string method=firstLine.substr(0,first_blank_position);
 this->request_method=method;
}
void ClientRequest::parseHeader(std::string header){
int host_info_pos=header.find_first_of("Host:");
if(host_info_pos==std::string::npos){
	std::cerr<<"no provided host!!"<<std::endl;
	return ;
}
int rem_start=host_info_pos+5;
std::string left=header.substr(rem_start,header.length()-host_info_pos-5);
int first_rm_line=left.find_first_of("\r\n");
std::string portInfo=left.substr(rem_start,first_rm_line-rem_start);
int seperator=portInfo.find_first_of(":");
this->host_name=portInfo.substr(0,seperator);
this->port=portInfo.substr(seperator+1,portInfo.length()-seperator-1);
}
