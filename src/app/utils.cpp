
#include <iomanip>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <sstream>
#include <vector>

#include "utils.h"
#include "implot.h"

int timeTickFormatter(double value, char *buff, int size, void *user_data)
{
    using namespace std::chrono;

    ImPlotRange r = ImPlot::GetPlotLimits().X;
    seconds range{(long long)r.Size()};

    std::time_t time = static_cast<std::time_t>(value);
    std::tm *tm = std::localtime(&time);

    if (!tm)
        return 0;

    const char *format;
    if (range < 1h)
        format = "%H:%M:%S";
    else if (range < 24h)
        format = "%H:%M";
    else if (range < 7 * 24h)
        format = "%a %Hh";
    else if (range < 30 * 24h)
        format = "%d %b";
    else if (range < 365 * 24h)
        format = "%Y-%m-%d";
    else
        format = "%b'%y";

    std::strftime(buff, size, format, tm);

    return static_cast<int>(std::strlen(buff));
}

std::string formatTimestamp(std::time_t timestamp)
{
    std::tm* tm = std::localtime(&timestamp);
    if (!tm) {
        return "";
    }

    std::ostringstream oss;
    oss << std::put_time(tm, "%c");
    return oss.str();
}
