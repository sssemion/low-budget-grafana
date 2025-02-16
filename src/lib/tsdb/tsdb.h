#ifndef TSDB_ABC_H
#define TSDB_ABC_H

#include <ctime>
#include <map>
#include <string>
#include <vector>

/**
 * @brief Исключение, возникающее при ошибке запроса к TSDB.
 */
class InvalidTSDBRequest : public std::exception
{
public:
    virtual ~InvalidTSDBRequest() = default;

    /**
     * @brief Получить описание ошибки.
     *
     * @return Строка с описанием ошибки.
     */
    virtual const char *what() const noexcept override;

protected:
    std::string message;
};

/**
 * @brief Описывает одну точку данных временного ряда.
 *
 * Содержит значение и временную метку, соответствующую этой точке.
 */
struct Point
{
    double value;
    std::time_t timestamp;
};

/**
 * @brief Описывакт метрику, содержащую временной ряд данных.
 *
 * Метрика включает имя, набор меток и значения, представленные точками временного ряда.
 */
struct Metric
{
    std::string name;
    std::map<std::string, std::string> labels;
    std::vector<Point> values;
};

/**
 * @brief Интерфейс клиента к TSDB
 */
class TSDBClient
{
public:
    virtual ~TSDBClient() = default;

    /**
     * @brief Выполнить запрос к TSDB для получения точек.
     *
     * @param query Строка запроса
     * @param start Начало временного диапазона
     * @param end Конец временного диапазона
     * @return Массив метрик типа Metric
     * @throws InvalidTSDBRequest В случае неуспешного статуса ответа от TSDB
     */
    virtual std::vector<Metric> query(const std::string &query_str, std::time_t start, std::time_t end) = 0;

    /**
     * @brief Проверить доступность TSDB.
     *
     * @return true, если TSDB доступна, иначе false
     */
    virtual bool isAvailable() noexcept = 0;

    /**
     * @brief Форматирует имя линии графика на основе метрики.
     *
     * @param metric Метрика типа Metric.
     * @return Отформатированное имя линии графика с метками.
     */
    static std::string format_line_name(const Metric &metric) { return format_line_name(metric.name, metric.labels); };

    /**
     * @brief Форматирует имя линии графика на основе имени и меток.
     *
     * @param name Имя метрики.
     * @param labels Словарь меток метрики, где ключ — имя метки, а значение — её значение.
     * @return Отформатированное имя линии графика с метками.
     */
    static std::string format_line_name(const std::string &name, const std::map<std::string, std::string> &labels);

protected:
    /**
     * @brief Выполнить HTTP-запрос и получить результат.
     *
     * @param url URL для запроса
     * @return Ответ от сервера
     */
    virtual std::string performHttpRequest(const std::string &url, int timeout = 5);

    /**
     * @brief Callback для записи данных из CURL.
     *
     * @param contents Данные, полученные из запроса
     * @param size Размер одного блока
     * @param nmemb Количество блоков
     * @param userp Пользовательские данные для записи
     * @return Количество байт, записанных в userp
     */
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif // TSDB_ABC_H
