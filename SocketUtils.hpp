#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <ctime>          // std::tm

#include <sstream>
#include <locale>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;      
namespace pt = boost::posix_time;
namespace dt = boost::date_time;

time_t parseDatetime(std::string date_str){
    // std::locale loc(std::cout.getloc(), new boost::posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S %Z"));
    // std::istringstream ss(date_str);
    // ss.imbue(loc);
    // pt::ptime date_time;
    // ss >> date_time;

    std::string format_str = "%a, %d %b %Y %H:%M:%S GMT";

    tm tm;
    tm.tm_isdst = 0;
    if (strptime(date_str.c_str(), format_str.c_str(), &tm) == nullptr) {
        std::cerr << "Failed to parse HTTP-date string" << std::endl;
    }
    time_t time = mktime(&tm);

    // Print the parsed time
    // std::cout<<"String is: "<<date_str<<std::endl;
    // std::cout << "Parsed time: " << std::ctime(&time);
    return time;
}

std::vector<std::string> split(std::string str_, char delimiter){
    std::string str = "";
    for(int i = 0; i < str_.size(); i++){
        if(str_[i]!=' '){
            str+=str_[i];
        }
    }

    std::vector<std::string> result;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start));
    return result;
}

std::map<std::string, long> parseFields(std::string & str){
    std::map<std::string, long> result;
    std::vector<std::string> fields = split(str, ',');
    int end;
    for(int i  = 0; i < fields.size(); i++){
        end = 0;
        if((end= fields[i].find('=', 0)) != std::string::npos){
            result[fields[i].substr(0, end)] = std::stol(fields[i].substr(end+1),NULL,10);
        }else{
            result[fields[i]] = -1;
        }
    }
    return result;
}

std::string parseVersion(unsigned version){
    unsigned major = version / 10;
    unsigned minor = version % 10;
    std::string ans = "HTTP/";
    ans = ans+ std::to_string(major) + "."  +std::to_string(minor); 
    return ans;
}


//  // if (socket_server->available() > 0){
//             //     std::cout << "num is" << num <<std::endl;
//             //     beast::http::read(*socket_server, buffer2, response);
//             //     http::write(*socket, response);
//             // }
//             // if (socket->available() > 0){
//             //     std::cout << "num is" << num <<std::endl;
//             //     beast::http::read(*socket, buffer2, request2);
//             //     http::write(*socket_server, request2);
//             // }

// pthread_mutex_lock(&lock);
// LogStream << ID << ": cached, but requires re-validation"<<std::endl;
// pthread_mutex_unlock(&lock);

 // bool isFresh(http::response<http::dynamic_body> * response, int ID){
    //     time_t now;
    //     time(&now);
    //     time_t gmt_now = mktime(gmtime(&now));
    //     //has cache control: max-age field
    //     if(response->find(http::field::cache_control) != response->end()){
    //         std::string str((*response)[http::field::cache_control]);
    //         std::map<std::string, long> fields = parseFields(str);
	//         std::string date_str((*response)[http::field::date]);
    //         time_t date_value = parseDatetime(date_str);
    //         if(fields.find("max-age") != fields.end()){
    //             double age = difftime(now, date_value);
    //             if(age > fields["max-age"]){
    //                 time_t expire = date_value + fields["max-age"];
    //                 pthread_mutex_lock(&lock);
    //                 LogStream<<ID<<": in cache, but expired at "<<ctime(&expire);
    //                 pthread_mutex_unlock(&lock);
    //                 return false;
    //             }else{
    //                 return true;
    //             }
    //         }
    //     }else{
    //         if(response->find(http::field::expires)== response->end()){
    //             return true;
    //         }else{
    //             std::string expire_str((*response)[http::field::expires]);
    //             time_t expire = parseDatetime(expire_str);
    //             if (expire > now){
    //                 return true;
    //             }else{
    //                 pthread_mutex_lock(&lock);
    //                 LogStream<<ID<<": in cache, but expired at "<<ctime(&expire);
    //                 pthread_mutex_unlock(&lock);
    //                 return false;
    //             }
    //         } 
    //     }
    //     return true;
    // }

    // bool needValidationWhenAccess(http::response<http::dynamic_body> * response, int ID){
    //     if(response->find(http::field::cache_control) == response->end()){
    //         if(isFresh(response, ID)){
    //             return false;
    //         }else{
    //             return true;
    //         }
    //     }else{
    //         if()
    //         std::string str((*response)[http::field::cache_control]);
    //         std::map<std::string, long> fields = parseFields(str);
            
    //         //if no-cache or must-revalidate, need validation
    //         if(fields.find("no-cache") != fields.end() || fields.find("must-revalidate") != fields.end()){
    //             return true;
    //         }else{
    //             if(isFresh(response, ID)){
    //                 return false;
    //             }else{
    //                 return true;
    //             }
    //         }
    //     } 
    // }

    // void CONNECT(http::request<http::dynamic_body> * request,int ID, tcp::socket * socket, tcp::socket * socket_server){
    //     boost::system::error_code ec;
    //     //send success to client, build the tunnel
    //     int status;
    //     std::string message = "HTTP/1.1 200 OK\r\n\r\n";
    //     status = net::write(*socket, net::buffer(message), ec);
    //     if(ec.value() != 0){
    //         std::cerr<< "CONNECT: send the 200 OK to client error: " <<ec.to_string()<< ", "<< ec.message()<<std::endl;
    //     }
    //     while(true){
    //         //if connection is closed, break the loop see error code
    //         //byte of data available to read from server
    //         int server_byte = socket_server->available(ec);
    //         if(ec.value() != 0){
    //             std::cerr<< "CONNECT: data available to read from server error" <<ec.to_string()<<", "<<ec.message()<< std::endl;
    //         }
    //         //byte of data available to read from client
    //         int clinet_byte = socket->available(ec);
    //         if(ec.value() != 0){
    //             std::cerr<< "CONNECT: data available to read from client error" <<ec.to_string()<<", "<<ec.message()<< std::endl;
    //         }

    //         //send message between client and server
    //         if(server_byte > 0){
    //             std::vector<char> bu1(server_byte);
    //             net::read(*socket_server, net::buffer(bu1), ec);
    //             if(ec.value() != 0){
    //                 std::cerr<< "CONNECT: read data from server error" <<ec.to_string()<<", "<<ec.message()<< std::endl;
    //             }
    //             net::write(*socket, net::buffer(bu1),ec);
    //             if(ec.value() != 0){
    //                 std::cerr<< "CONNECT: send data to client error" <<ec.to_string()<<", "<<ec.message()<< std::endl;
    //             }
    //         }
    //         if(clinet_byte > 0){
    //             std::vector<char> bu2(clinet_byte);
    //             net::read(*socket, net::buffer(bu2), ec);
    //             if(ec.value() != 0){
    //                 std::cerr<< "CONNECT: read data from client error" <<ec.to_string()<<", "<<ec.message()<< std::endl;
    //             }
    //             net::write(*socket_server, net::buffer(bu2),ec);
    //             if(ec.value() != 0){
    //                 std::cerr<< "CONNECT: send data to server error" <<ec.to_string()<<", "<<ec.message()<< std::endl;
    //             }
    //         }
    //         //if connection is closed, break the loop
    //         if(!socket_server->is_open()||!socket->is_open()){
    //             pthread_mutex_lock(&lock);
    //             LogStream<<ID<<": Tunnel closed"<<std::endl;
    //             pthread_mutex_unlock(&lock);
    //             break;
    //         }
    //     }
    // }