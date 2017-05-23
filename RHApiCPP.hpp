#ifndef RHAPICPP_HPP
#define RHAPICPP_HPP

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <boost/algorithm/string/regex.hpp> //for splitting a string
#include <boost/algorithm/string.hpp> //for splitting a string
#include <rapidjson/document.h>
#include <mutex>

using namespace rapidjson;

namespace Robinhood
{

struct response
{
    char *ptr;
    size_t len;
};


class RHApiCPP
{
public:
    std::string token;
    std::string account;
    CURL *curl;
    CURL *curl_Get;
    CURLcode resCode;
    bool authentication = false;
    struct response resStr_Get;
    CURLcode res_Get;
    std::shared_ptr<Document> quoteDetail;
    std::shared_ptr<Document> instrumentDetail;
    std::mutex m;

public:
    RHApiCPP();
    void test();
    void Login();
    std::shared_ptr<Document> GetQuote(std::string symbol);
    //void GetQuote(std::string symbol);
    std::unique_ptr<rapidjson::Document> PlaceOrder(std::string symbol,
            int quantity, std::string side, std::string type,
            std::string time_for_force, std::string trigger,
            double price, std::string instrument
                                                   );
    void Init();
    void init_string(struct response *s);
    static size_t writefunc(void *ptr, size_t size, size_t nmemb,  struct response *s);
    std::string GetInstrument(std::string ticker);
    std::unique_ptr<Document> CancelOrder(std::string url);
    std::string GetOrder(std::string ticker);
    std::unique_ptr<rapidjson::Document> GetOrderStatus(std::string ticker);
    std::string GetToken();
    //static std::string GetQuote(std::string ticker);
    //static size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
    ~RHApiCPP();


};
}
#endif // ROBINHOODAPI_HPP
