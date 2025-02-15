
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
#include "imgui_internal.h"

constexpr std::pair<int, const char *> TIME_UNITS[] = {
    {60, "m"},
    {60, "h"},
    {24, "d"},
};

constexpr const char *SIZE_UNITS[] = {"KiB", "MiB", "GiB", "TiB"};

constexpr int K = 1 << 10;

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
        unit = "s";
        for (auto &it : TIME_UNITS)
        {
            if (abs(value) < it.first)
                break;
            value /= it.first;
            unit = it.second;
        }
        break;
    case YAxisUnit::Bytes:
        unit = " B";
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
        value_format = "%.0f";
        break;
    }

    if (abs(value) > 1e12)
        value_format = "%.6e";
    value_format += "%s";
    return ImFormatString(buff, 15, value_format.c_str(), value, unit.c_str());
}
