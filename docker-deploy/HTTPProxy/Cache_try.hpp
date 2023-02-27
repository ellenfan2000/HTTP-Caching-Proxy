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

	/**
	 * this method remove the least used item from the object
	*/
	void evict(){
		std::string key = used_list[0];
		cache_map.erase(key);
		used_list.erase(used_list.begin());
		capacity++;
	}

public:
    Cache(int m):capacity(m){}

	/**
	 * whether the key in in the cache
	 * @param key the key of the map
	 * @return true if in cache; false if not
	*/
	bool isInCache(std::string & key){
        return cache_map.find(key) != cache_map.end();
    }

	/**
	 * update one key in cache
	*/
	int update(std::string & key, http::response<http::dynamic_body> response){
		if(isInCache(key)){
			// cache_map[key] = std::pair<http::response<http::dynamic_body>, time_t>(response, t);
			cache_map[key] = response;
			return 1;
		} 
		return 0;
	}
	/**
	 * return the reponse stored in cache, update the LRU list
	 * @param key the key to get
	 * @return NULL if not in cache; reponse stored in cache
	*/
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
		return NULL;
	}
	
	/**
	 * insert a item into the cache
	 * @param key 
	 * @param reponse value
	 * @return 1 if success, 0 if not
	*/
	int put(std::string key, http::response<http::dynamic_body> response){
		//already in cache, do not store
		if(isInCache(key)){
			return 0;
		}
		if(capacity == 0){
			evict();
		}else{
			// cache_map[key] = std::pair<http::response<http::dynamic_body>, time_t>(response, t);
			cache_map[key] = response;
			capacity--;
			used_list.push_back(key);
			return 1;
		}
		return 0; 
	}
}; 
