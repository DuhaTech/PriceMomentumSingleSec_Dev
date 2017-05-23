#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <fstream>
#include "RHApiCPP.hpp"
#include <Python.h>
#include "RHApiPy.hpp"
#include <unistd.h> //for sleep function
#include <cmath> //math functions
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "PriceMomentumSingleSec.hpp"
#include <thread>

using namespace Robinhood;
using namespace strategy;
using namespace std;
using namespace rapidjson;

int main(void)
{
//    RHApiCPP api;
//    api.Init();
//    shared_ptr<Document> quotes (new rapidjson::Document());
//    quotes.reset();
    //quotes = api.GetQuote("CHK");
    //std::cout<<quotes->operator[]("ask_price").GetString()<<std::endl;
    //std::cout<<api.GetInstrument("CHK");
    //PriceMomentumSingleSec obj("TDW");
    //obj.posStatus = util::ProcessingNewTrade;
    //obj.OpenNewPosition();

//    quotes.reset();
//    quotes = api.GetQuote("NFLX");
//    std::cout<<quotes->operator[]("ask_price").GetString()<<std::endl;
//    const unsigned int bo_len = 10*60; //number of minutes during which the price has been moving in the same direction
//    const float chgspd_trg = 0.007 / 15.00 / 60.00;
//    const int negResponse_trg = 5;
//    const float retTrg = 0.1/100;
//    const int timeWindowForExit = 10*60; //the wider this window, the more risky is the strategy
//    const long int secToFormStrategy = 10*60;
    //PriceMomentumSingleSec priceMom("VXX",bo_len, chgspd_trg, secToFormStrategy,  negResponse_trg,  retTrg,  timeWindowForExit);


    ifstream file;
    vector<string> tickerList;
    file.open("tickerList.txt");
     std::string content( (std::istreambuf_iterator<char>(file) ),
                       (std::istreambuf_iterator<char>()    ) );
    Document document;
    document.Parse(content.c_str());
    file.close();
   string tickers = document["ticker"].GetString();
   //tickers =
    boost::split(tickerList,tickers, boost::is_any_of(","));

    vector<shared_ptr<thread>> threadList;
    vector<shared_ptr<PriceMomentumSingleSec>> strategyList;
    std::cout<<tickers<<"."<<std::endl;
    //RHApiCPP* obj = new RHApiCPP();
    //obj->Init();
    //std::cout<<obj->GetQuote("AVP");
    //usleep(4000000);
    //PriceMomentumSingleSec* obj2 = new PriceMomentumSingleSec("NFLX");
    //obj2->Run();
    //obj.FetchMktData();
    //std::thread t1(&RHApiCPP::GetQuote, obj,"BAC");
    //std::thread t2(&PriceMomentumSingleSec::FetchMktData, obj2);
   //t1.join();
    //t2.join();
    int count = 0;

    for(auto it = tickerList.begin(); it != tickerList.end(); ++it)
    {
        std::cout<<*it<<endl;
        //PriceMomentumSingleSec* obj= new PriceMomentumSingleSec(*it);
        //objList.push_back(obj);
        //std::thread*  threadObj = new thread(&PriceMomentumSingleSec::Run, new PriceMomentumSingleSec(*it));
        //threadObj->join();
if(*it != "")
{


        strategyList.push_back(make_shared<PriceMomentumSingleSec>(*it));
        threadList.push_back(make_shared<thread>(&PriceMomentumSingleSec::Run,strategyList[count].get()));


        //threadList.push_back(new thread(&PriceMomentumSingleSec::FetchMktData, new PriceMomentumSingleSec(*it)));
        //threadList.push_back(threadObj);
        //threadList[count]->join();
        //std::cout<<"end"<<endl;
        count++;
}
    }

    for(auto it = threadList.begin(); it != threadList.end();++it)
    {
        (*it)->join();
    }

    //vector<PriceMomentumSingleSec> objList;

    //priceMom.FetchMktData();
    //priceMom.ExecTrade();
    //RHApiPy py;

    /*
    std::thread strategyMain(&PriceMomentumSingleSec::ExecTrade, &priceMom);
    std::thread strategySetMarketData(&PriceMomentumSingleSec::SetMktData, &priceMom);
    std::thread strategyOpenNewPosition(&PriceMomentumSingleSec::OpenNewPosition, &priceMom);
    std::thread strategyCloseLongPosition(&PriceMomentumSingleSec::CloseLongPosition, &priceMom);

    strategyMain.join();
  strategySetMarketData.join();
    strategyOpenNewPosition.join();
    strategyCloseLongPosition.join();
    */


  getchar();


  return 0;
}
