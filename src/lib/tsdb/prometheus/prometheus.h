/**
 * @file prometheus.h
 * @brief Реализация интерфейса для работы с Prometheus.
 *
 * @details Этот заголовочный файл содержит класс `PrometheusClient`, который является реализацией
 *          интерфейса `TSDBClient` для взаимодействия с Prometheus.
 *
 *          Класс предоставляет методы для и чтения метрик из Prometheus и проверки доступности сервера с
 *          использованием HTTP API.
 */

/**
 * @defgroup prometheus Клиент к Prometheus
 * @ingroup tsdb
 * @brief Клиент для взаимодействия с Prometheus.
 *
 * @details  модуль включает `PrometheusClient`, реализующий интерфейс `TSDBClient`, для работы с Prometheus.
 *           Реализация включает методы для запроса метрик и проверки доступности сервера через HTTP API.
 */
/** @{ */

#ifndef TSDB_PROMETHEUS_H
#define TSDB_PROMETHEUS_H

#include <ctime>

#include "../tsdb.h"

/**
 * @brief Исключение, возникающее при ошибке запроса к Prometheus.
 */
class InvalidPrometheusRequest : public InvalidTSDBRequest
{
public:
    /**
     * @brief Конструктор исключения.
     *
     * @param errorMsg Сообщение об ошибке.
     * @param errorType Тип ошибки.
     */
    InvalidPrometheusRequest(const std::string &errorMsg, const std::string &errorType);
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

    /**
     * @brief Выполнить запрос к Prometheus для получения данных.
     *
     * @param query_str Строка запроса в формате PromQL
     * @param start Начало временного диапазона
     * @param end Конец временного диапазона
     * @return Массив метрик типа Metric
     * @throws InvalidPrometheusRequest В случае неуспешного статуса ответа от Prometheus
     */
    std::vector<Metric> query(const std::string &query_str, std::time_t start, std::time_t end) override;

    /**
     * @brief Выполнить запрос к Prometheus с шагом между точками.
     *
     * @param query_str Строка запроса в формате PromQL
     * @param start Начало временного диапазона
     * @param end Конец временного диапазона
     * @param step Интервал между точками в секундах, по умолчанию 15
     * @return Массив метрик типа Metric
     * @throws InvalidPrometheusRequest В случае неуспешного статуса ответа от Prometheus
     */
    std::vector<Metric> query(const std::string &query_str, std::time_t start, std::time_t end, int step);

    /**
     * @brief Проверить доступность Prometheus.
     *
     * @return true, если Prometheus доступен, иначе false
     */
    bool isAvailable() noexcept override;

protected:
    std::string base_url;

    /**
     * @brief Распарсить ответ от Prometheus.
     *
     * @param response JSON-ответ от Prometheus
     * @return Вектор метрик
     */
    std::vector<Metric> parse_response(const std::string &response);
};

/** @} */

#endif // TSDB_PROMETHEUS_H
