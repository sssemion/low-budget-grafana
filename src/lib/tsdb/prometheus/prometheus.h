#ifndef TSDB_PROMETHEUS_H
#define TSDB_PROMETHEUS_H

#include <ctime>

#include "../tsdb.h"

class InvalidPrometheusRequest : public std::exception
{
public:
    InvalidPrometheusRequest(const std::string &errorMsg, const std::string &errorType);

    const char *what() const noexcept override;

private:
    std::string message;
};

/**
 * @brief Клиент к Prometheus
 */
class PrometheusClient : public TSDBClient
{
public:
    /**
     * @brief Конструктор клиента Prometheus.
     *
     * @param base_url Базовый URL Prometheus API (например, "http://localhost:9090")
     */
    explicit PrometheusClient(const std::string &base_url);
    ~PrometheusClient() override = default;

    std::vector<Metric> query(const std::string &query_str, std::time_t start, std::time_t end) override;

    /**
     * @brief Выполнить запрос к Prometheus для получения данных.

     * @param query Строка запроса в формате PromQL
     * @param start Начало временного диапазона
     * @param end Конец временного диапазона
     * @param step Интервал между точками в секундах, по умолчанию 15
     * @return Массив метрик типа Metric
     */
    std::vector<Metric> query(const std::string &query_str, std::time_t start, std::time_t end, int step);

    /**
     * @brief Проверить доступность Prometheus.
     *
     * @return true, если Prometheus доступен, иначе false
     */
    bool isAvailable() override;


protected:
    std::string base_url; // Базовый URL для API Prometheus

    std::vector<Metric> parse_response(const std::string &response);
};

#endif // TSDB_PROMETHEUS_H
