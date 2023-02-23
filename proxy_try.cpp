#include "Cache_try.hpp"
#include <boost/bind/bind.hpp>
// #include <boost/asio/ssl.hpp>
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
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

class Proxy{
private:
    const char * host;
    const char * port;
    boost::asio::io_context io_context;
    Cache c;
public:
    Proxy(std::string p):host(NULL), port(p.c_str()){}

    void run(){
        // boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), strtol(port, NULL, 0)));
        while (true){
            tcp::socket * socket = new tcp::socket(io_context);
            // pthread_t thread;
            acceptor.accept(*socket);
            std::thread t(&Proxy::requestProcess, this, socket);
            t.detach();
        }
    }
    
    tcp::socket * connectToServer_socket (const char * host, const char * port){

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

    void requestProcess(tcp::socket * socket){
        beast::flat_buffer buffer;
        http::request<http::dynamic_body> request;
        http::read(*socket, buffer, request);

        // if request is not a valid type, quit
        http::verb method = request.method();
        std::cout << "Received HTTP request: " << request << std::endl;
        if (method ==http::verb::get){
            get(&request, socket);
		}
        else if(method == http::verb::post){
            post(&request, socket);

        }else if(method == http::verb::connect){
            connect(&request,socket);
        }else{
            delete socket;
            std::cerr<<"Bad Request Type!!!"<<std::endl;
			exit(1);
        }
        socket->close();
        delete socket;
    }

    bool isHTTPS(std::string hostname, std::string * host, std::string * port){
        size_t f = hostname.find(":");
        if(f != std::string::npos){
            *host = hostname.substr(0, f);
            *port = hostname.substr(f+1);
            return true;
        }
        *host = hostname;
        *port = "80";
        return false;
    }

    void post(http::request<http::dynamic_body> * request, tcp::socket * socket){
        // Send the HTTP request to requested server HTTP not HTTPS
        // beast::tcp_stream * stream = connectToServer(std::string(request->at("HOST")).c_str(), "80");
        // http::write(*stream, *request);

        // //recieve the HTTP response from the server
        // boost::beast::flat_buffer buffer2;
        // http::response<http::dynamic_body> response;
        // boost::beast::http::read(*stream, buffer2, response);

        // // Send the response to the client
        // http::write(*socket, response);
        std::string port;
        std::string host;
        isHTTPS(std::string(request->at("HOST")), &host, &port);
        tcp::socket * socket_server = connectToServer_socket(host.c_str(), port.c_str());
        http::write(*socket_server, *request);

        //recieve the HTTP response from the server
        boost::beast::flat_buffer buffer2;
        http::response<http::dynamic_body> response;
        
        boost::beast::http::read(*socket_server, buffer2, response);
        // std::cout << "reponse header is " << response.base()<<std::endl;
        // // std::cout << "reponse method is " << response.at()<<std::endl;
        // std::cout << "reponse status is "<<response.result_int()<<std::endl;

        // Send the response to the client
        http::write(*socket, response);
        socket_server->close();
    }
    void get(http::request<http::dynamic_body> * request, tcp::socket * socket){
        post(request, socket);
        // http::response<http::dynamic_body> * response;
        //  if(c.isInCache(request)){
        //     //get response from cache
        //     response = c.getCache(request);

        //     // Send response to the client
        //     http::write(*socket, *response);
        // }else{
        //     post(request, socket);
        // }
    }

    void connect(http::request<http::dynamic_body> * request, tcp::socket * socket){
        // post(request, socket);
        std::string port;
        std::string host;
    
        isHTTPS(std::string(request->at("HOST")), &host, &port);
        tcp::socket * socket_server = connectToServer_socket(host.c_str(), port.c_str());

        http::write(*socket_server, *request);

        std::string message = "HTTP/1.1 200 OK\r\n\r\n";
        net::write(*socket, net::buffer(message));

        // net::io_context io_context;
        // tcp::resolver resolver(io_context);
        // auto endpoints = resolver.resolve(host, port);

        // boost::asio::io_context my_context;
        // boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

        // net::ssl::stream<tcp::socket> sockssl(my_context, ctx);
        // net::connect(sockssl.lowest_layer(),endpoints);
        // // ctx.set_default_verify_paths();
        
        // http::write(sockssl.next_layer(),*request);
        // beast::flat_buffer buffer;
        // http::response<http::dynamic_body> response;


        // http::read(sockssl, buffer, response);
        // std::cout << "reponse status is "<<response.result_int()<<std::endl;
        // if(response.result_int() == 200){
        //     std::cout << "reponse header is " << response.base()<<std::endl;
        // }



        // beast::flat_buffer buffer;
        // http::response<http::dynamic_body> response;

        // http::read(*socket_server, buffer, response);
        // std::cout << "reponse status is "<<response.result_int()<<std::endl;
        // if(response.result_int() == 200){
        //     std::cout << "reponse header is " << response.base()<<std::endl;
        // }


        int num = 0;
        std::cout<<"hostname is " <<host <<" portnumber is " <<port <<std::endl;
        while(true){
            beast::flat_buffer buffer;
            http::response<http::dynamic_body> response;
            http::request<http::dynamic_body> request2;
            std::cout<<num<<std::endl;
            socket_server->async_wait(tcp::socket::wait_read, boost::bind(wait_handler, boost::asio::placeholders::error,socket, socket_server,1));
            socket->async_wait(tcp::socket::wait_read,boost::bind(wait_handler, boost::asio::placeholders::error,socket, socket_server,-1));
            if(!socket->is_open() || !socket_server->is_open()){
                break;
            }
            num++;
        }
        socket_server->close();
    }

    static void wait_handler(const boost::system::error_code & ec, tcp::socket * socketClient, tcp::socket * socketServer, int flag){
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> response;
        http::request<http::dynamic_body> request;
        std::cout<<"?" <<std::endl;
        if(flag == 1){
            beast::http::read(*socketServer, buffer, response);
            http::write(*socketClient, response);
        }
        else{
            beast::http::read(*socketClient, buffer, request);
            http::write(*socketServer, request);
        }
    }
};

int main(){
    std::string host = "12345";
    Proxy p(host);
    p.run();
}