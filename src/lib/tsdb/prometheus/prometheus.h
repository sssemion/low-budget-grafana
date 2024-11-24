#ifndef TSDB_PROMETHEUS_H
#define TSDB_PROMETHEUS_H

#include "../tsdb.h"

/**
 * @brief Клиент к Prometheus
 */
class PrometheusClient : public TSDBClient {
public:
    /**
     * @brief Конструктор клиента Prometheus.
     *
     * @param base_url Базовый URL Prometheus API (например, "http://localhost:9090")
     */
    explicit PrometheusClient(const std::string& base_url) : base_url(base_url) {}

    inline virtual std::vector<Metric> query(const std::string& query_str, Timestamp start, Timestamp end) override {
        return query(query_str, start, end, 15); // default step = 15
    };

    /**
     * @brief Выполнить запрос к Prometheus для получения данных.

     * @param query Строка запроса в формате PromQL
     * @param start Начало временного диапазона
     * @param end Конец временного диапазона
     * @param step Интервал между точками в секундах, по умолчанию 15
     * @return Массив метрик типа Metric
     */
    virtual std::vector<Metric> query(const std::string& query_str, Timestamp start, Timestamp end, int step);

    /**
     * @brief Проверить доступность Prometheus.
     *
     * @return true, если Prometheus доступен, иначе false
     */
    bool isAvailable() override;


protected:
    std::string base_url; // Базовый URL для API Prometheus
};


#endif // TSDB_PROMETHEUS_H
