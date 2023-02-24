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

#include <sstream>
#include <locale>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;      
namespace pt = boost::posix_time;
using namespace boost::posix_time;

bool storeCanCache(http::request<http::dynamic_body> * request, http::response<http::dynamic_body> * response){
    //response is not cacheable
    if(response->result_int() != 200){
        return false;
    }
    //the "no-store" cache directive does not appear in request or response header fields


    //the Authorization header field (see Section 4.2 of [RFC7235]) does not appear in the request, if the cache is shared,
    // unless the response explicitly allows it (see Section 3.2), and

    //the response either:
    //contains an Expires header field (see Section 5.3), or
    
    //no cache control field
    if(response->find(http::field::cache_control) == response->end()){
        return false;
    }
    //contains a max-age response directive (see Section 5.2.2.8), or

    //contains a s-maxage response directive (see Section 5.2.2.9) and the cache is shared, or

    //contains a Cache Control Extension (see Section 5.2.3) that allows it to be cached, or

    //has a status code that is defined as cacheable by default (see Section 4.2.2), or

    //contains a public response directive (see Section 5.2.2.5).
	return false;
}
bool checkExpire(){
    return false;
    
}
ptime getDatetime(std::string date_str){
    std::locale loc(std::cout.getloc(), new boost::posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S %Z"));
    std::istringstream ss(date_str);
    ss.imbue(loc);
    ptime date_time;
    ss >> date_time;

    // Print the datetime object
    // std::cout << "Date: " << to_simple_string(date_time) << std::endl;
    return date_time;
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