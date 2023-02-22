#include<sys/socket.h>
#include "netdb.h"
#include <unistd.h>
#include <iostream>
#include "TCPClient.hpp"
#include "TCPServer.hpp"
#include "ClientRequest.hpp"
#include "ServerResponse.hpp"
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void*request_process(void* fds);
void post(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd);
void get(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd);
void connect(int length,std::string request_info,ClientRequest* request,int server_fd, int client_fd);
void start(){
	int socket_fd=server_init(std::string("12345"));
	int req_id=0;
	while(1){
		 struct sockaddr_storage their_addr;
	     socklen_t addr_size;
	     int client_fd=accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
	     std::string client_IP=inet_ntoa(((struct sockaddr_in*)&their_addr)->sin_addr);
	   	 pthread_t thread;
	   	 std::stringstream ss1;
         ss1 << client_fd;
         std::string client_fd_str = ss1.str();
         std::stringstream ss2;
          ss2 << req_id;
         std::string req_id_str = ss2.str();
	   	 std::string*fds=new std::string[3];
	   	 fds[0]=client_fd_str; 
	   	 fds[1]=req_id_str;
	   	 fds[2]=client_IP;
	   	 pthread_create(&thread,NULL,&request_process,fds);
	   	 
	   	 req_id+=1;
	}
}
void* request_process(void* fds){
	    std::string client_fd_str=((std::string*)fds)[0];
	    int client_fd=atoi(client_fd_str.c_str());
	    std::string req_id_str=((std::string*)fds)[1];
	    int req_id=atoi(req_id_str.c_str());
		char msg[2147483647];
		int length=recv(client_fd,msg,sizeof(msg),0);
		std::string request_info=std::string(msg,length);
		 //pthread_mutex_lock(&lock);
		ClientRequest* request=new ClientRequest(request_info,req_id,((std::string*)fds)[2]);
		std::string method=request->request_method;
        std::string host=request->host_name;
        std::string port=request->port;
        int server_fd=client_init(host,port);
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
		 //pthread_mutex_unlock(&lock);
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
