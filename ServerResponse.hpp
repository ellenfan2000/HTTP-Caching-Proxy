#include <string>
#include <map>
<<<<<<< HEAD
#ifndef ServerResponse_HPP
#define ServerResponse_HPP
=======
>>>>>>> 5f031b076a8bc561f91aeece89a86f0c9d8472be
class ServerResponse{
	public:
   	bool is_200_ok;
	std::string http_version;
	std::string response_body;
	std::string status_info;
	std::map<std::string,std::string> response_info;
		ServerResponse(std::string response){
			parse(response);
		}
		void parse(std::string response_parse);
	   void parseHead(std::string head);
	   void parseMiddle(std::string middle);
	   void parseBody(std::string body);
}; 
#endif