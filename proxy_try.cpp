#include "Cache_try.hpp"
#include <boost/bind/bind.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <locale>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;      
// using namespace boost::posix_time;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

class Proxy{
private:
    const char * host;
    const char * port;
    boost::asio::io_context io_context;
    boost::system::error_code ec;
    Cache cache;
public:
    Proxy(std::string p, int c):host(NULL), port(p.c_str()), cache(Cache(c)){}

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

        std::string port;
        std::string host;
        isHTTPS(std::string(request.at("HOST")), &host, &port);
        tcp::socket * socket_server = connectToServer_socket(host.c_str(), port.c_str());

        http::verb method = request.method();
        std::cout << "Received HTTP request: " << request << std::endl;
        if (method ==http::verb::get){
            GET(&request, socket, socket_server);
		}
        else if(method == http::verb::post){
            POST(&request, socket,socket_server);

        }else if(method == http::verb::connect){
            CONNECT(&request,socket,socket_server);
        }else{
            // if request is not a valid type, quit
            //should response 502
            // delete socket;
            std::cerr<<"Bad Request Type!!!"<<std::endl;
            return;
            // exit(1);
        }
        socket->close();
        socket_server->close();
        delete socket;
        delete socket_server;
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
     * @param socket_server connection to server
    */
    void POST(http::request<http::dynamic_body> * request, tcp::socket * socket, tcp::socket * socket_server){
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
        std::cout << "reponse header is " << response.base()<<std::endl;
        // // std::cout << "reponse method is " << response.at()<<std::endl;
        // std::cout << "reponse status is "<<response.result_int()<<std::endl;

        // Send the response to the client
        http::write(*socket, response,ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
    }

    /**
     * process CONNECT method, build a tunnel between client and server
     * send data in buffer between them
     * @param request request get from client
     * @param socket connection to client
     * @param socket_server connection to server
    */
    void CONNECT(http::request<http::dynamic_body> * request, tcp::socket * socket, tcp::socket * socket_server){
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
        // socket_server->close();
    }

    /**
     * process GET method, if cached...
     * read reponse from server and send it to client
     * @param request request get from client
     * @param socket connection to client
     * @param socket_server connection to server
    */
    void GET(http::request<http::dynamic_body> * request, tcp::socket * socket, tcp::socket * socket_server){
        // POST(request, socket, socket_server);

        std::string key;
        key = std::string((*request)[http::field::host]) + std::string(request->target());
        
        if(cache.isInCache(key)){
            //get response from cache
            http::response<http::dynamic_body> * response = cache.get(key);
            //no cache control, check fresh
            if(needValidation(response)){
                http::response<http::dynamic_body> vali_response = doValidation(socket_server,request, response);
                if(vali_response.result_int() ==200){
                    cache.update(key, vali_response);
                    http::write(*socket, vali_response);
                }else if(vali_response.result_int() == 304){
                    //modify reponse in cache, replace some values in header


                    //still need implementation
                    http::write(*socket, *response);
                }
            }else{
                // Send response to the client
                http::write(*socket, *response);
            }
        }else{
            //connect to server
            http::write(*socket_server, *request, ec);
            if(ec){
                std::cerr<< ec.message()<<std::endl;
            }

            //recieve the HTTP response from the server
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> response;
            
            boost::beast::http::read(*socket_server, buffer, response,ec);
            if(ec){
                std::cerr<< ec.message()<<std::endl;
            }
            
            //store in cache
            if(cacheCanStore(request, &response)){
                cache.put(key, response);
            }
            // Send the response to the client
            http::write(*socket, response,ec);
            if(ec){
                std::cerr<< ec.message()<<std::endl;
            }
        }
    }


    /**
     * check is the response get from a cache need to be validate
     * yes: 1. the response has "no-cache", "must-revalidate" in cache-control
     *      2. the response is not fresh
     * otherise, no
     * @param response the response stored in cache
     * @return true is need validate; false if not
    */
    bool needValidation(http::response<http::dynamic_body> * response){
        if(response->find(http::field::cache_control) == response->end()){
            if(isFresh(response)){
                return false;
            }else{
                return true;
            }
        }else{
            std::string str((*response)[http::field::cache_control]);
            std::map<std::string, long> fields = parseFields(str);
            
            //if no-store or must-revalidate, need validation
            if(fields.find("no-store") != fields.end() || fields.find("must-revalidate") != fields.end()){
                return true;
            }else{
                if(isFresh(response)){
                    return false;
                }else{
                    return true;
                }
            }
        } 
    }

    http::request<http::dynamic_body> makeConditionalRequest(http::request<http::dynamic_body> * request, http::response<http::dynamic_body> * response){
        http::request<http::dynamic_body> new_request = *request;
        //what is the two fields does not exist??


        new_request.set(http::field::if_none_match,(*response)[http::field::etag]);
        new_request.set(http::field::if_modified_since, (*response)[http::field::last_modified]);
        return new_request;
    }

     /**
     * do one validation. send conditional request to server
     * @param socket the socket connection to server
     * @param request the request send from client
     * @param response response saved in cache
     * @return the response got from the server, 200 if updated, 304 if not
    */
    http::response<http::dynamic_body> doValidation(tcp::socket * socket_server, http::request<http::dynamic_body> * request, http::response<http::dynamic_body> * response){
        http::request<http::dynamic_body> Crequest = makeConditionalRequest(request, response);
        http::write(*socket_server, Crequest, ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        //recieve the HTTP response from the server
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> new_response;
        
        boost::beast::http::read(*socket_server, buffer, new_response,ec);
        if(ec){
            std::cerr<< ec.message()<<std::endl;
        }
        return new_response;
    }

    /**
     * indicate whether the reponse can be stored in the cache
     * @param request
     * @param response
     * @return yes if can cache; no if not
    */
    bool cacheCanStore(http::request<http::dynamic_body> * request, http::response<http::dynamic_body> * response){
        //response is not cacheable 200 ok
        if(response->result_int() != 200){
            return false;
        }
        //the Authorization header field (see Section 4.2 of [RFC7235]) does not appear in the request, if the cache is shared,
        // unless the response explicitly allows it (see Section 3.2), and

        //no cache control field
        if(response->find(http::field::cache_control) == response->end()){
            return true;
        }else{
            std::string str((*response)[http::field::cache_control]);
            std::map<std::string, long> fields = parseFields(str);
            //the "no-store" cache directive does not appear in request or response header fields
            if(fields.find("no-store") != fields.end()){
                return false;
            }else{
                //the "private" response directive (see Section 5.2.2.6) does not
                //appear in the response, if the cache is shared, and
                if(fields.find("private") != fields.end()){
                    return false;
                }else{
                    return true;
                }
            }
        }
        
        return false;
    }

    /**
     * check whether the reponse in cache is still fresh
     * @param response the response stored in cache
     * @return true if still fresh; false if not
    */
    bool isFresh(http::response<http::dynamic_body> * response){
        time_t now;
        time(&now);
        time_t gmt_now = mktime(gmtime(&now));

        
        return false;
    }


};

int main(){
    // std::string host = "12345";
    // Proxy p(host, 1000);
    // p.run();

    // std::string a = "no-store, no-cache, max-age=1000, must-revalidate, proxy-revalidate";
    // std::map<std::string, long> b = parseFields(a);
    // for (auto it = b.begin(); it != b.end(); ++it) {
    //     std::cout << it->first << " => " << it->second << '\n';
    // }
    // std::vector<std::string> b = split(a, ',');
    // for(int i = 0 ; i < b.size();  i++){
    //     std::cout<<b[i] <<" is"<<std::endl;
    // }

    // std::string d1 = "Tue, 01 Feb 2022 12:30:45 GMT";
    // std::string d2 = "Tue, 01 Feb 2022 12:30:45 UTC";
    // pt::ptime p1 = getDatetime(d1);
    // pt::ptime p2 = getDatetime(d2);
    // std::cout << "Date: " << to_simple_string(p1) << std::endl;
    // std::cout << "Date: " << to_simple_string(p2) << std::endl;


    std::string d1 = "Tue, 01 Feb 2022 12:30:45 GMT";
    std::string d2 = "Tue, 01 Feb 2022 12:30:45 UTC";
    std::time_t p1 = parseDatetime(d1);
    time_t now;
    time(&now);
    std::cout << "Parsed time: now is " << asctime(gmtime(&now));
    std::cout << "Parsed time: " << ctime(&p1);
    // std::time_t p2 = getDatetime(d2);
    // std::cout << "Date: " << to_simple_string(p1) << std::endl;
    // std::cout << "Date: " << to_simple_string(p2) << std::endl;



    return 0;
}