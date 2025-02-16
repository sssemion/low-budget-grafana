#include "tsdb.h"
#include <curl/curl.h>
#include <stdexcept>

const char *InvalidTSDBRequest::what() const noexcept
{
    return message.c_str();
}

std::string TSDBClient::format_line_name(const std::string &name, const std::map<std::string, std::string> &labels)
{
    std::string res = name;
    if (labels.empty())
        return res;
    res += '[';
    for (auto it = labels.begin(); it != labels.end(); it++) {
        res += it->first + '=' + it->second + ';';
    }
    res[res.length() - 1] = ']';
    return res;
}

std::string TSDBClient::performHttpRequest(const std::string &url, int timeout)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
    }

    return response_data;
}

size_t TSDBClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response_data = static_cast<std::string*>(userp);
    response_data->append(static_cast<char*>(contents), total_size);
    return total_size;
}
