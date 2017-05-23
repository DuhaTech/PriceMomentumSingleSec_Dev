#include "PriceMomentumSingleSec.hpp"

//std::mutex PriceMomentumSingleSec::mutex_;
//atomic<int> PriceMomentumSingleSec::timeWindowForExit;
//atomic<float> PriceMomentumSingleSec::entryQuant(0);
//atomic<double> PriceMomentumSingleSec::entryPrice(0);
//atomic<PositionStatus> PriceMomentumSingleSec::posStatus(ReadyForNewTrade);
//atomic<float> PriceMomentumSingleSec::orderPrice(0);
//PriceMomentumSingleSec::PriceMomentumSingleSec(){}

PriceMomentumSingleSec::PriceMomentumSingleSec(string ticker_, unsigned int bo_len_, float chgspd_,float chgdur_,int negResponse, float retTrg_, int timeWindowForExit_):
    ticker(ticker_),bo_len(bo_len_),chgspd_trg(chgspd_), chgdur_trg(chgdur_),negResponse_trg(negResponse), retTrg(retTrg_), timeWindowForExit(timeWindowForExit_),
    mktData(), cleanEntryVect(),side(""), order_type("limit"),time_in_force("gfd"), trigger("immediate"),entryVect()
{
    instrumentId = pyRobinAPI.GetInstrument(ticker);
    posStatus = ReadyForNewTrade;
}

PriceMomentumSingleSec::PriceMomentumSingleSec(const PriceMomentumSingleSec& obj)
{

}

PriceMomentumSingleSec::PriceMomentumSingleSec(string ticker_)
{

    ticker = ticker_;
    ifstream configFile;

    ioMutex.lock();
    logFilePath = "log/tradeLog.txt";
    tradeLog.open(logFilePath,ios::app);
    configFile.open("config.txt");

    std::string content( (std::istreambuf_iterator<char>(configFile) ),
                         (std::istreambuf_iterator<char>()    ) );

    configFile.close();
    ioMutex.unlock();

    Document configs;
    configs.Parse(content.c_str());
    //std::cout<<content<<endl;
    // delete[] buffer;
    //std::cout<<"close config file"<<endl;


    //initialize configurable variables
    //ticker = configs["ticker"].GetString();
    order_type = configs["ordertype"].GetString();
    time_in_force = configs["time_in_force"].GetString();
    trigger = configs["trigger"].GetString();
    chgspd_trg = configs["targetchangespeed"].GetFloat();
    chgdur_trg = configs["targetchangeduration"].GetFloat();
    retTrg = configs["targetreturn"].GetFloat();
    timeWindowForExit = configs["timewindowforexit"].GetInt();

    delay_place_new_order = configs["delay_place_new_order"].GetInt64();
    delay_close_order = configs["delay_close_order"].GetInt64();
    delay_cancel_order = configs["delay_cancel_order"].GetInt64();
    delay_positionstatus_check = configs["delay_positionstatus_check"].GetInt64();
    delay_fetch_marketdata = configs["delay_fetch_marketdata"].GetInt64();
    delay_strategy_check = configs["delay_strategy_check"].GetInt64();

    instrumentId = robinhoodAPI.GetInstrument(ticker_);
    posStatus = ReadyForNewTrade;
}

void PriceMomentumSingleSec::StgSta()
{
}

void PriceMomentumSingleSec::Run()
{
    std::thread strategyMain(&PriceMomentumSingleSec::ExecTrade, this);
    std::thread strategyFetchMktData(&PriceMomentumSingleSec::FetchMktData, this);
    std::thread strategyOpenNewPosition(&PriceMomentumSingleSec::OpenNewPosition, this);
    std::thread strategyCloseLongPosition(&PriceMomentumSingleSec::CloseLongPosition, this);

    strategyMain.join();
    strategyFetchMktData.join();
    strategyOpenNewPosition.join();
    strategyCloseLongPosition.join();

}

void PriceMomentumSingleSec::ExecTrade()
{
    while(mktData.size() <= 2)
    {
        usleep(delay_positionstatus_check);
    }

    string date_benchM = "";
    string date = "";
    string dateTime = "";
    float preHigh = std::numeric_limits<float>::min();
    float preLow = std::numeric_limits<float>::max();
    float curHigh = 0;
    float curLow = 0;
    float price = 0;
    float price_TrendBegin = 0;
    long int preTimeStamp = 0;
    long int timeStamp_trendBegin = 0;
    long int timeStamp;
    long int timeStamp_secondLast;
    long int preCriticalTS = 0;
    long int curCriticalTS = 0;
    CriticalPoint highLow = Neutral;
    string preTrend = "" ;
    string curTrend = "";
    bool sameDay = false;
    long int dur = 0;
    float chgspd = 0.0;
    float prePrice = 0.0;
    float entryOrderPrice = 0;
    float buyingPower = 0;
    //ofstream tradeLog;
    list<MarketDataPoint>::iterator it_first = mktData.begin();
    list<MarketDataPoint>::iterator it_secondLast;
    list<MarketDataPoint>::iterator it_last;
    CriticalPoint highLow_secondLast = Neutral;
    float price_SecondLast = 0.0;

    price_TrendBegin = it_first->GetQuotePrice();
    prePrice = it_first->GetQuotePrice();
    timeStamp_trendBegin = it_first->GetTimeStamp();
    date_benchM = (it_first->GetTimeStr()).substr(0,10);
    preLow = price_TrendBegin;
    preHigh = price_TrendBegin;

    //tradeLog<<timeStamp<<";"<<price_TrendBegin<<";"<<curTrend<<";"<<preTrend<<";"<<preHigh<<";"<<preLow<<";"<<preCriticalTS<<";"<<preTimeStamp<<";"<<price_TrendBegin<<"\n";
    //tradeLog<<std::flush;

    list<MarketDataPoint>::iterator it_second = std::next(it_first);
    if(it_second->GetQuotePrice() >= prePrice)
    {
        preTrend = "UP";
        curTrend = "UP";
        prePrice = it_second->GetQuotePrice();
    }
    else if(it_second->GetQuotePrice() <= prePrice)
    {
        preTrend = "DOWN";
        curTrend = "DOWN";
        prePrice = it_second->GetQuotePrice();
    }
    preTimeStamp = it_second->GetTimeStamp();

    //tradeLog<<it_second->GetTimeStamp()<<";"<<it_second->GetQuotePrice()<<";"<<curTrend<<";"<<preTrend<<";"<<preHigh<<";"<<preLow<<";"<<preCriticalTS<<";"<<preTimeStamp<<";"<<price_TrendBegin<<"\n";
    //tradeLog<<std::flush;

    while(1)
    {
        usleep(delay_strategy_check);
        //auto mkIt = std::prev(mktData.end());
        //MarketDataPoint it = mktData.back();
        it_secondLast = prev(prev(mktData.end()));
        highLow_secondLast = it_secondLast->GetHighLow();
        price_SecondLast = it_secondLast->GetQuotePrice();
        timeStamp_secondLast = it_secondLast->GetTimeStamp();
        if(highLow_secondLast == 1)
        {
            preHigh = price_SecondLast;
            preCriticalTS = timeStamp_secondLast;
        }
        else if(highLow_secondLast == -1)
        {
            preLow = price_SecondLast;
            preCriticalTS = timeStamp_secondLast;
        }
        it_last = std::prev(mktData.end());
        dateTime = it_last->GetTimeStr();
        date = dateTime.substr(0,10);
        timeStamp = it_last->GetTimeStamp();
        highLow = it_last->GetHighLow();
        price = it_last->GetQuotePrice();
        buyingPower = 10;

        if(timeStamp > preTimeStamp)
        {
            //if price is between preHigh and preLow, the curTrend will stay what it is so far.
            if(price >= preHigh)
            {
                curTrend = "UP";
                //curHigh = price;
                //curCriticalTS = timeStamp;
            }
            //else if(highLow == -1 && price > preLow){
            //curTrend = "UP";
            //curLow = price;
            //curCriticalTS = timeStamp;
            //}
            //else if (highLow == 1 && price < preHigh){
            // curTrend = "DOWN";
            // curHigh = price;
            //curCriticalTS = timeStamp;
            // }
            else if (price <= preLow)
            {
                curTrend = "DOWN";
                //curLow = price;
                //curCriticalTS = timeStamp;
            }
            dur = timeStamp - timeStamp_trendBegin;
            chgspd = (price - price_TrendBegin)/price_TrendBegin/float(dur);

            //tradeLog<<timeStamp<<";"<<price<<";"<<curTrend<<";"<<preTrend<<";"<<preHigh<<";"<<preLow<<";"<<preCriticalTS<<";"<<preTimeStamp<<";"<<price_TrendBegin<<"\n";
            //tradeLog<<std::flush;
            //assert(preTrend != "" && curTrend != "");
            if(curTrend == preTrend && curTrend == "UP" && dur > chgdur_trg && chgspd > chgspd_trg && posStatus == ReadyForNewTrade
              ) //&& buyingPower >= price * util::sharesEachTrade
            {
                //OpenNewPosition("buy","limit","gfd","immediate",price);
                side = "buy";
                posStatus = util::CloseLongPosition;
                //orderPrice = price;
                ioMutex.lock();
                tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<timeStamp<<";"<<"Detected"<<";"<<side<<";"<<price<<";"<<util::sharesEachTrade<<"\n";
                tradeLog<<std::flush;
                ioMutex.unlock();
            }
            else if(curTrend == preTrend && curTrend == "DOWN" && dur > chgdur_trg && chgspd < -chgspd_trg)
            {
                //short selling is currently not available
                //entryVect.push_back(std::make_tuple("Short", dateTime, timeStamp, price,false));
                //std::cout<<"here"<<std::endl;
                //tradeLog<<timeStamp<<";"<<"short"<<";"<<price<<util::sharesEachTrade<<"\n";
                //tradeLog<<std::flush;
            }
            //else if(highLow == 0 && dur > chgdur_trg && chgspd > chgspd_trg && posStatus == ReadyForNewTrade && buyingPower >= price * util::sharesEachTrade){
            //OpenNewPosition("buy","limit","gfd","immediate",price);
            //side = "buy";
            //posStatus = ProcessingNewTrade;
            //orderPrice = price;
            //tradeLog<<timeStamp<<";"<<side<<";"<<price<<util::sharesEachTrade<<"\n";
            //tradeLog.close();
            //}
            //else if(highLow == 0 && dur > chgdur_trg && chgspd < -chgspd_trg){
            //short selling is currently not available
            //entryVect.push_back(std::make_tuple("Short", dateTime, timeStamp, price,false));
            //}
            else if(preTrend != curTrend && curTrend == "UP")
            {
                price_TrendBegin = preLow;
                //preHigh = price_TrendBegin;
                timeStamp_trendBegin = preCriticalTS;
                preTrend = curTrend;
            }
            else if(preTrend != curTrend && curTrend == "DOWN")
            {
                price_TrendBegin = preHigh;
                //preLow = price_TrendBegin;
                timeStamp_trendBegin = preCriticalTS;
                preTrend = curTrend;
            }

            //preCriticalTS = curCriticalTS;
            //preHigh = curHigh;
            //preLow = curLow;
            preTimeStamp = timeStamp;
        }
    }
    tradeLog.close();
}

void PriceMomentumSingleSec::OpenNewPosition()
{
    unique_ptr<Document> orderDetail(new Document());
    string orderId = "";
    std::clock_t start;
    clock_t end_;
    double duration = 0;
    string orderState ="";
    std::shared_ptr<Document> quotes;

    while(1)
    {
        usleep(delay_positionstatus_check);

        if(posStatus == ProcessingNewTrade)
        {
            ioMutex.lock();
            tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<"Processing New Trade..."<<"\n";
            tradeLog<<std::flush;
            ioMutex.unlock();
            orderId="";
            orderPrice = 0;
            entryPrice = 0;
            //mutex_.lock();
            //order_type = "limit";
            //time_in_force = "gfd";
            //trigger = "immediate";
            //mutex_.unlock();
            //side = "buy";
            orderDetail.reset();
            quotes.reset();
            try
            {
                quotes = robinhoodAPI.GetQuote(ticker);
                orderPrice = stof(quotes->operator[]("ask_price").GetString());
                orderDetail = robinhoodAPI.PlaceOrder(ticker,util::sharesEachTrade,side,order_type,time_in_force,trigger,orderPrice,instrumentId);
                /** wait 30 seconds for the order to be filled.*/
                usleep(delay_place_new_order);
                orderId = orderDetail->operator[]("id").GetString();
            }
            catch(...)
            {

            }

            //vector<string> orderResponseList;
            //boost::split(orderResponseList, orderResponse, boost::is_any_of(","));
            //string orderId = (orderResponseList[5]).substr(6, 36);;
            start = clock();
            //orderDetail.reset(robinhoodAPI.GetOrderStatus(orderId).get());
// = (orderDetail)->operator[]("state").GetString();



            /** when the program comes here, it is either the order has been filled, or partially filled,
            or timed out for filling the order. For either case, we will send a canceling
            order.
            */
            /**if order is not filled after 10 seconds, cancel the order*/
            //TO do list: what if the order was filled, but there is an exception in this step will
            //will keep entry quant to be zero which would mislead the program to be ready for trades
            //rather than close the existing position. probably make sure RHApiCpp always return
            //correct data.
            try
            {
                orderDetail.reset();
                orderDetail = robinhoodAPI.CancelOrder(orderId);
                orderState = orderDetail->operator[]("state").GetString();
                entryQuant = orderDetail->operator[]("cumulative_quantity").GetDouble();
            }
            catch(...)
            {

            }

            /** if the order has been filled or partially filled, it should signal the program to close
            however many shared filled.
            if no share was filled, signal the program to be ready for new trades.
            */
            if(entryQuant > 0 && side == "buy")
            {
                entryPrice = orderDetail->operator[]("average_price").GetDouble();
                posStatus = util::CloseLongPosition;
                ioMutex.lock();
                tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<" "<<";"<<"Executed"<<";"<<side<<";"<<entryPrice<<";"<<entryQuant<<"\n";
                tradeLog<<std::flush;
                ioMutex.unlock();
            }
            else if (entryQuant > 0 && side == "sell")
            {
                entryPrice = orderDetail->operator[]("average_price").GetDouble();
                posStatus = util::CloseLongPosition;
                ioMutex.lock();
                tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<" "<<";"<<"Executed"<<";"<<side<<";"<<entryPrice<<";"<<entryQuant<<"\n";
                tradeLog<<std::flush;
                ioMutex.unlock();
            }
            else if(entryQuant == 0.0)
            {
                posStatus = ReadyForNewTrade;
            }
        }
    }
}

/** a trade must be filled in order to close a position. This method will keep looping until a trade
has been filled to close a position.*/
void PriceMomentumSingleSec::CloseLongPosition()
{
    while(1)
    {
        usleep(delay_positionstatus_check);
        if(posStatus == util::CloseLongPosition)
        {


            ioMutex.lock();
            tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<"Closing position..."<<"\n";
            tradeLog<<std::flush;
            ioMutex.unlock();
            //mutex_.lock();
            //type = "limit";
            //time_in_force = "gfd";
            //trigger = "immediate";
            exitPrice = 0;
            //mutex_.unlock();

            //sleep for one minute before closing out a position
            //usleep(60000000);
            //SetMktData();
            //auto it = prev(mktData.end());
            //float price = it->GetQuotePrice();
            clock_t tradeStart = clock();
            clock_t tradeEnd;
            float tradeDur = 0;
            int tradeStatus = 0; //a trade could have multiple orders, including those unsuccessful one, canceled ones.
            //string orderResponse;// = robinhoodAPI.PlaceOrder(ticker,util::sharesEachTrade,direction,type,life,delay,price,instrumentId);
            vector<string> orderResponseList;
            //boost::split(orderResponseList, orderResponse, boost::is_any_of(","));
            string orderId;// = (orderResponseList[5]).substr(6, 36);;
            std::clock_t orderStart;// = clock();
            clock_t orderEnd;
            float orderDur = 0;
            string orderState = "";// = robinhoodAPI->GetOrderStatus(orderId);
            std::shared_ptr<Document> quotes(new Document());
            float latestBidPrice = 0.0;
            unique_ptr<Document> orderDetail(new Document());
            float latestFilledQuant = 0;
            float cumulativeFilledQuant = 0;
            float ret = 0;

            /**If the return of the trade has NOT reached the target level and the time elapsed since the
            position was open has NOT exceed the waiting time limit, the program will keep fetching data from the
            market and calculate a new return until either one the two conditions mentioned above breaches. */
            while(ret < retTrg && tradeDur <= timeWindowForExit * CLOCKS_PER_SEC)
            {
                try
                {
                    quotes.reset();
                    quotes = robinhoodAPI.GetQuote(ticker);
                    latestBidPrice = stof(quotes->operator[]("bid_price").GetString());

                    /**calculate return*/
                    ret = (latestBidPrice - entryPrice) / entryPrice;

                    tradeEnd = clock();
                    tradeDur = tradeEnd - tradeStart;
                }
                catch(...) {}

            }
            ioMutex.lock();
            tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<" "<<";"<<"Executed"<<";"<<"Sell"<<";"<<latestBidPrice<<";"<<10<<"\n";
            tradeLog<<std::flush;
            ioMutex.unlock();

            /**When program comes to this step, it is because the observed return is
            higher than or equal to the targeted return; or the time out for filling the order; in either case
            an order will be placed to close the position.
            While loop will keep sending orders until the naked position is covered.*/
            while(cumulativeFilledQuant < entryQuant)
            {
                try
                {
                    orderDetail = robinhoodAPI.PlaceOrder(ticker,entryQuant - cumulativeFilledQuant,"sell",order_type,time_in_force,trigger,latestBidPrice,instrumentId);
                    orderId = orderDetail->operator[]("id").GetString();
                    orderDetail.reset();
                    usleep(delay_close_order);
                    orderDetail = robinhoodAPI.GetOrderStatus(orderId);
                    orderState = orderDetail->operator[]("state").GetString();
                    latestFilledQuant = orderDetail->operator[]("cumulative_quantity").GetDouble();
                    orderStart = clock();

                    /* wait for 10 seconds to fill the order */
                    orderDetail.reset();
                    orderDetail = robinhoodAPI.CancelOrder(orderId);
                    usleep(delay_cancel_order);
                    orderDetail = robinhoodAPI.GetOrderStatus(orderId);
                    latestFilledQuant = orderDetail->operator[]("cumulative_quantity").GetDouble();
                    exitPrice = orderDetail->operator[]("average_price").GetDouble();
                    cumulativeFilledQuant += latestFilledQuant;
//                    ioMutex.lock();
//                    tradeLog<<ticker<<";"<<utility.GetCurrentDateTimeStr()<<";"<<" "<<";"<<"Executed"<<";"<<"Sell"<<";"<<exitPrice<<";"<<latestFilledQuant<<"\n";
//                    tradeLog<<std::flush;
//                    ioMutex.unlock();
                }
                catch(...) {}
                try
                {
                    //fetch latest bid price data
                    quotes.reset(robinhoodAPI.GetQuote(ticker).get());
                    latestBidPrice = quotes->operator[]("bid_price").GetDouble();
                }
                catch(...) {}
            }
            /**When the program comes to this step, all open positions have been closed and
            it is ready for new trades.
            */
            posStatus = ReadyForNewTrade;
            entryQuant = 0;
        }
    }
}

void PriceMomentumSingleSec::FetchMktData()
{

    list<MarketDataPoint>::iterator lastIt;
    list<MarketDataPoint>::iterator secondLastIt;
    float curQuote;
    float lastQuote;
    float secondLastQuote;
    long int curTimeStamp = 0;
    long int lastTimestamp = 0;
    long int secondLastTimestamp = 0;
    long int lastCriticalTimestamp = 0;
    long int criticalPointSpan = 0;
    bool newData = false;
    int mktDataLen = mktData.size();
    int hour = 0;
    int minute = 0;
    int sec  = 0;
    string dateTime = "";
    shared_ptr<Document> quotes(new rapidjson::Document());
    MarketDataPoint dataP;
    //ofstream dataLog;
    //dataLog.open("data.txt", ios::app);
    //std::cout<<"mark"<<std::endl;

    while(1)
    {
        quotes.reset();

        //pythonMutex.lock();
        //std::lock_guard<std::mutex> lock(pythonMutex);
        //std::cout<<ticker<<std::endl;
        quotes = robinhoodAPI.GetQuote(ticker);
        //std::cout<<ticker<<": "<<quotes->operator[]("last_trade_price").GetString()<<std::endl;
        //std::cout<<"land mark"<<std::endl;
        //pythonMutex.unlock();

        mktDataLen = mktData.size();
        curQuote = stof(quotes->operator[]("last_trade_price").GetString());
        dateTime = quotes->operator[]("updated_at").GetString();
        hour = stoi(dateTime.substr(11,2));
        minute = stoi(dateTime.substr(14,2));
        sec = stoi(dateTime.substr(17,2));
        curTimeStamp = hour * 3600 + minute * 60 + sec;

        //If there is already data in the list, not include the new data point if it has the same
        //timestamp as last data point in existing data list.
        if(mktDataLen > 2)
        {
            lastIt  = prev(mktData.end());
            lastTimestamp = lastIt->GetTimeStamp();

            if(curTimeStamp > lastTimestamp + 50)
            {
                newData = true;
                dataP.SetQuotePrice(curQuote);
                dataP.SetTimeStamp(curTimeStamp);
                dataP.SetTimeStr("");
                dataP.SetHighLow(Neutral);
            }
        }
        //If there is no data in the list, always add the first data.
        else if(mktDataLen <= 2 && curTimeStamp > lastTimestamp + 50)
        {
            dataP.SetQuotePrice(curQuote);
            dataP.SetTimeStamp(curTimeStamp);
            dataP.SetTimeStr("");
            dataP.SetHighLow(Neutral);
            mktData.push_back(dataP);
            lastTimestamp = curTimeStamp;
            //dataLog<<ticker<<";"<<curTimeStamp<<";"<<curQuote<<";"<<0<<"\n";
            //dataLog<<std::flush;
        }

        //If there are at least three data points in the market data list,
        //update the high/low of second last data point. The high/low of last
        //data point is always neutral.
        if(mktDataLen > 2 && newData == true)
        {
            secondLastIt = prev(lastIt);
            lastQuote = lastIt->GetQuotePrice();
            secondLastQuote = secondLastIt->GetQuotePrice();
            lastTimestamp = lastIt->GetTimeStamp();
            secondLastTimestamp = secondLastIt->GetTimeStamp();
            criticalPointSpan = lastTimestamp - lastCriticalTimestamp;

            if (curQuote >= lastQuote && lastQuote < secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(Low);
                lastCriticalTimestamp = curTimeStamp;
            }
            else if (curQuote > lastQuote && lastQuote <= secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(Low);
                lastCriticalTimestamp = curTimeStamp;
            }
            else if (curQuote < lastQuote && lastQuote >= secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(High);
                lastCriticalTimestamp = curTimeStamp;
            }
            else if (curQuote <= lastQuote && lastQuote > secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(High);
                lastCriticalTimestamp = curTimeStamp;
            }
            mktData.push_back(dataP);
            //dataLog<<ticker<<";"<<lastTimestamp<<";"<<lastQuote<<";"<<lastIt->GetHighLow()<<"\n";
            //dataLog<<std::flush;
            newData = false;
        }
        //Sleep for 60 seconds before fetching a new market data.
        //Due to Robinhood constraint, only one data request can be made
        //every 40 seconds.

        usleep(delay_fetch_marketdata);
    }
    //dataLog.close();
}

PriceMomentumSingleSec::~PriceMomentumSingleSec()
{
    tradeLog.close();
    //cout<<"********************strategy destroy*************************"<<endl;
}


void PriceMomentumSingleSec::DetectStg() {}
void PriceMomentumSingleSec::Output() {}
void PriceMomentumSingleSec::CleanEntryPoint() {}
void PriceMomentumSingleSec::CleanMktData() {}

void PriceMomentumSingleSec::FetchMktData2()
{
    list<MarketDataPoint>::iterator lastIt;
    list<MarketDataPoint>::iterator secondLastIt;
    float curQuote;
    float lastQuote;
    float secondLastQuote;
    long int curTimeStamp;
    long int lastTimestamp = 0;
    long int secondLastTimestamp;
    long int lastCriticalTimestamp = 0;
    long int criticalPointSpan = 0;
    bool newData = false;
    int mktDataLen = mktData.size();
    int hour = 0;
    int minute = 0;
    int sec  = 0;
    string dateTime = "";
    unique_ptr<Document> quotes(new rapidjson::Document());
    MarketDataPoint dataP;
    string fileName = ticker + "data.txt";
    ofstream dataLog;
    dataLog.open(fileName, ios::app);
    ifstream file;
    file.open("AVP.csv");


    while(!file.eof())
    {
        string line;
        getline(file, line);
        //std::cout<<line<<endl;
        vector<string> lineSplit;
        boost::split(lineSplit, line, boost::is_any_of(","));
        mktDataLen = mktData.size();
        //curQuote = stof(quotes->operator[]("last_trade_price").GetString());
        curQuote = stof(lineSplit[4]);
        //dateTime = quotes->operator[]("updated_at").GetString();
        //hour = stoi(dateTime.substr(11,2));
        //minute = stoi(dateTime.substr(14,2));
        // sec = stoi(dateTime.substr(17,2));
        curTimeStamp = stoi(lineSplit[0]);
        //cout<<curTimeStamp<<endl;

        //If there is already data in the list, not include the new data point if it has the same
        //timestamp as last data point in existing data list.
        if(mktDataLen > 2)
        {
            lastIt  = prev(mktData.end());
            lastTimestamp = lastIt->GetTimeStamp();

            if(curTimeStamp > lastTimestamp)
            {
                //cout<<"here"<<endl;
                newData = true;
                dataP.SetQuotePrice(curQuote);
                dataP.SetTimeStamp(curTimeStamp);
                dataP.SetTimeStr("");
                dataP.SetHighLow(Neutral);
            }
        }
        //If there is no data in the list, always add the first data.
        else if(mktDataLen <= 2)
        {
            dataP.SetQuotePrice(curQuote);
            dataP.SetTimeStamp(curTimeStamp);
            dataP.SetTimeStr("");
            dataP.SetHighLow(Neutral);
            mktData.push_back(dataP);
            //dataLog<<curTimeStamp<<";"<<curQuote<<"\n";
            //dataLog<<std::flush;
            dataLog<<curTimeStamp<<";"<<curQuote<<";"<<0<<"\n";
            dataLog<<std::flush;
        }

        //If there are at least three data points in the market data list,
        //update the high/low of second last data point. The high/low of last
        //data point is always neutral.
        //std::cout<<mktDataLen<<endl;
        if(mktDataLen > 2 && newData == true)
        {
            //std::cout<<123<<endl;
            secondLastIt = prev(lastIt);
            lastQuote = lastIt->GetQuotePrice();
            secondLastQuote = secondLastIt->GetQuotePrice();
            lastTimestamp = lastIt->GetTimeStamp();
            secondLastTimestamp = secondLastIt->GetTimeStamp();
            criticalPointSpan = lastTimestamp - lastCriticalTimestamp;

            if (curQuote >= lastQuote && lastQuote < secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(Low);
                lastCriticalTimestamp = curTimeStamp;
            }
            else if (curQuote > lastQuote && lastQuote <= secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(Low);
                lastCriticalTimestamp = curTimeStamp;
            }
            else if (curQuote < lastQuote && lastQuote >= secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(High);
                lastCriticalTimestamp = curTimeStamp;
            }
            else if (curQuote <= lastQuote && lastQuote > secondLastQuote && criticalPointSpan > 50)
            {
                lastIt->SetHighLow(High);
                lastCriticalTimestamp = curTimeStamp;
            }

            //dataLog<<curTimeStamp<<";"<<curQuote<<"\n";
            // dataLog<<std::flush;
            dataLog<<lastTimestamp<<";"<<lastQuote<<";"<<lastIt->GetHighLow()<<"\n";
            dataLog<<std::flush;
            mktData.push_back(dataP);
        }
        //Sleep for 60 seconds before fetching a new market data.
        //Due to Robinhood constraint, only one data request can be made
        //every 3 seconds.
        usleep(10000000);
    }
    dataLog.close();
}
