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
#include "SocketUtils.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


class Server{
private:
    char * hostname;
    char * port;
public:
    Server(char * h, char * p){
        hostname = h;
        port = p;
    }
    beast::tcp_stream * buildServer(){
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream * stream = new beast::tcp_stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(hostname, port);

        // Make the connection on the IP address we get from a lookup
        
        (*stream).connect(results);

        return stream;

    }


};

int main()
{
    char name[512];
    gethostname(name,sizeof(name));
    std::cout<<"current host name "<<name<<std::endl;
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
    tcp::socket socket(io_context);
    acceptor.accept(socket);

    // Read the HTTP request
    boost::beast::flat_buffer buffer;
    http::request<http::dynamic_body> request;
    boost::beast::http::read(socket, buffer, request);


    // Print the request
    std::cout << "Received HTTP request: " << request << std::endl;
    std::cout << "Received HTTP request: " << request.at("HOST") << std::endl;

    // Parse the request
    if (request.method() == http::verb::get) {
        std::cout << "GET request" << std::endl;
    } else if (request.method() == http::verb::post) {
        std::cout << "POST request" << std::endl;
    }

    beast::tcp_stream * stream = connectToServer(std::string(request.at("HOST")).c_str(), "80");
    http::write(*stream, request);

    boost::beast::flat_buffer buffer2;
    http::response<http::dynamic_body> response;
    boost::beast::http::read(*stream, buffer2, response);


    // Print the request
    std::cout << "Received HTTP request: " << response << std::endl;
   // std::cout << "Received HTTP request: " << request.at("HOST") << std::endl;




    return 0;
}



