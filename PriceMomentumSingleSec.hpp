#ifndef  PRICEMOMENTUMSINGLESEC_HPP
#define  PRICEMOMENTUMSINGLESEC_HPP

#include <vector> //for using vector
#include <deque>
#include <tuple> //for using pair<> and tuple<>
#include <iterator> //for using vector iterator
#include <memory> //for using shared_ptr
#include <iostream>
#include <fstream> //for reading/writing flat files
#include <boost/algorithm/string.hpp> //for splitting strings
#include <boost/algorithm/string/regex.hpp> //for splitting strings
#include <boost/algorithm/string/regex.hpp> //for splitting strings
#include <string> //for using stol() stof()
#include <time.h> //for using struct tm, gmtime()
#include <limits> //for getting max and min value of float, int ect
#include <assert.h>
#include <mutex>
#include <dirent.h>
#include <unistd.h>  //for system sleeping
#include <ctime>  /for clock
#include <cstdio>
#include "MarketDataPoint.hpp"
#include "Utility.hpp"
#include "Strategy.hpp"
#include "RHApiCPP.hpp"
#include "RawMarketDataPoint.hpp"
#include "Utility.hpp"
#include "RHApiPy.hpp"
#include "rapidjson/document.h"
#include <atomic>
#include <thread>

using namespace std;
using namespace ds;
using namespace util;
using namespace strategy;
using namespace Robinhood;
using namespace rapidjson;

namespace strategy
{
typedef list<MarketDataPoint> MktDataPtr;

class PriceMomentumSingleSec:Strategy
{
private:
    //number of data point to decide whether a break out exists
    unsigned int bo_len;
    float chgspd_trg;
    float chgdur_trg;
    /*usually this strategy requires price continuously increases or decreases.
    but few opposite price movements are tolerable as long as the total of
    such reverse movement are limited to a certain amount.
    */
    int negResponse_trg;
    //market data
    MktDataPtr mktData;
    float retTrg;
    list<tuple<string, string, long int, float,bool>> entryVect;
    list<tuple<string, string, long int, float,bool>> cleanEntryVect;
    RHApiPy pyRobinAPI;
    RHApiCPP robinhoodAPI;
    //static std::mutex mutex_;

protected:
    long int totalCount = 0;
    long int falseCount = 0;
    long int trueCount = 0;
    float profit = 0.0;
    float loss = 0.0;
    int timeWindowForExit;
    ofstream log;
    string ticker;
    string instrumentId;
    float entryQuant;
    float entryPrice;
    float exitPrice;
    //PositionStatus posStatus;
    string side;
    string order_type;
    string time_in_force;
    string trigger;
    float orderPrice;
    std::mutex ioMutex;
    ofstream tradeLog;
    string logFilePath;
    Utility utility;

    long int delay_place_new_order;
	long int delay_close_order;
	long int delay_cancel_order;
	long int delay_positionstatus_check;
	long int delay_fetch_marketdata;
	long int delay_strategy_check;

public:
    PositionStatus posStatus;
    PriceMomentumSingleSec(string ticker);
    PriceMomentumSingleSec(string ticker,unsigned int bo_len,float chgspd_trg,float chgdur_trg, int negResponse_trg, float retTrg, int timeWindowForExit);
    PriceMomentumSingleSec(const PriceMomentumSingleSec & obj);
    ~PriceMomentumSingleSec();
    /**
    Two conditions need to be met to form a breakout:
    First, prices have been dropping/rallying for the last 15 mins. This can be changed to other number of data points, for example, 10.
    Second, the average minute to minute rate of change are greater or equal 0.5%/15mins
    Third, the prices were moving in an opposite direction before the breakout.
    */
    void DetectStg() override ;
    void StgSta() override;
    void Run() override;
    void Output() override ;
    void FetchMktData() override;
    void CleanEntryPoint() override;
    void CleanMktData() override;
    void WriteDataToFile(string ticker);
    void ReleaseMemory();
    void ExecTrade() override;
    void OpenNewPosition();
    void CloseLongPosition();
    void FetchMktData2();

};

}


#endif
