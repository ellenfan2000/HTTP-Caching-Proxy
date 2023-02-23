#include "Cache_try.hpp"
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

class Proxy{
private:
    const char * host;
    const char * port;

    Cache c;
public:
    Proxy(std::string p):host(NULL), port(p.c_str()){}

    void run(){
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), strtol(port, NULL, 0)));
        while (true){
            tcp::socket * socket = new tcp::socket(io_context);
            // pthread_t thread;
            acceptor.accept(*socket);
            std::thread t(&Proxy::requestProcess, this, socket);
            t.detach();
        }
    }

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
        delete socket;

    }

    bool ifHTTPS(std::string & hostname){
        if(hostname.find(":") != std::string::npos){
            return true;
        }
        return false;
    }

    void post(http::request<http::dynamic_body> * request, tcp::socket * socket){
        // Send the HTTP request to requested server HTTP not HTTPS
        beast::tcp_stream * stream = connectToServer(std::string(request->at("HOST")).c_str(), "80");
        http::write(*stream, *request);

        //recieve the HTTP response from the server
        boost::beast::flat_buffer buffer2;
        http::response<http::dynamic_body> response;
        boost::beast::http::read(*stream, buffer2, response);

        // Send the response to the client
        http::write(*socket, response);
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
        post(request, socket);
    }
};

int main(){
    std::string host = "12345";
    Proxy p(host);
    p.run();
}