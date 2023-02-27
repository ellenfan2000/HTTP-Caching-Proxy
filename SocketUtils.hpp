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
    std::locale loc(std::cout.getloc(), new boost::posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S %Z"));
    std::istringstream ss(date_str);
    ss.imbue(loc);
    pt::ptime date_time;
    ss >> date_time;

   // Print the datetime object
    // std::cout << "Date: " << to_simple_string(date_time) << std::endl;

    std::string format_str = "%a, %d %b %Y %H:%M:%S %Z";

    tm tm;
    tm.tm_isdst=0;
    if (strptime(date_str.c_str(), format_str.c_str(), &tm) == nullptr) {
        std::cerr << "Failed to parse HTTP-date string" << std::endl;
    }
    time_t time = mktime(&tm);

    // Print the parsed time
    // std::cout << "Parsed time: " << std::asctime(&tm);
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

http::response<http::dynamic_body> make400Response(http::request<http::dynamic_body> * request){
    http::response<http::dynamic_body> response;
    response.result(boost::beast::http::status::bad_request);
    response.version(request->version());
    // response.keep_alive(request->keep_alive());
    response.set(boost::beast::http::field::server, "My Server");
    response.set(boost::beast::http::field::content_type, "text/plain");
    // response.body() = "Bad Request";
    response.prepare_payload();
    return response;
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
