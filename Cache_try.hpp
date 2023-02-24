#include <map>
#include <string>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include "SocketUtils.hpp"

class Cache{
private:
    std::map<std::string, http::response<http::dynamic_body> > cache_map;
	int capacity;
	std::vector<std::string> used_list; //least recently used item -> most recently used item


	void evict(){
		std::string key = used_list[0];
		cache_map.erase(key);
		used_list.erase(used_list.begin());
		capacity++;
	}

public:
    Cache(int m):capacity(m){}
	bool isInCache(std::string & key){
        return cache_map.find(key) != cache_map.end();
    }


	bool validate(std::string & key);

	http::response<http::dynamic_body> * get(std::string & key){
		if(!isInCache(key)){
			return NULL;
		}
		//update the used_list
		for(int i = 0; i < used_list.size(); i++){
			if(used_list[i].compare(key) == 0){
				if(i!=used_list.size()-1)
				used_list.erase(used_list.begin()+ i);
				used_list.push_back(key);
				return &cache_map[key];
			}else{
				return &cache_map[key];
			}
		}
	}

	
	int put(std::string & key, http::response<http::dynamic_body> response){
		//already in cache, do not store
		if(isInCache(key)){
			return 0;
		}
		if(capacity == 0){
			evict();
		}else{
			cache_map[key] = response;
			capacity--;
			used_list.push_back(key);
		}
	}

	// bool in_cache(std::string request);
	//   bool validate(std::ofstream&LogStream,std::mutex lock,int server_fd, ClientRequest* request,ServerResponse* response);
	//   bool can_cache(std::string response);
	//   bool check_expire(std::ofstream&LogStream,pthread_mutex_t lock,int id,std::string info,ServerResponse* response);
	//   std::vector<std::string> get_cache_control(ServerResponse* response);
	//   ServerResponse* check_request_save(std::ofstream&LogStream,pthread_mutex_t lock,int server_fd,ClientRequest* request);
	//   void check_response_save(std::ofstream&LogStream, pthread_mutex_t lock,int server_fd,ClientRequest* request, ServerResponse* response);
}; 