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
while(std::getline(ss,line)){
	if(line.length()>0){
		sum+=line;
	}else{
		if(cnt==0){
			parseHead(sum);
		}else if(cnt==1){
			parseMiddle(sum);
		}else if(cnt>=2&&sum.length()>0){
			parseBody(sum);
		}
		cnt+=1;
		sum="";
	}
}	
}
void ServerResponse::parseHead(std::string head){
int flag=0;
std::string sum="";
int cnt=0;
for(int i=0;i<head.length();i++){
	std::string sub=head.substr(i,1);
	if(sub!=" "){
		sum+=sub;
		flag=0;
	}else{
		if(cnt==0){
			this->http_version=sum;
		}else if(flag==0){
			this->status_info=sum;
			this->is_200_ok=(sum.find("200 OK")!=std::string::npos);
		}
		cnt+=1;
		flag=1;
		sum="";
	}
}
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
				std::string value=line.substr(index+1,line.length()-index);
				this->response_info.insert(this->response_info.begin(),std::pair<std::string,std::string>(key,value)); 
			}
		}
	}
}
void ServerResponse::parseBody(std::string body){
	this->response_body=body;
}
