#include<sys/socket.h>
#include "netdb.h"
#include <unistd.h>
#include <iostream>
#include "TCPClient.hpp"
#include "TCPServer.hpp"
#include "ClientRequest.hpp"
#include "ServerResponse.hpp"
#include <pthread.h>
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void*request_process(void* fds);
void post(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd);
void get(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd);
void connect(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd);
void start(){
	int socket_fd=server_init(std::string("12345"));
	while(1){
	   	 int client_fd=server_client_communicate(socket_fd);
	   	 pthread_t thread;
	   	 int*fds=new int[2];
	   	 fds[0]=socket_fd;
	   	 fds[1]=client_fd; 
	   	 pthread_create(&thread,NULL,&request_process,fds);
	}
}
void* request_process(void* fds){
	    int client_fd=((int*)fds)[1];
	    int server_fd=((int*)fds)[0];
		char msg[2147483647];
		int length=recv(client_fd,msg,sizeof(msg),0);
		std::string request_info=std::string(msg,length);
		 pthread_mutex_lock(&lock);
		ClientRequest* request=new ClientRequest(request_info);
		std::string method=request->request_method;
		bool is_bad_request=(method!="GET")&&(method!="POST")&&(method!="CONNECT");
		if(is_bad_request){
			std::cerr<<"Bad Request Type!!!"<<std::endl;
			exit(1);
		}
	    if(method=="POST"){
	    	post(length,request_info,request,server_fd,client_fd);
		}else if(method=="GET"){
			get(length,request_info,request,server_fd,client_fd);
		}else if(method=="CONNECT"){
			connect(length,request_info,request,server_fd,client_fd);
		}
		 pthread_mutex_unlock(&lock);
   return NULL;		
}
ServerResponse* forward(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd){
	int status_1=send(server_fd,request_info.c_str(),length,0);
  if(status_1==-1){
  	std::cerr<<"send error!!"<<std::endl;
  	exit(1);
  }
  char response_char[2147483647];
  int len=recv(server_fd,response_char,sizeof(response_char),0);
  std::string Response_Info=std::string(response_char,len);
  ServerResponse* response=new ServerResponse(Response_Info);
  int status_2= send(client_fd,Response_Info.c_str(),len,0);
  if(status_2==-1){
  	std::cerr<<"send error!!!"<<std::endl;
  	exit(1);
  }
  return response;
}
void post(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd){
  ServerResponse* response=forward(length,request_info,request,server_fd,client_fd);
}
void get(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd){
   ServerResponse* response=forward(length,request_info,request,server_fd,client_fd);
}
void connect(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd){
	 ServerResponse* response=forward(length,request_info,request,server_fd,client_fd);
}
int main(){
	start();
	return 0;
}
