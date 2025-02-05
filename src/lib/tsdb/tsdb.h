#ifndef TSDB_ABC_H
#define TSDB_ABC_H

#include <string>
#include <vector>
#include <map>

using Timestamp = unsigned long long;

struct Point {
    double value;
    Timestamp timestamp;
};

struct Metric {
    std::string name;
    std::map<std::string, std::string> labels;
    std::vector<Point> values;
};

/**
 * @brief Интерфейс клиента к TSDB
 */
class TSDBClient {
public:
    virtual ~TSDBClient() = default;

    /**
     * @brief Выполнить запрос к TSDB для получения точек.
     *
     * @param query Строка запроса
     * @param start Начало временного диапазона
     * @param end Конец временного диапазона
     * @return Массив метрик типа Metric
     */
    virtual std::vector<Metric> query(const std::string& query_str, Timestamp start, Timestamp end) = 0;

    /**
     * @brief Проверить доступность TSDB.
     *
     * @return true, если TSDB доступна, иначе false
     */
    virtual bool isAvailable() = 0;

protected:

    /**
     * @brief Выполнить HTTP-запрос и получить результат.
     *
     * @param url URL для запроса
     * @return Ответ от сервера
     */
    virtual std::string performHttpRequest(const std::string& url);

    /**
     * @brief Callback для записи данных из CURL.
     *
     * @param contents Данные, полученные из запроса
     * @param size Размер одного блока
     * @param nmemb Количество блоков
     * @param userp Пользовательские данные для записи
     * @return Количество байт, записанных в userp
     */
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // TSDB_ABC_H
