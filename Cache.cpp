#include "Cache.hpp"
#include <iostream>
#include "ServerResponse.hpp"
#include "ClientRequest.hpp"
#include <ctime>
#include <sstream>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include "netdb.h"
#include <algorithm>
#include "Time.hpp"
#include <ctime>
#include<fstream> 
#include <pthread.h>
bool Cache::in_cache(std::string request){
	return this->cache_map.find(request)!=this->cache_map.end();
}
bool Cache::validate(std::ofstream& LogStream,pthread_mutex_t lock,int server_fd,ClientRequest* request, ServerResponse* response){
	//need implementation
	std::map<std::string,std::string> info_map=response->response_info;
	bool hasET=info_map.find("ETag")!=info_map.end();
	bool hasModified=info_map.find("Last-Modified")!=info_map.end();
	if(hasET==false&&hasModified==false){
		return true;
	}
	std::string prev_request=request->whole_request;
	std::string new_request="";
	if(hasET){
		std::stringstream ss;
		ss<<prev_request;
		std::string line;
		while(std::getline(ss,line)){
			if(!line.empty()){
				new_request+=line+"\r\n";
			}else{
				new_request+="If-None-Match: ";
				new_request+=info_map.find("ETag")->second;
				new_request+="\r\n";
			}
		}
	}
	if(hasModified){
		new_request.insert(new_request.length()-2,"If-Modified-Since: "+info_map.find("Last-Modified")->second+"\r\n");
	}
	int status=send(server_fd,new_request.c_str(),new_request.length()+1,0);
	if(status==-1){
		std::cerr<<"send error!!!"<<std::endl;
	}
	  char response_char[2147483647];
      int len=recv(server_fd,response_char,sizeof(response_char),0);
      std::string new_response=std::string(response_char,len);
      bool need_val=new_response.find("200")!=std::string::npos;
      if(need_val){
      	pthread_mutex_lock(&lock);
      	LogStream<<request->ID<<": in cache, requires validation"<<std::endl;
      	pthread_mutex_unlock(&lock);
      	return false;
	  }
	return true;
}
bool Cache::check_expire(std::ofstream&LogStream,pthread_mutex_t lock,int id,std::string info,ServerResponse* response){
	std::map<std::string, std::string> info_map=response->response_info;
	bool has_expire=(info_map.find("Expires")!=info_map.end());
    //if expired log info, return true, else, return false;
    if(has_expire==false){
    	return false;
	}
	std::string expire_string=info_map.find("Expires")->second;
	Time* eTime=new Time(expire_string);
	bool noExpire=eTime->before(std::time(0));
	if(noExpire){
		return true;
	}
	pthread_mutex_lock(&lock);
	LogStream<<id<<info<<expire_string<<std::endl;
	pthread_mutex_unlock(&lock);
    return false;
}
std::vector<std::string> Cache::get_cache_control(ServerResponse* response){
		std::map<std::string, std::string> info_map=response->response_info;
		bool has_cache_control=(info_map.find("Cache-Control")!=info_map.end());
		std::vector<std::string> cache_controls;
		std::string controls=info_map.find("Cache-Control")->second;
		char* cache_str;
		cache_str=strtok((char*)controls.c_str(),",");
		while(cache_str!=NULL){
			cache_controls.push_back(std::string(cache_str));
			cache_str=strtok(NULL,",");
		}
	     return cache_controls;
}
ServerResponse* Cache::check_request_save(std::ofstream&LogStream,pthread_mutex_t lock,int server_fd,ClientRequest* request){
	int req_id=request->ID;
	std::string request_line=request->line_one;
	if(in_cache(request_line)==false){
		pthread_mutex_lock(&lock);
		LogStream<<req_id<<":"<<" not in cache"<<std::endl;
		pthread_mutex_lock(&lock);
		return NULL;
	}
	std::string cached_response=cache_map.find(request_line)->second;
	ServerResponse* response=new ServerResponse(cached_response);
    bool has_expire=check_expire(LogStream,lock,req_id,": in cache, but expired at",response);
    std::vector<std::string> controls=get_cache_control(response);
    if(std::find(controls.begin(),controls.end(),std::string("no-cache"))!=controls.end()){
        if(validate(LogStream,lock,server_fd,request,response)){
        	pthread_mutex_lock(&lock);
        	LogStream<<req_id<<": valid"<<std::endl;
        	pthread_mutex_unlock(&lock);
        	return response;
		}
	}
	if(validate(LogStream,lock,server_fd,request,response)){
		    pthread_mutex_lock(&lock);
        	LogStream<<req_id<<": valid"<<std::endl;
        	pthread_mutex_unlock(&lock);
        	return response;
	}
	return NULL;
}
void Cache::check_response_save(std::ofstream&LogStream,pthread_mutex_t lock,int server_fd,ClientRequest* request, ServerResponse* response ){
	std::vector<std::string> controls=get_cache_control(response);
	std::string request_line=request->line_one;
	int req_id=request->ID;
	if(in_cache(request_line)==false){
		bool no_store=(std::find(controls.begin(),controls.end(),"no-store")!=controls.end());
		if(no_store){
			pthread_mutex_lock(&lock);
			LogStream<<"Not cacheable because no-store"<<std::endl;
			pthread_mutex_unlock(&lock);
			return;
		}
		this->cache_map.insert(this->cache_map.begin(),std::pair<std::string,std::string>(request_line,response->whole_response));
		return;
	}
	std::string cached_response=this->cache_map.find(request_line)->second;
	ServerResponse* cresponse=new ServerResponse(cached_response);
	bool has_expire=check_expire(LogStream, lock,req_id,": cached, expires at",cresponse);
	std::vector<std::string> old_controls=get_cache_control(cresponse);
	bool a=std::find(old_controls.begin(),old_controls.end(),"no-cache")!=old_controls.end();
	bool b=false;
	for(int i=0;i<old_controls.size();i++){
		std::string cache_control=controls[i];
		if(cache_control.find("revalidate")!=std::string::npos){
			b=true;
		}
	}
	if(a||b){
		if(validate(LogStream, lock,server_fd,request,response)==false){
				this->cache_map.insert(this->cache_map.begin(),std::pair<std::string, std::string>(request_line,response->whole_response));
		}
	}else{
		this->cache_map.insert(this->cache_map.begin(),std::pair<std::string, std::string>(request_line,response->whole_response));
	}
}