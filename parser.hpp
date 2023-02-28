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
#include <boost/date_time.hpp>
#include <ctime>          // std::tm

#include <sstream>
#include <locale>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;      
namespace pt = boost::posix_time;
namespace dt = boost::date_time;

time_t parseDatetime(std::string date_str){
    // std::locale loc(std::cout.getloc(), new boost::posix_time::time_input_facet("%a, %d %b %Y %H:%M:%S %Z"));
    // std::istringstream ss(date_str);
    // ss.imbue(loc);
    // pt::ptime date_time;
    // ss >> date_time;

    std::string format_str = "%a, %d %b %Y %H:%M:%S GMT";

    tm tm;
    tm.tm_isdst = 0;
    if (strptime(date_str.c_str(), format_str.c_str(), &tm) == NULL) {
        std::cerr << "Failed to parse HTTP-date string" << std::endl;
    }
    time_t time = mktime(&tm);

    // Print the parsed time
    // std::cout<<"String is: "<<date_str<<std::endl;
    // std::cout << "Parsed time: " << std::ctime(&time);
    return time;
}

std::vector<std::string> split(std::string str_, char delimiter){
    std::string str = "";
    for(int i = 0; i < str_.size(); i++){
        if(str_[i]!=' '){
            str+=str_[i];
        }
    }

    std::vector<std::string> result;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start));
    return result;
}

std::map<std::string, long> parseFields(std::string & str){
    std::map<std::string, long> result;
    std::vector<std::string> fields = split(str, ',');
    int end;
    for(int i  = 0; i < fields.size(); i++){
        end = 0;
        if((end= fields[i].find('=', 0)) != std::string::npos){
            result[fields[i].substr(0, end)] = std::stol(fields[i].substr(end+1),NULL,10);
        }else{
            result[fields[i]] = -1;
        }
    }
    return result;
}

std::string parseVersion(unsigned version){
    unsigned major = version / 10;
    unsigned minor = version % 10;
    std::string ans = "HTTP/";
    ans = ans+ std::to_string(major) + "."  +std::to_string(minor); 
    return ans;
}

void test(){
    // http::response<http::dynamic_body> response;
    // response.result(http::status::ok);
    // response.version(11);
    // // response.keep_alive(request->keep_alive());
    // response.set(http::field::server, "My Server");
    // response.set(http::field::content_type, "text/plain");
    // response.set(http::field::cache_control, "no-cache, must-revalidate, max-age=0");
    // std::cout<<response<<std::endl;

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

}