#include "ServerResponse.hpp"
#include <iostream>
#include<sstream>
#include<string>
void ServerResponse::parse(std::string response_parse){
std::stringstream ss;
ss<<response_parse;
std::string line;
std::string sum="";
int cnt=0;
int blank_flag=0;
while(std::getline(ss,line)){
  if(cnt==0&&line.length()>0){
  	parseHead(line);
  }else if(cnt>0&&blank_flag==0){
  	sum+=line+"\r\n";
  }else if(line.empty()){
  	parseMiddle(sum);
  	sum="";
  	blank_flag==1;
  }else if(blank_flag==1&&line.length()>0){
  	sum+=line+"\r\n";
  }
  cnt+=1;
}
parseBody(sum);
}
void ServerResponse::parseHead(std::string head){
this->first_line=head;
int first_space=head.find_first_of(" ");
int end_line=head.find_first_of("\r\n");
this->http_version=head.substr(0,first_space);
std::string left=head.substr(first_space,end_line-first_space);
int pos=left.find_first_not_of(" ");
this->status_info=left.substr(pos,left.length()-pos);
this->is_200_ok=this->status_info.find("200 OK")!=std::string::npos;

}
void ServerResponse::parseMiddle(std::string middle){
	std::stringstream ss;
	ss<<middle;
	std::string line;
	while(std::getline(ss,line)){
		if(line!=""){
			if(line.find(":")!=std::string::npos){
				int index=line.find(":");
				std::string key=line.substr(0,index);
				std::string value=line.substr(index+1,line.length()-index-1);
				this->response_info.insert(this->response_info.begin(),std::pair<std::string,std::string>(key,value)); 
			}
		}
	}
}
void ServerResponse::parseBody(std::string body){
	this->response_body=body;
}
