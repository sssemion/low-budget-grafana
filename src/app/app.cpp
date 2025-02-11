#define GL_SILENCE_DEPRECATION

#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <limits>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "../lib/tsdb/prometheus/prometheus.h"
#include "constants.h"
#include "utils.h"

struct GraphSeries
{
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
};

static bool autoRefresh = false;
static double lastRefreshTime = 0.0;
static float refreshIntervalSec = DEFAULT_REFRESH_INTERVAL;
static std::string queryStr = DEFAULT_QUERY;
static std::unique_ptr<PrometheusClient> prometheusClient = nullptr;
static double leftTimeBound = static_cast<double>(std::time(nullptr)) - DEFAULT_PLOT_TIME_RANGE;
static double rightTimeBound = static_cast<double>(std::time(nullptr));
static std::string connectionMessage;
static bool showConnectionMessage = false;

static PlotType currentPlotType = PlotType::Line;

YAxisUnit currentYAxisUnit = YAxisUnit::No;

static std::vector<GraphSeries> seriesData;

void fetchData()
{
    if (!prometheusClient)
        return;

    std::time_t now = std::time(nullptr);
    std::time_t start = now - DEFAULT_FETCH_RANGE;
    std::time_t end = now;

    std::vector<Metric> metrics = prometheusClient->query(queryStr, start, end);

    seriesData.clear();

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto &m : metrics)
    {
        for (const auto &p : m.values)
        {
            if (p.value < minY)
                minY = p.value;
            if (p.value > maxY)
                maxY = p.value;
        }
    }
    double yMargin = std::max((maxY - minY) * 0.1, 1.0);
    minY -= yMargin;
    maxY += yMargin;
    ImPlot::SetNextAxisLimits(ImAxis_Y1, minY, maxY, ImPlotCond_Always);

    for (auto &m : metrics)
    {
        GraphSeries s;
        s.name = prometheusClient->format_line_name(m);
        for (auto &p : m.values)
        {
            s.x.push_back(static_cast<double>(p.timestamp));
            s.y.push_back(p.value);
        }
        seriesData.push_back(s);
    }
}

void renderMetricsViewer()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - SETTINGS_WIDTH, WINDOW_HEIGHT), ImGuiCond_Always);
    ImGui::Begin("Metrics Viewer", nullptr, ImGuiWindowFlags_NoDecoration);

    ImVec2 plotSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
    if (ImPlot::BeginPlot("Time Series", plotSize, ImPlotFlags_NoTitle))
    {
        ImPlot::SetupAxisLimits(ImAxis_Y1, DEFAULT_MIN_Y, DEFAULT_MAX_Y);
        ImPlot::SetupAxisLimits(ImAxis_X1, leftTimeBound, rightTimeBound);
        ImPlot::SetupAxes("Time", "Value");
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, HUGE_VAL);
        ImPlot::SetupAxisFormat(ImAxis_X1, timeTickFormatter, nullptr);
        ImPlot::SetupAxisFormat(ImAxis_Y1, valueTickFormatter, &currentYAxisUnit);

        // Обновляем границы времени при изменении масштаба в графике
        ImPlotRange range = ImPlot::GetPlotLimits().X;
        leftTimeBound = range.Min;
        rightTimeBound = range.Max;

        for (auto &s : seriesData)
        {
            if (s.x.empty())
                continue;
            if (currentPlotType == PlotType::Line)
            {
                ImPlot::PlotLine(s.name.c_str(), s.x.data(), s.y.data(), s.x.size());
            }
            else if (currentPlotType == PlotType::Scatter)
            {
                ImPlot::PlotScatter(s.name.c_str(), s.x.data(), s.y.data(), s.x.size());
            }
            else if (currentPlotType == PlotType::Bar)
            {
                double barWidth = 0.5;
                ImPlot::PlotBars(s.name.c_str(), s.x.data(), s.y.data(), s.x.size(), barWidth);
            }
        }

        ImPlot::EndPlot();
    }
    ImGui::End();
}

void renderSettings()
{
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - SETTINGS_WIDTH, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(SETTINGS_WIDTH, WINDOW_HEIGHT), ImGuiCond_Always);
    ImGui::Begin(Strings::WINDOW_SETTINGS, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (ImGui::TreeNode(Strings::NODE_CONNECTION))
    {
        ImGui::Text(Strings::LABEL_PROMETHEUS_URL);
        static char urlBuffer[128];
        std::strncpy(urlBuffer, DEFAULT_PROMETHEUS_URL, sizeof(urlBuffer) - 1);
        urlBuffer[sizeof(urlBuffer) - 1] = '\0';
        ImGui::InputText("##BaseURL", urlBuffer, IM_ARRAYSIZE(urlBuffer));

        if (ImGui::Button(Strings::BUTTON_CONNECT))
        {
            prometheusClient = std::make_unique<PrometheusClient>(urlBuffer);
            if (prometheusClient->isAvailable())
            {
                connectionMessage = Strings::MESSAGE_CONNECTION_SUCCESS;
            }
            else
            {
                connectionMessage = Strings::MESSAGE_CONNECTION_FAILED;
            }
            showConnectionMessage = true;
        }

        if (showConnectionMessage)
        {
            ImGui::TextWrapped("%s", connectionMessage.c_str());
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    if (ImGui::TreeNode(Strings::NODE_QUERY))
    {
        ImGui::Text(Strings::LABEL_QUERY);
        static char queryBuffer[255] = {}; // Инициализация пустого буфера
        queryBuffer[sizeof(queryBuffer) - 1] = '\0';
        ImGui::InputText("##QueryStr", queryBuffer, IM_ARRAYSIZE(queryBuffer));
        queryStr = queryBuffer;

        if (ImGui::Button(Strings::BUTTON_FETCH_DATA))
        {
            fetchData();
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    if (ImGui::TreeNode(Strings::NODE_PLOT_SETTINGS))
    {
        ImGui::Text(Strings::LABEL_PLOT_TYPE);
        if (ImGui::RadioButton(Strings::RADIO_BUTTON_LINE, currentPlotType == PlotType::Line))
        {
            currentPlotType = PlotType::Line;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(Strings::RADIO_BUTTON_SCATTER, currentPlotType == PlotType::Scatter))
        {
            currentPlotType = PlotType::Scatter;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(Strings::RADIO_BUTTON_BAR, currentPlotType == PlotType::Bar))
        {
            currentPlotType = PlotType::Bar;
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    if (ImGui::TreeNode(Strings::NODE_TIME_INTERVALS))
    {
        ImGui::Text("Left bound: %s", formatTimestamp(leftTimeBound).c_str());
        ImGui::Text("Right bound: %s", formatTimestamp(rightTimeBound).c_str());

        ImGui::Separator();
        ImGui::Checkbox("Auto Refresh", &autoRefresh);
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::InputFloat("sec", &refreshIntervalSec, 1.0f, 5.0f, "%.1f");
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    ImGui::End();
}

void renderUI()
{
    renderMetricsViewer();
    renderSettings();
}

int main(int, char **)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, Strings::WINDOW_TITLE, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    io.FontGlobalScale = 1.2;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderUI();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    prometheusClient.reset();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
