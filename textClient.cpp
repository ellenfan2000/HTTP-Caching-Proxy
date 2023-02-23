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

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


int main(){
    net::io_context ioc;

    const char * hostname = "0.0.0.0";
    const char * port = "8080";

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    auto const target = "/forms/post";
    // int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;
    int version = 11;
    tcp::resolver::query query(hostname,port);

    // Look up the domain name
    auto const results = resolver.resolve(query);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, "httpbin.org");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // std::string r = "GET /forms/post HTTP/1.1\r\n"
    // "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
    // "Accept-Encoding: gzip, deflate\r\n"
    // "Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
    // "Cache-Control: max-age=0\r\n"
    // "Connection: keep-alive\r\n"
    // "Host: httpbin.org\r\n"
    // "Upgrade-Insecure-Requests: 1\r\n"
    // "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/109.0.0.0 Safari/537.36\r\n";

    // Send the HTTP request to the remote host
    http::write(stream, req);
    return EXIT_SUCCESS;
}