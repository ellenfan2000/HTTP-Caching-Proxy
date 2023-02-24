#include "Cache_try.hpp"
#include <boost/bind/bind.hpp>
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
    boost::system::error_code ec;
    Cache c;
public:
    Proxy(std::string p):host(NULL), port(p.c_str()){}

    void run(){
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), strtol(port, NULL, 0)));
        while (true){
            //accept client connection
            tcp::socket * socket = new tcp::socket(io_context);
            acceptor.accept(*socket, ec);
            if(ec){
                std::cerr<< ec.message()<<std::endl;
            }
            std::thread t(&Proxy::requestProcess, this, socket);
            t.detach();
        }
    }
    
    /**
     * make connection from proxy to server
     * @param host server hostname
     * @param port host port
     * @return tcp socket of the connection
    */
    tcp::socket * connectToServer_socket (const char * host, const char * port){
        // These objects perform our I/O
        tcp::resolver resolver(io_context);

        tcp::socket * socket = new tcp::socket(io_context);
        // beast::tcp_stream * stream = new beast::tcp_stream(io_context);
        tcp::resolver::query query(host,port);

        // Look up the domain name
        auto const results = resolver.resolve(query,ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        // Make the connection on the IP address we get from a lookup
        net::connect(*socket, results, ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        // (*socket).connect(results);
    return socket;
};

    /**
     * process incomming request from client:
     * POST
     * GET
     * CONNECT
     * @param socket client connection
    */
    void requestProcess(tcp::socket * socket){
        //read request from client
        beast::flat_buffer buffer;
        http::request<http::dynamic_body> request;
        http::read(*socket, buffer, request, ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        
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
            // if request is not a valid type, quit
            //should response 502
            delete socket;
            std::cerr<<"Bad Request Type!!!"<<std::endl;
        }
        socket->close();
        delete socket;
    }

    /**
     * see if a request is http or https, and get their host name and port
     * @param hostname host field of a request
     * @param host the return host name
     * @param port return the port number from hostname
    */
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

    /**
     * process post method, send request to server
     * read reponse from server and send it to client
     * @param request request get from client
     * @param socket connection to client
    */
    void post(http::request<http::dynamic_body> * request, tcp::socket * socket){
        std::string port;
        std::string host;
        isHTTPS(std::string(request->at("HOST")), &host, &port);
        tcp::socket * socket_server = connectToServer_socket(host.c_str(), port.c_str());
        http::write(*socket_server, *request,ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }

        //recieve the HTTP response from the server
        boost::beast::flat_buffer buffer2;
        http::response<http::dynamic_body> response;
        
        boost::beast::http::read(*socket_server, buffer2, response,ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        // std::cout << "reponse header is " << response.base()<<std::endl;
        // // std::cout << "reponse method is " << response.at()<<std::endl;
        // std::cout << "reponse status is "<<response.result_int()<<std::endl;

        // Send the response to the client
        http::write(*socket, response,ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        socket_server->close();
    }

    /**
     * process GET method, if cached...
     * read reponse from server and send it to client
     * @param request request get from client
     * @param socket connection to client
    */
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

    /**
     * process CONNECT method, build a tunnel between client and server
     * send data in buffer between them
     * @param request request get from client
     * @param socket connection to client
    */
    void connect(http::request<http::dynamic_body> * request, tcp::socket * socket){
        std::string port;
        std::string host;

        //get server host and port
        isHTTPS(std::string(request->at("HOST")), &host, &port);
        tcp::socket * socket_server = connectToServer_socket(host.c_str(), port.c_str());

        //send success to client, build the tunnel
        int status;
        std::string message = "HTTP/1.1 200 OK\r\n\r\n";
        status = net::write(*socket, net::buffer(message),ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }

        int num = 0;
        while(true){
            //byte of data available to read from server
            int server_byte = socket_server->available(ec);
            if(ec){
                std::cerr<< ec.message()<<std::endl;
            }
            //byte of data available to read from client
            int clinet_byte = socket->available(ec);
            if(ec){
                std::cerr<< ec.message()<<std::endl;
            }
            //send message between client and server
            if(server_byte > 0){
                std::vector<char> bu1(server_byte);
                net::read(*socket_server, net::buffer(bu1));
                net::write(*socket, net::buffer(bu1));
            }
            if(clinet_byte > 0){
                std::vector<char> bu2(clinet_byte);
                net::read(*socket, net::buffer(bu2));
                net::write(*socket_server, net::buffer(bu2));
            }
            //if connection is closed, break the loop
            if(!socket_server->is_open()||!socket->is_open()){
                break;
            }
            num++;
        }
        socket_server->close();
    }
};

int main(){
    std::string host = "12345";
    Proxy p(host);
    p.run();
}