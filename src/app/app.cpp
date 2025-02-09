#include <vector>
#include <ctime>
#include <memory>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "../lib/tsdb/prometheus/prometheus.h"
#include "constants.h"

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
static std::string connectionMessage;
static bool showConnectionMessage = false;

static PlotType currentPlotType = PlotType::Line;
static std::vector<GraphSeries> seriesData;

void fetchData()
{
    if (!prometheusClient)
        return;

    Timestamp now = std::time(nullptr);
    Timestamp start = now - DEFAULT_FETCH_RANGE;
    Timestamp end = now;

    std::vector<Metric> metrics = prometheusClient->query(queryStr, start, end);

    seriesData.clear();

    for (auto &m : metrics)
    {
        GraphSeries s;
        s.name = m.name;
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

    ImVec2 plotSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 20);
    if (ImPlot::BeginPlot("Time Series", plotSize))
    {
        ImPlot::SetupAxes("Time", "Value");

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
        ImGui::InputText("##BaseURL", urlBuffer, IM_ARRAYSIZE(urlBuffer));

        if (ImGui::Button(Strings::NODE_CONNECTION))
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
            ImGui::TextWrapped(connectionMessage.c_str());
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    if (ImGui::TreeNode(Strings::NODE_QUERY))
    {
        ImGui::Text(Strings::LABEL_QUERY);
        static char queryBuffer[128];
        std::strncpy(queryBuffer, DEFAULT_QUERY, sizeof(queryBuffer) - 1);
        if (ImGui::InputText("##QueryStr", queryBuffer, IM_ARRAYSIZE(queryBuffer)))
        {
            queryStr = queryBuffer;
        }

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
