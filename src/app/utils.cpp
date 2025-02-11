
#include <iomanip>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <sstream>
#include <vector>

#include "utils.h"
#include "constants.h"
#include "implot.h"

constexpr std::pair<int, const char *> TIME_UNITS[] = {
    {60, "m"},
    {60, "h"},
    {24, "d"},
};

constexpr const char *SIZE_UNITS[] = {"KiB", "MiB", "GiB", "TiB"};

constexpr int K = 1 << 10;

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

int valueTickFormatter(double value, char *buff, int size, void *user_data)
{
    std::string unit = "";
    std::string value_format = "%.2f";
    switch (*(YAxisUnit *)user_data)
    {
    case YAxisUnit::No:
        break;
    case YAxisUnit::Seconds:
        unit = "B";
        for (auto &it : TIME_UNITS)
        {
            if (abs(value) < it.first)
                break;
            value /= it.first;
            unit = it.second;
        }
        break;
    case YAxisUnit::Bytes:
        unit = "B";
        value_format = "%.0f";
        for (auto &it : SIZE_UNITS)
        {
            if (abs(value) < K)
                break;
            value /= K;
            unit = it;
        }
        break;
    case YAxisUnit::Percents:
        value *= 100;
        unit = "%";
        break;
    }
    value_format += " %s";
    std::snprintf(buff, sizeof(buff) - 1, value_format.c_str(), value, unit.c_str());
    return std::strlen(buff);
}
