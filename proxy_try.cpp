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

class Proxy{
private:
    const char * host;
    const char * port;
    boost::asio::io_context io_context;
    Cache cache;
    int id = 0; //request id
    std::ofstream LogStream;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

public:
    Proxy(std::string p, int c):host(NULL), port(p.c_str()), cache(Cache(c)), LogStream(std::ofstream("proxy.log")){}

    void run(){
        boost::system::error_code ec;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), strtol(port, NULL, 0)));
        while (true){
            //accept client connection
            tcp::socket * socket = new tcp::socket(io_context);
            acceptor.accept(*socket, ec);
            id++;
            if(ec.value() != 0){
                //if cannot connect, go to next
                std::cerr<<"cannot connect with client: "<< ec.message()<<std::endl;
                socket->close();
                continue;
            }
            std::thread t(&Proxy::requestProcess, this, socket, id-1);
            t.detach();
        }
    }
    
    /**
     * make connection from proxy to server
     * @param host server hostname
     * @param port host port
     * @return tcp socket of the connection
    */
    tcp::socket * connectToServer(const char * host, const char * port){
        boost::system::error_code ec;
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

    /**
     * process incomming request from client:
     * POST
     * GET
     * CONNECT
     * @param socket client connection
    */
    void requestProcess(tcp::socket * socket, int ID){
        boost::system::error_code ec;
        net::ip::tcp::endpoint client_ip = socket->remote_endpoint();
        time_t now;
        time(&now);
        time_t gmt_now = mktime(gmtime(&now));
        //read request from client
        beast::flat_buffer buffer;
        http::request<http::dynamic_body> request;
        http::read(*socket, buffer, request, ec);
        if(ec.value() != 0 || request.find(http::field::host) == request.end()){
            //read invalid request
            std::cerr<< "Read Request error: " << ec.value() <<", "<<ec.to_string()<< ", "<<ec.message()<<std::endl;
            http::write(*socket, make400Response(&request, ID),ec);
            if(ec.value() != 0){
                std::cerr<< "Send 400 error: " << ec.value() <<", "<<ec.to_string()<< ", "<<ec.message()<<std::endl;
            }

            pthread_mutex_lock(&lock);
            LogStream<<ID<<": Invalid Request from " << client_ip.address()\
            <<" @ "<<ctime(&gmt_now);
            pthread_mutex_unlock(&lock);

            socket->close();
            delete socket;
            return;
        }

        pthread_mutex_lock(&lock);
        LogStream<<ID<<": \""<<request.method()<<" "<<request.target()\
        <<" "<<parseVersion(request.version())<<"\" from " <<client_ip.address()\
        <<" @ "<<ctime(&gmt_now);
        pthread_mutex_unlock(&lock);

        //connect to server
        std::string port;
        std::string host;
        isHTTPS(std::string(request.at("HOST")), &host, &port);
        tcp::socket * socket_server = connectToServer(host.c_str(), port.c_str());

        http::verb method = request.method();
        // std::cout << "Received HTTP request: " << request << std::endl;
        if (method ==http::verb::get){
            try{
                GET(&request,ID,  socket, socket_server);
            }catch(std::exception & e){
                std::cerr<< "GET error:" <<e.what()<< std::endl;
                pthread_mutex_lock(&lock);
                LogStream<<ID<<": Connection Lost"<<std::endl;
                pthread_mutex_unlock(&lock);
            }
		}
        else if(method == http::verb::post){
            try{
                POST(&request,ID,  socket,socket_server);
            }catch(std::exception & e){
                std::cerr<< "POST error:" <<e.what()<< std::endl;
                pthread_mutex_lock(&lock);
                LogStream<<ID<<": Connection Lost"<<std::endl;
                pthread_mutex_unlock(&lock);
            }

        }else if(method == http::verb::connect){
            try{
                CONNECT(&request,ID, socket,socket_server);
            }catch(std::exception & e){
                pthread_mutex_lock(&lock);
                LogStream<<ID<<": Tunnel closed"<<std::endl;
                pthread_mutex_unlock(&lock);
                std::cerr<< "CONNECT error:" <<e.what()<< std::endl;
            }

        }else{
            // if request method is not a valid type, should response 400
            http::write(*socket, make400Response(&request, ID),ec);
            std::cerr<<"Bad Request Type!!!"<<std::endl;
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
    void POST(http::request<http::dynamic_body> * request,int ID,  tcp::socket * socket, tcp::socket * socket_server){
        // boost::system::error_code ec;
        http::write(*socket_server, *request);
        //recieve the HTTP response from the server
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> response;
        
        boost::beast::http::read(*socket_server, buffer, response);
        // std::cout << "reponse header is " << response.base()<<std::endl;

        // Send the response to the client
        http::write(*socket, response);
    }

    /**
     * process CONNECT method, build a tunnel between client and server
     * send data in buffer between them
     * @param request request get from client
     * @param socket connection to client
     * @param socket_server connection to server
    */
    void CONNECT(http::request<http::dynamic_body> * request,int ID, tcp::socket * socket, tcp::socket * socket_server){
        //send success to client, build the tunnel
        int status;
        std::string message = "HTTP/1.1 200 OK\r\n\r\n";
        status = net::write(*socket, net::buffer(message));
        while(true){
            //if connection is closed, break the loop see error code
            //byte of data available to read from server
            int server_byte = socket_server->available();
            //byte of data available to read from client
            int clinet_byte = socket->available();

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
                pthread_mutex_lock(&lock);
                LogStream<<ID<<": Tunnel closed"<<std::endl;
                pthread_mutex_unlock(&lock);
                break;
            }
        }
    }

    /**
     * process GET method, if cached...
     * read reponse from server and send it to client
     * @param request request get from client
     * @param socket connection to client
     * @param socket_server connection to server
    */
    void GET(http::request<http::dynamic_body> * request,int ID, tcp::socket * socket, tcp::socket * socket_server){
        // POST(request, socket, socket_server);
        boost::system::error_code ec;
        std::string key;
        key = std::string((*request)[http::field::host]) + std::string(request->target());
        
        if(cache.isInCache(key)){
            //get response from cache
            http::response<http::dynamic_body> * response = cache.get(key);
            
            if(needValidationWhenAccess(response, ID)){
                pthread_mutex_lock(&lock);
                LogStream<<ID << ": in cache, requires validation"<<std::endl;
                pthread_mutex_unlock(&lock);
                http::response<http::dynamic_body> vali_response = doValidation(socket_server,request, response);
                if(vali_response.result_int() ==200){
                    //multithread safe, error handling
                    cache.update(key, vali_response);
                    http::write(*socket, vali_response);
                }else if(vali_response.result_int() == 304){
                    //modify reponse in cache, replace some values in header
                    
                    //still need implementation
                    http::write(*socket, *response);
                }
            }else{
                pthread_mutex_lock(&lock);
                LogStream<<ID<< ": in cache, valid"<<std::endl;
                pthread_mutex_unlock(&lock);
                // Send response to the client
                http::write(*socket, *response);
            }
            // std::cout<<"Cached response is: "<<response->base()<<std::endl;
        }else{
            pthread_mutex_lock(&lock);
            LogStream<<ID<<": not in cache"<<std::endl;
            LogStream<<ID<<": Requesting \""<<request->method()<<" "<<request->target()\
            <<" "<<parseVersion(request->version())<<"\" from " << request->at("host")<<std::endl;
            pthread_mutex_unlock(&lock);
            //connect to server
            http::write(*socket_server, *request);
            //recieve the HTTP response from the server
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> response;
            
            boost::beast::http::read(*socket_server, buffer, response);
            pthread_mutex_lock(&lock);
            LogStream<<ID<<": Received \"" \
            << parseVersion(response.version())<< " " << response.result_int() <<" "<< response.reason() \
            <<"\" from "<< request->at("host")<<std::endl;
            pthread_mutex_unlock(&lock);
            
            //store in cache
            if(cacheCanStore(request, &response, ID)){
                if(hasValidation(&response, ID)){
                    pthread_mutex_lock(&lock);
                    LogStream<<ID<< ": cached, but requires re-validation"<<std::endl;
                    pthread_mutex_unlock(&lock);
                }
                time_t expire;
                if(getExpireTime(&response, ID,&expire)==1){
                    pthread_mutex_lock(&lock);
                    LogStream<<ID<< ": cached, expires at "<<ctime(&expire);
                    pthread_mutex_unlock(&lock);
                }
                else{
                    pthread_mutex_lock(&lock);
                    LogStream<<ID<< ": cached, does not expire "<<std::endl;
                    pthread_mutex_unlock(&lock);
                }
                cache.put(key, response);
            }
            // Send the response to the client
            http::write(*socket, response);
            // std::cout<<"response is: "<<response.base()<<std::endl;
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
    bool needValidationWhenAccess(http::response<http::dynamic_body> * response, int ID){
        if(hasValidation(response, ID)){
             pthread_mutex_lock(&lock);
            LogStream<<ID << ": in cache, requires validation"<<std::endl;
            pthread_mutex_unlock(&lock);
            return true;
        }else{
            if(isFresh(response, ID)){
                return false;
            }else{
                time_t expire;
                if(getExpireTime(response, ID,&expire)==1){
                    pthread_mutex_lock(&lock);
                    LogStream<<ID<< ": in cache, but expired at "<<ctime(&expire);
                    pthread_mutex_unlock(&lock);
                }

                return true;
            }
        }
    
    }

    int getExpireTime(http::response<http::dynamic_body> * response, int ID, time_t * expire){
        //has cache control
        if(response->find(http::field::cache_control) != response->end()){
            std::string str((*response)[http::field::cache_control]);
            std::map<std::string, long> fields = parseFields(str);

	        std::string date_str((*response)[http::field::date]);
            time_t date_value = parseDatetime(date_str);

            if(fields.find("max-age") != fields.end()){
                *expire = date_value + fields["max-age"];
                return 1;
            }else{
                //never expire
                expire = NULL;
                return 0;
            }
        }else{
            if(response->find(http::field::expires)== response->end()){
                //never expire
                expire = NULL;
                return 0; 
            }else{
                std::string expire_str((*response)[http::field::expires]);
                //what is the str is just 0 or -1;
                *expire = parseDatetime(expire_str);
                return 1;
             }
        }
        return 0;
    }

    bool hasValidation(http::response<http::dynamic_body> * response, int ID){
        if(response->find(http::field::cache_control) != response->end()){
            std::string str((*response)[http::field::cache_control]);
            std::map<std::string, long> fields = parseFields(str);
            
            //if no-cache or must-revalidate, need validation
            if(fields.find("no-cache") != fields.end() || fields.find("must-revalidate") != fields.end()){
                return true;
            }
        }
        return false;
    }

    /**
     * Based on the new request get from client and old response saved in cache,
     * add if_none_match and if_modified_since fields to generate the new conditonal request
     * @param request new request get from client
     * @param reponse old response saved in cache
     * @return conditonal request
    */
    http::request<http::dynamic_body> makeConditionalRequest(http::request<http::dynamic_body> * request, http::response<http::dynamic_body> * response){
        http::request<http::dynamic_body> new_request = *request;
        //what if the two fields does not exist??
        if(response->find(http::field::etag)!=response->end()){
            new_request.set(http::field::if_none_match,(*response)[http::field::etag]);
        }
        
        if(response->find(http::field::last_modified)!=response->end()){
             new_request.set(http::field::if_modified_since, (*response)[http::field::last_modified]);
        }
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
        boost::system::error_code ec;
        http::request<http::dynamic_body> Crequest = makeConditionalRequest(request, response);
        http::write(*socket_server, Crequest, ec);
        if(ec){
            std::cerr<< "Validation: Send Conditional request error: "<<ec.to_string()<<", "<<ec.message()<< std::endl;
        }
        //recieve the HTTP response from the server
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> new_response;
        
        boost::beast::http::read(*socket_server, buffer, new_response,ec);
        if(ec){
            std::cerr<<"Validation: Recieve conditional request result from server error: "<<ec.to_string()<<", "<<ec.message()<< std::endl;
        }
        return new_response;
    }

    /**
     * indicate whether the reponse can be stored in the cache
     * @param request
     * @param response
     * @return yes if can cache; no if not
    */
    bool cacheCanStore(http::request<http::dynamic_body> * request, http::response<http::dynamic_body> * response, int ID){
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
                pthread_mutex_lock(&lock);
                LogStream<<ID <<": not cacheable because \"no-store\""<<std::endl;
                pthread_mutex_unlock(&lock);
                return false;
            }else{
                //the "private" response directive (see Section 5.2.2.6) does not
                //appear in the response, if the cache is shared, and
                if(fields.find("private") != fields.end()){
                    pthread_mutex_lock(&lock);
                    LogStream<<ID <<": not cacheable because \"private\""<<std::endl;
                    pthread_mutex_unlock(&lock);
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
    bool isFresh(http::response<http::dynamic_body> * response, int ID){
        time_t now;
        time(&now);
        time_t gmt_now = mktime(gmtime(&now));

        time_t expire;
        int status = getExpireTime(response, ID, &expire);
        if(status == 0){
            return true;
        }else{
            if (expire > gmt_now){
                return true;
            }else{
                return false;
            }
        }
    }
   

    http::response<http::dynamic_body> make400Response(http::request<http::dynamic_body> * request, int ID ){
        http::response<http::dynamic_body> response;
        response.result(boost::beast::http::status::bad_request);
        response.version(request->version());
        // response.keep_alive(request->keep_alive());
        response.set(boost::beast::http::field::server, "My Server");
        response.set(boost::beast::http::field::content_type, "text/plain");
        // response.body() = "Bad Request";
        response.prepare_payload();
        pthread_mutex_lock(&lock);
        LogStream<<ID<<": Responding \"" \
        << parseVersion(response.version())<< " " << response.result_int() <<"\""<< response.reason()<<std::endl;
        pthread_mutex_unlock(&lock);

        return response;
    }

};

int main(){
    std::string host = "12345";
    Proxy p(host, 1000);
    p.run();

    // unsigned a = 10;
    // std::cout<<parseVersion(a)<<std::endl;
    // unsigned b = 11;
    // std::cout<<parseVersion(b)<<std::endl;


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


    // std::string d1 = "Tue, 01 Feb 2022 12:30:45 GMT";
    // std::string d2 = "Tue, 01 Feb 2022 12:30:46 GMT";
    // std::string d3 = "Sun, 26 Feb 2023 23:51:19 GMT";
    // std::time_t p1 = parseDatetime(d1);
    // std::time_t p2 = parseDatetime(d2);
    // std::time_t p3 = parseDatetime(d3);
    // time_t now;
    // time(&now);
    // std::cout << "Parsed time: now is " << asctime(gmtime(&now));
    // std::cout << "Parsed time: " << ctime(&p1);
    // std::cout << "Parsed time: " << ctime(&p1);
    // bool a = (p1 < now );
    // bool b = (p2 < p1);
    // std::cout << "Date: " << a << b <<std::endl;
    // std::time_t p2 = getDatetime(d2);
    // std::cout << "Date: " << to_simple_string(p1) << std::endl;
    // std::cout << "Date: " << to_simple_string(p2) << std::endl;



    return 0;
}
