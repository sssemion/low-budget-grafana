#include <curl/curl.h>
#include "prometheus.h"

PrometheusClient::PrometheusClient(const std::string& base_url) : base_url(base_url) {}

std::vector<Metric> PrometheusClient::query(const std::string &query_str, Timestamp start, Timestamp end)
{
    return query(query_str, start, end, 15); // default step = 15
}

std::vector<Metric> PrometheusClient::query(const std::string &query_str, Timestamp start, Timestamp end, int step)
{
    std::string url = base_url + "/api/v1/query_range" + "?query=" + curl_escape(query_str.c_str(), query_str.length());
    url += "&start=" + std::to_string(start);
    url += "&end=" + std::to_string(end);
    url += "&step=" + std::to_string(step);
    std::string response = performHttpRequest(url);
    return std::vector<Metric>{};
}

bool PrometheusClient::isAvailable()
{
    try {
        std::string url = base_url + "/-/healthy";
        std::string response = performHttpRequest(url);
        return response == "Prometheus Server is Healthy.";
    } catch (...) {
        return false;
    }
}
