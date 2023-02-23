#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;      

beast::tcp_stream * connectToServer(const char * host, const char * port){
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream * stream = new beast::tcp_stream(ioc);
    tcp::resolver::query query(host,port);

    // Look up the domain name
    auto const results = resolver.resolve(query);

    // Make the connection on the IP address we get from a lookup
    (*stream).connect(results);

    return stream;
};
