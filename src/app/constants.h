#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

// Оконные параметры
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr int SETTINGS_WIDTH = 400;

// OpenGL параметры
constexpr const char *GLSL_VERSION = "#version 410";

// Параметры по умолчанию
constexpr const char *DEFAULT_PROMETHEUS_URL = "http://84.201.168.241:9090";
constexpr const char *DEFAULT_QUERY = "process_resident_memory_bytes";
constexpr int DEFAULT_FETCH_RANGE = 3600;         // Диапазон запроса данных в секундах (1ч)
constexpr float DEFAULT_REFRESH_INTERVAL = 60;    // Интервал автообновления (1м)
constexpr int DEFAULT_PLOT_TIME_RANGE = 2 * 3600; // Дефолтное окно просмотра (2ч)

// Остальные параметры
constexpr double MIN_X_ZOOM = 1e-3, MAX_X_ZOOM = 3600 * 24 * 30 * 3; // from 1ms to 3M
constexpr double MIN_Y_ZOOM = 1e-6, MAX_Y_ZOOM = 1e12;
constexpr double DEFAULT_MIN_Y = 0, DEFAULT_MAX_Y = 1;
constexpr int AUTO_REFRESH_INTERVAL_STEP = 5, AUTO_REFRESH_INTERVAL_MIN = 5;

namespace Strings
{
    constexpr const char *WINDOW_TITLE = "Low budget grafana";
    constexpr const char *WINDOW_METRICS_VIEWER = "Metrics Viewer";
    constexpr const char *PLOT_TITLE = "Time Series";
    constexpr const char *AXIS_TIME = "Time";
    constexpr const char *AXIS_VALUE = "Value";

    constexpr const char *WINDOW_SETTINGS = "Settings";
    constexpr const char *NODE_CONNECTION = "Connection";
    constexpr const char *NODE_QUERY = "Query";
    constexpr const char *NODE_PLOT_SETTINGS = "Plot settings";
    constexpr const char *NODE_TIME_INTERVALS = "Time intervals";

    constexpr const char *LABEL_PROMETHEUS_URL = "Prometheus Base URL:";
    constexpr const char *LABEL_QUERY = "PromQL Query:";
    constexpr const char *LABEL_PLOT_TYPE = "Plot Type:";
    constexpr const char *LABEL_AUTO_REFRESH = "Auto Refresh";

    constexpr const char *BUTTON_CONNECT = "Connect";
    constexpr const char *BUTTON_FETCH_DATA = "Fetch Data";

    constexpr const char *RADIO_BUTTON_LINE = "Line";
    constexpr const char *RADIO_BUTTON_SCATTER = "Scatter";
    constexpr const char *RADIO_BUTTON_BAR = "Bar";

    constexpr const char *MESSAGE_CONNECTION_SUCCESS = "Connection successful!";
    constexpr const char *MESSAGE_CONNECTION_FAILED = "Failed to connect to Prometheus.";
}

enum class PlotType
{
    Line,
    Scatter,
    Bar,
};

enum class YAxisUnit
{
    No,
    Seconds,
    Bytes,
    Percents,
};

constexpr const char *Y_AXIS_UNIT_LABELS[] = {"Count", "Seconds", "Bytes", "Percent"};

#endif // APP_CONSTANTS_H
