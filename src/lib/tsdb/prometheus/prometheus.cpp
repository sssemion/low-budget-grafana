#include <curl/curl.h>
#include "prometheus.h"
#include <nlohmann/json.hpp>

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
    return parse_response(response);
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

std::vector<Metric> PrometheusClient::parse_response(const std::string &response)
{
    std::vector<Metric> metrics;

    auto parsed_json = nlohmann::json::parse(response);

    if (parsed_json["status"] != "success") {
        throw std::runtime_error("Prometheus request failed");
    }

    for (const auto& result : parsed_json["data"]["result"]) {
        Metric metric;

        auto metric_info = result["metric"];
        for (auto it = metric_info.begin(); it != metric_info.end(); ++it) {
            if (it.key() == "__name__") {
                metric.name = it.value();
            } else {
                metric.labels[it.key()] = it.value();
            }
        }

        for (const auto& value_pair : result["values"]) {
            Point point;
            point.timestamp = value_pair[0];
            point.value = std::stod((std::string)value_pair[1]);
            metric.values.push_back(point);
        }

        metrics.push_back(metric);
    }
    return metrics;
}
