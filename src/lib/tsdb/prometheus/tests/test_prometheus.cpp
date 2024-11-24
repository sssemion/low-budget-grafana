#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../prometheus.h"
#include <stdexcept>

const Timestamp TEST_START = 1732448700, TEST_END = 1732449600;
const int TEST_STEP = 15;

class MockPrometheusClient : public PrometheusClient {
public:
    MockPrometheusClient(const std::string& base_url) : PrometheusClient(base_url) {};

    void mockRequest(const std::string& expected_url, const std::string& response) {
        this->expected_url = expected_url;
        this->response = response;
        ready = true;
    };

protected:
    std::string performHttpRequest(const std::string& url) override {
        if (!ready)
            throw std::runtime_error("Перед тестом необходимо замокать запрос через `mockRequest`");
        ready = false;
        if (url == base_url + expected_url)
            return response;
        throw std::runtime_error("Попытка запроса по незарегистрированному url: " + url);
    }

private:
    std::string expected_url, response;
    bool ready;
};

TEST_SUITE("Test PrometheusClient") {
    MockPrometheusClient client("http://test.host");

    TEST_CASE("Test query") {
        SUBCASE("Валидный запрос") {
            client.mockRequest(
                "/api/v1/query?query=test&start=" + std::to_string(TEST_START)
                    + "&end=" + std::to_string(TEST_END)
                    + "&step=" + std::to_string(TEST_STEP),
                R"({"status":"success","data":{"result":[{"metric":{"__name__":"up"},"value":[1690,"1"]}]}})"
            );
            std::vector<Metric> result = client.query("test", TEST_START, TEST_END, TEST_STEP);
            CHECK(result.size() ==  std::vector<Metric>{}.size());
        }
    }

    TEST_CASE("Test isAvailable") {
        SUBCASE("Available") {
            client.mockRequest("/-/healthy", "200 OK");
            CHECK(client.isAvailable());
        }

        SUBCASE("Not ready") {
            client.mockRequest("/-/healthy", "200 NOT_READY");
            CHECK_FALSE(client.isAvailable());
        }

        SUBCASE("Not available") {
            // Отсутствие мока приводит к ошибке - имитируем ошибку соединения
            CHECK_FALSE(client.isAvailable());
        }
    }
}
