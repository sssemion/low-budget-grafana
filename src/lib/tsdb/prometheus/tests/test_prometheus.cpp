#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../prometheus.h"
#include <stdexcept>

const Timestamp TEST_START = 1732448700, TEST_END = 1732449600;
const int TEST_STEP = 15;
const std::string TEST_MULTUPLE_METRICS = R"(
{
    "status": "success",
    "data": {
        "resultType": "matrix",
        "result": [
            {
                "metric": {
                    "__name__": "test_metric",
                    "exported_job": "test_job"
                },
                "values": [
                    [
                        1738707300,
                        "42"
                    ],
                    [
                        1738709100,
                        "442"
                    ],
                    [
                        1738710900,
                        "442"
                    ]
                ]
            },
            {
                "metric": {
                    "__name__": "test_metric",
                    "exported_job": "test_job_2"
                },
                "values": [
                    [
                        1738710900,
                        "37"
                    ]
                ]
            }
        ]
    }
}
)";

class MockPrometheusClient : public PrometheusClient
{
public:
    MockPrometheusClient(const std::string &base_url) : PrometheusClient(base_url) {};

    void mockRequest(const std::string &expected_url, const std::string &response)
    {
        this->expected_url = expected_url;
        this->response = response;
        ready = true;
    };

protected:
    std::string performHttpRequest(const std::string &url) override
    {
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

TEST_SUITE("Test PrometheusClient")
{
    MockPrometheusClient client("http://test.host");

    TEST_CASE("Test query")
    {
        SUBCASE("Валидный запрос")
        {
            client.mockRequest(
                "/api/v1/query_range?query=test&start=" + std::to_string(TEST_START) + "&end=" + std::to_string(TEST_END) + "&step=" + std::to_string(TEST_STEP),
                R"({"status":"success","data":{"result":[{"metric":{"__name__":"up"},"values":[[1690,"1"]]}]}})");
            std::vector<Metric> result = client.query("test", TEST_START, TEST_END, TEST_STEP);
            CHECK(result.size() == 1);
            CHECK(result[0].name == "up");
            CHECK(result[0].labels.empty());
            CHECK(result[0].values.size() == 1);
            CHECK(result[0].values[0].timestamp == 1690);
            CHECK(result[0].values[0].value == 1);
        }

        SUBCASE("Несколько метрик")
        {
            client.mockRequest(
                "/api/v1/query_range?query=test_metric&start=" + std::to_string(TEST_START) + "&end=" + std::to_string(TEST_END) + "&step=" + std::to_string(TEST_STEP),
                TEST_MULTUPLE_METRICS);
            std::vector<Metric> result = client.query("test_metric", TEST_START, TEST_END, TEST_STEP);
            CHECK(result.size() == 2);
            CHECK(result[0].name == "test_metric");
            CHECK(result[1].name == "test_metric");
            CHECK(result[0].labels["exported_job"] == "test_job");
            CHECK(result[1].labels["exported_job"] == "test_job_2");
            CHECK(result[0].values.size() == 3);
            CHECK(result[1].values.size() == 1);
        }

        SUBCASE("Ошибка")
        {
            client.mockRequest(
                "/api/v1/query_range?query=test&start=" + std::to_string(TEST_START) + "&end=" + std::to_string(TEST_END) + "&step=" + std::to_string(TEST_STEP),
                R"({"status":"failure"})");
            CHECK_THROWS_AS_MESSAGE(
                std::vector<Metric> result = client.query("test", TEST_START, TEST_END, TEST_STEP),
                std::runtime_error,
                "Prometheus request failed");
        }

        SUBCASE("Дефолтный step")
        {
            client.mockRequest(
                "/api/v1/query_range?query=test&start=" + std::to_string(TEST_START) + "&end=" + std::to_string(TEST_END) + "&step=15",
                R"({"status":"success","data":{"result":[{"metric":{"__name__":"up"},"values":[[1690,"1"]]}]}})");
            std::vector<Metric> result = client.query("test", TEST_START, TEST_END);
        }
    }

    TEST_CASE("Test isAvailable")
    {
        SUBCASE("Available")
        {
            client.mockRequest("/-/healthy", "Prometheus Server is Healthy.");
            CHECK(client.isAvailable());
        }

        SUBCASE("Not ready")
        {
            client.mockRequest("/-/healthy", "200 NOT_READY");
            CHECK_FALSE(client.isAvailable());
        }

        SUBCASE("Not available")
        {
            // Отсутствие мока приводит к ошибке - имитируем ошибку соединения
            CHECK_FALSE(client.isAvailable());
        }
    }
}
