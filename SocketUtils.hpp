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

    // Print the datetime object
    // std::cout << "Date: " << to_simple_string(date_time) << std::endl;

    std::string format_str = "%a, %d %b %Y %H:%M:%S %Z";

    tm tm;
    if (strptime(date_str.c_str(), format_str.c_str(), &tm) == nullptr) {
        std::cerr << "Failed to parse HTTP-date string" << std::endl;
    }
    time_t time = mktime(&tm);
    // time_t now;
    // time(&now);
    // std::tm * gmt = gmtime(&time);

    // Print the parsed time
    std::cout << "Parsed time: " << std::asctime(&tm);
    // std::cout << "Parsed time: " << asctime(gmtime(&now));

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
// beast::tcp_stream * connectToServer(const char * host, const char * port){
//     net::io_context ioc;

//     // These objects perform our I/O
//     tcp::resolver resolver(ioc);
//     beast::tcp_stream * stream = new beast::tcp_stream(ioc);
//     tcp::resolver::query query(host,port);

//     // Look up the domain name
//     auto const results = resolver.resolve(query);

//     // Make the connection on the IP address we get from a lookup
//     (*stream).connect(results);


//     return stream;
// };

// tcp::socket * connectToServer_socket (const char * host, const char * port){
//     // net::io_context ioc;

//     // // These objects perform our I/O
//     // tcp::resolver resolver(ioc);
//     // beast::tcp_stream * stream = new beast::tcp_stream(ioc);
//     // tcp::resolver::query query(host,port);

//     // // Look up the domain name
//     // auto const results = resolver.resolve(query);

//     // // Make the connection on the IP address we get from a lookup
//     // (*stream).connect(results);


//     net::io_context io_context;

//     // These objects perform our I/O
//     tcp::resolver resolver(io_context);

//     tcp::socket * socket = new tcp::socket(io_context);
//     // beast::tcp_stream * stream = new beast::tcp_stream(io_context);
//     tcp::resolver::query query(host,port);

//     // Look up the domain name
//     auto const results = resolver.resolve(query);

//     // Make the connection on the IP address we get from a lookup
//     net::connect(*socket, results);
//     // (*socket).connect(results);


//     return socket;
// };

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