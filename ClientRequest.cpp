#include <iostream>
#include "ClientRequest.hpp"
void ClientRequest::parseFirstLine(std::string firstLine){
std::string sum="";
int cnt=0;
for(int i=0;i<firstLine.length();i++){
	if(firstLine.substr(i,1)!=" "){
		sum+=firstLine.substr(i,1);
	}else if(cnt==0){
		this->request_method=sum;
		sum="";
		cnt+=1;
	}else if(cnt==1){
		this->request_url=sum;
		sum="";
		cnt+=1;
	}else if(cnt==2){
		this->http_version=sum;
		sum="";
	}
}
}
void ClientRequest::parseHeader(std::string header){
	int host_pos=header.find("Host:");
	if(host_pos==std::string::npos){
		std::cout<<"No host info!";
		return;
	}
	int host_info_end_pos=header.find_first_of("\r\n");
	std::string host_info=header.substr(host_pos+6,host_info_end_pos-host_pos);
	int before_port=host_info.find_first_of(":");
	if(before_port==std::string::npos){
		this->host_name=host_info.substr(0,before_port);
		this->port="80";
	}else{
		this->host_name=host_info.substr(0,before_port);
		this->port=host_info.substr(before_port+1,host_info.length()-this->host_name.length()-1);
	}
}
