
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

#include "utils.h"
#include "implot.h"

int timeTickFormatter(double value, char *buff, int size, void *user_data)
{
    using namespace std::chrono;

    ImPlotRange range = ImPlot::GetPlotLimits().X;
    seconds rangeSize{(long long)range.Size()};

    std::time_t time = static_cast<std::time_t>(value);
    std::tm *tm = std::localtime(&time);

    if (!tm)
        return 0;

    const char *format;
    if (rangeSize < 1h)
        format = "%H:%M:%S";
    else if (rangeSize < 24h)
        format = "%H:%M";
    else if (rangeSize < 7 * 24h)
        format = "%a %Hh";
    else if (rangeSize < 30 * 24h)
        format = "%d %b";
    else if (rangeSize < 365 * 24h)
        format = "%Y-%m-%d";
    else
        format = "%b'%y";

    std::strftime(buff, size, format, tm);

    return static_cast<int>(std::strlen(buff));
}
