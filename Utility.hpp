#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include <ctime>

namespace util
{
    extern float rhSellingFee; //selling fees from Robinhood
    extern int sharesEachTrade;

    enum PositionStatus
    {
        ReadyForNewTrade = 1,
        ProcessingNewTrade = 2,
        CloseLongPosition = 3,
        CloseShortPosition = 4,
    };

    class Utility
    {
    private:
        time_t currentTimeT;
        struct tm currentTimeStruct;

public:
    std::string GetCurrentDateTimeStr();
    };

}

#endif // UTILITY_HPP
