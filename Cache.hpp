class Cache{
    std::map<std::string, std::string> cache_map;
    std::string request_line;
    std::string response_info;
	Cache(std::string request_line, std::string response_info){
		this->request_line=request_line;
		this->response_info=response_info;
	}
	bool in_cache(std::string request);
	bool validate(std::string request);
	std::string get_cache(std::string request);
	bool can_cache(std::string response);
	std::string  try_save();
}; 
