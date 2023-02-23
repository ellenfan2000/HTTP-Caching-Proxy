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

tcp::socket * connectToServer_socket (const char * host, const char * port){
    // net::io_context ioc;

    // // These objects perform our I/O
    // tcp::resolver resolver(ioc);
    // beast::tcp_stream * stream = new beast::tcp_stream(ioc);
    // tcp::resolver::query query(host,port);

    // // Look up the domain name
    // auto const results = resolver.resolve(query);

    // // Make the connection on the IP address we get from a lookup
    // (*stream).connect(results);


    net::io_context io_context;

    // These objects perform our I/O
    tcp::resolver resolver(io_context);

    tcp::socket * socket = new tcp::socket(io_context);
    // beast::tcp_stream * stream = new beast::tcp_stream(io_context);
    tcp::resolver::query query(host,port);

    // Look up the domain name
    auto const results = resolver.resolve(query);

    // Make the connection on the IP address we get from a lookup
    net::connect(*socket, results);
    // (*socket).connect(results);


    return socket;
};

 // if (socket_server->available() > 0){
            //     std::cout << "num is" << num <<std::endl;
            //     beast::http::read(*socket_server, buffer2, response);
            //     http::write(*socket, response);
            // }
            // if (socket->available() > 0){
            //     std::cout << "num is" << num <<std::endl;
            //     beast::http::read(*socket, buffer2, request2);
            //     http::write(*socket_server, request2);
            // }