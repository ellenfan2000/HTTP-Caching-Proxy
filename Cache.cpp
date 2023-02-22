#include "Cache.hpp"
#include <iostream>
bool Cache::in_cache(std::string request){
	return this->cache_map.find(request)!=this.cache_map.end();
}
bool Cache::validate(std::string request){
	return true;
}
std::string get_cache(std::string request){
	return "";
}
bool Cache::can_cache(std::string response){
	return true;
}
std::string Cache::trysave(){
	if(in_cache(this->request_line)&&validate(this->request_line)){
		return (this->cache_map.find(this->request_line))->second;
	}else if(in_cache(this->request_line)&&!validate(this->request_line)){
		return "ID: in cache, but expired at EXPIREDTIME";
	}else if(!in_cache(this->request_line)){
		this->cache_map.insert(new pair<std::string,std::string>(this->request_line,this->response_info));
		return "not in cache";
	}
}
