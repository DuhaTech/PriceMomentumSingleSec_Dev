#include "RHApiCPP.hpp"

using namespace rapidjson;

namespace Robinhood
{
//CURL * RobinhoodAPI::curl = curl_easy_init();
//struct response RobinhoodAPI::resStr =  NULL;
//CURLcode RobinhoodAPI::resCode =  CURLE_OK;

RHApiCPP::RHApiCPP()
{
    Init();
}

void RHApiCPP::Init()
{
    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    resCode =  CURLE_OK;

    struct curl_slist* list = NULL;
    list = curl_slist_append(list, "Accept:*/*");
    list = curl_slist_append(list,"Accept-Encoding:utf-8, gzip, deflate");
    list = curl_slist_append(list,"Accept-Language:en;q=1, fr;q=0.9, de;q=0.8, ja;q=0.7, nl;q=0.6, it;q=0.5");
    list = curl_slist_append(list,"Content-Type: application/x-www-form-urlencoded; charset=utf-8");
    list = curl_slist_append(list,"X-Robinhood-API-Version:1.0.0");
    list = curl_slist_append(list,"Connection:keep-alive");
    list = curl_slist_append(list,"User-Agent: Robinhood/823 (iPhone; iOS 7.1.2; Scale/2.00)");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);

    GetToken();
    list = curl_slist_append(list, ("Authorization: Token " + token).c_str());

            /*separate the inintialization for curl_get */

    quoteDetail = std::make_shared<Document>();
    instrumentDetail = std::make_shared<Document>();
    init_string(&resStr_Get);
    res_Get =  CURLE_OK;
    curl_Get = curl_easy_init();
}

RHApiCPP::~RHApiCPP()
{
    /* always cleanup */
    curl_easy_cleanup(curl_Get);
    curl_easy_cleanup(curl);

}
std::string RHApiCPP::GetToken()
{
    struct response resStr;
    init_string(&resStr);
    curl_easy_setopt(curl, CURLOPT_POST,1);
    std::string loginCredentials = "username=jhuang0724&password=HjhCh6969!";
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, loginCredentials.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.robinhood.com/api-token-auth/");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);

    resCode = curl_easy_perform(curl);

    if(resCode != CURLE_OK)
    {
        return "";
    }

    std::string rhResponse = std::string(resStr.ptr);
    token = rhResponse.substr(10, rhResponse.size() - 12);
    authentication = true;
    return token;
}

/* place and order given order parameters.
returns order ID
*/
std::unique_ptr<Document> RHApiCPP::PlaceOrder(std::string symbol,
        int quantity, std::string side, std::string type,
        std::string time_for_force, std::string trigger,
        double price, std::string instrument)
{
    struct response resStr;
    init_string(&resStr);
    //CURLcode res;
    //std::string orderSpec = "symbol=" + symbol + "&price=" + std::to_string(price) + "&quantity=" + std::to_string(quantity) + "&side=" + side + "&time_in_force=" + time_for_force + "&type=" + type + "&trigger=" + trigger + "&account=https://api.robinhood.com/accounts/5QS63703/" + "&instrument=https://api.robinhood.com/instruments/7da371f1-ca40-4c5d-ae4a-7d75c9437b73/";
    //WKHS
    //std::string orderSpec = "symbol=" + symbol + "&price=" + std::to_string(price) + "&quantity=" + std::to_string(quantity) + "&side=" + side + "&time_in_force=" + time_for_force + "&type=" + type + "&trigger=" + trigger + "&account=https://api.robinhood.com/accounts/5QS63703/" + "&instrument=https://api.robinhood.com/instruments/85b61d0d-257b-43ee-8498-a69fb02703a4/";
    std::string orderSpec = "symbol=" + symbol + "&price=" + std::to_string(price) + "&quantity=" + std::to_string(quantity) + "&side=" + side + "&time_in_force=" + time_for_force + "&type=" + type + "&trigger=" + trigger + "&account=https://api.robinhood.com/accounts/5QS63703/" + "&instrument=" + instrument;
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, orderSpec.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.robinhood.com/orders/");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);

    resCode = curl_easy_perform(curl);

    if(resCode != CURLE_OK)
    {
        std::cout<<"error"<<std::endl;
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(resCode));
        //return "";
    }

    //std::string test = "{"updated_at":"2017-01-08T04:01:05.115557Z","ref_id":null,"time_in_force":"gfd","fees":"0.00","cancel":"https:\/\/api.robinhood.com\/orders\/6e9e57ef-7b21-42d0-bd85-428023f02baa\/cancel\/","id":"6e9e57ef-7b21-42d0-bd85-428023f02baa","cumulative_quantity":"0.00000","stop_price":null,"reject_reason":null,"instrument":"https:\/\/api.robinhood.com\/instruments\/8f92e76f-1e0e-4478-8580-16a6ffcfaef5\/","state":"unconfirmed","trigger":"immediate","type":"limit","last_transaction_at":"2017-01-08T04:01:05.033252Z","price":"1.00000000","executions":[],"extended_hours":false,"account":"https:\/\/api.robinhood.com\/accounts\/5QS63703\/","url":"https:\/\/api.robinhood.com\/orders\/6e9e57ef-7b21-42d0-bd85-428023f02baa\/","created_at":"2017-01-08T04:01:05.033252Z","side":"buy","position":"https:\/\/api.robinhood.com\/accounts\/5QS63703\/positions\/8f92e76f-1e0e-4478-8580-16a6ffcfaef5\/","average_price":null,"quantity":"1.00000"}";
    /*std::vector<std::string> responseVect;
    std::string resTemp = std::string(resStr.ptr);
    boost::split(responseVect, resTemp, boost::is_any_of(","));
    std::string orderId = responseVect[5];
    orderId = orderId.substr(6,36);
    std::cout<<std::string(resStr.ptr)<<std::endl;
    */
    std::unique_ptr<Document> orderDetail(new Document());
    orderDetail->Parse(resStr.ptr);
    return orderDetail;

}

size_t RHApiCPP::writefunc(void *ptr, size_t size, size_t nmemb,  struct response *s)
{
    if(ptr == nullptr || ptr == NULL)
    {
        std::cout<<"null pointer"<<std::endl;
    }
    size_t new_len = s->len + size*nmemb;
    s->ptr = (char*)realloc(s->ptr, new_len+1);

    if (s->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    std::memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

void RHApiCPP::init_string(struct response *s)
{
    s->len = 0;
    s->ptr = (char*) malloc(s->len+1);
    if (s->ptr == NULL)
    {
        std::cout<<"malloc() failed"<<std::endl;
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

std::string RHApiCPP::GetInstrument(std::string ticker)
{
    std::string url = "https://api.robinhood.com/instruments/?symbol=" + ticker;
      std::string instrumentURL;

    //struct response s;
    //init_string(&resStr_Get);
    //CURLcode res;
    //struct response resStr;
    init_string(&resStr_Get);
    if(curl_Get)
    {
        curl_easy_setopt(curl_Get, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_Get,CURLOPT_WRITEFUNCTION,writefunc);
        curl_easy_setopt(curl_Get, CURLOPT_WRITEDATA, &resStr_Get);
    }

    try
    {
        resCode = curl_easy_perform(curl_Get);
    instrumentDetail->Parse(resStr_Get.ptr);
    if(instrumentDetail->operator[]("results").Capacity() != 0)
    {
//std::cout<<resStr_Get.ptr<<std::endl;

instrumentURL = instrumentDetail->operator[]("results")[0]["url"].GetString(); //<<std::endl;
    }
    else
    {
        //std::cout<<resStr_Get.ptr<<std::endl;
        instrumentURL = " ";
    }
    }
    catch(...)
{

    //std::cout<<resStr_Get.ptr<<std::endl;
    std::cout<<"exception"<<std::endl;
    }

    return instrumentURL;
}

std::unique_ptr<Document> RHApiCPP::CancelOrder(std::string orderID)
{
    struct response resStr;
    std::string orderState = "";
    init_string(&resStr);
    std::string url = "https://api.robinhood.com/orders/" + orderID + "/cancel/";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    resCode = curl_easy_perform(curl);

    std::unique_ptr<Document> orderStatus = GetOrderStatus(orderID);
    //orderStatus.Parse(resStr.ptr);
    orderState = orderStatus->operator[]("state").GetString();

    while(orderState != "filled" && orderState != "canceled")
    {
        usleep(2000000);
        resCode = curl_easy_perform(curl);
        orderStatus.reset();
        orderStatus = GetOrderStatus(orderID);
        orderState = orderStatus->operator[]("state").GetString();
    }

    return orderStatus;
}

std::shared_ptr<Document> RHApiCPP::GetQuote(std::string ticker)
//void RHApiCPP::GetQuote(std::string ticker)
{
    //curl_easy_reset(curl_Get);
    init_string(&resStr_Get);
    std::string url = "https://api.robinhood.com/quotes/" + ticker + "/";
    //std::cout<<url<<std::endl;

    if(curl_Get)
    {
        curl_easy_setopt(curl_Get, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_Get, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl_Get, CURLOPT_WRITEDATA, &resStr_Get);
    }
#ifdef SKIP_HOSTNAME_VERIFICATION
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    /* Perform the request, res will get the return code */
    //m.lock();
    res_Get = curl_easy_perform(curl_Get);
    while(res_Get != CURLE_OK)
    {
        usleep(5000000);
        res_Get = curl_easy_perform(curl_Get);
    }
    //m.unlock();
    //std::cout<<ticker<<std::endl;
    //std::unique_ptr<Document> orderDetail(new Document());
    quoteDetail->Parse(resStr_Get.ptr);
    return quoteDetail;
}

std::string RHApiCPP::GetOrder(std::string ticker)
{
    curl_easy_setopt(curl, CURLOPT_POST, 0);
    struct response resStr;
    init_string(&resStr);
    std::string url = "https://api.robinhood.com/orders/";
    //85b61d0d-257b-43ee-8498-a69fb02703a4
    //f85150aa-6e4e-4d1c-bff3-d7074084ee9c
    //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "utf-8");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);
    //curl_easy_setopt(curl, CURLOPT_POST, 1);
    //curl_easy_setopt(curl,CURLOPT_POSTFIELDS, "account=https://api.robinhood.com/accounts/5QS63703/&instrument=https://api.robinhood.com/instruments/85b61d0d-257b-43ee-8498-a69fb02703a4/");
    resCode = curl_easy_perform(curl);

    return std::string(resStr.ptr);

}

/*return order status given order ID */
std::unique_ptr<Document> RHApiCPP::GetOrderStatus(std::string orderID)
{
    struct response resStr;
    std::string status = "";
    std::vector<std::string> temp;
    init_string(&resStr);
    curl_easy_setopt(curl, CURLOPT_POST, 0); //reset option because this is not a post request
    //orderID = "919cb0f2-6a09-4ad2-b639-53652dd014bf";
    std::string url = "https://api.robinhood.com/orders/" + orderID + "/"; //919cb0f2-6a09-4ad2-b639-53652dd014bf
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);
    resCode = curl_easy_perform(curl);

    /**retry every 5 seconds if the connection failed*/
    while(resCode != CURLE_OK)
    {
        usleep(2000000);
        resCode = curl_easy_perform(curl);
    }
    /*
    status = std::string(resStr.ptr);
    //std::cout<<status<<std::endl;

    boost::split(temp,status,boost::is_any_of(","));
    status = temp[10];
    status = status.substr(9,status.size() - 10);

    std::string quant = temp[6];
    quant = quant.substr(23, quant.size() - 24);
    float quantFlt = std::stod(quant);

    std::string filledPrice = temp[21];
    filledPrice = filledPrice.substr(16, filledPrice.size() - 17);
    float filledPriceFlt = 0;
    if(quantFlt > 0)
    {
        filledPriceFlt = std::stod(filledPrice);
    }
     temp.clear();
    */
    std::unique_ptr<Document> orderStatus(new Document());
    orderStatus->Parse(resStr.ptr);
    return orderStatus;
}

void RHApiCPP::test()
{
    std::cout<<"mark"<<std::endl;
}
}


