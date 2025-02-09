#include <vector>
#include <ctime>
#include <memory>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "../lib/tsdb/prometheus/prometheus.h"

struct GraphSeries
{
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
};

static bool autoRefresh = false;
static float refreshIntervalSec = 5.0f;
static float lastRefreshTime = 0.0f;
static std::string queryStr = "up";
static std::unique_ptr<PrometheusClient> prometheusClient = nullptr;
static std::string connectionMessage;
static bool showConnectionMessage = false;

enum class PlotType
{
    Line,
    Scatter,
    Bar
};
static PlotType currentPlotType = PlotType::Line;

static std::vector<GraphSeries> seriesData;

void fetchData()
{
    if (!prometheusClient)
        return;

    Timestamp now = std::time(nullptr);
    Timestamp start = now - 60 * 60;
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
    ImGui::SetNextWindowSize(ImVec2(1280 - 400, 720), ImGuiCond_Always);
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
    ImGui::SetNextWindowPos(ImVec2(1280 - 400, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 720), ImGuiCond_Always);
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (ImGui::TreeNode("Range Widgets"))
    {
        ImGui::Text("Prometheus Base URL:");
        static char urlBuffer[128] = "http://localhost:9090";
        ImGui::InputText("##BaseURL", urlBuffer, IM_ARRAYSIZE(urlBuffer));

        if (ImGui::Button("Connect"))
        {
            prometheusClient = std::make_unique<PrometheusClient>(urlBuffer);
            if (prometheusClient->isAvailable())
            {
                connectionMessage = "Connection successful!";
            }
            else
            {
                connectionMessage = "Failed to connect to Prometheus.";
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
    if (ImGui::TreeNode("Query"))
    {
        ImGui::Text("PromQL Query:");
        static char queryBuffer[128] = "up";
        if (ImGui::InputText("##QueryStr", queryBuffer, IM_ARRAYSIZE(queryBuffer)))
        {
            queryStr = queryBuffer;
        }

        if (ImGui::Button("Fetch Data"))
        {
            fetchData();
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    if (ImGui::TreeNode("Plot settings"))
    {
        ImGui::Text("Plot Type:");
        if (ImGui::RadioButton("Line", currentPlotType == PlotType::Line))
        {
            currentPlotType = PlotType::Line;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scatter", currentPlotType == PlotType::Scatter))
        {
            currentPlotType = PlotType::Scatter;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Bar", currentPlotType == PlotType::Bar))
        {
            currentPlotType = PlotType::Bar;
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
    if (ImGui::TreeNode("Time intervals"))
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

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Low budget grafana", nullptr, nullptr);
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
    ImGui_ImplOpenGL3_Init("#version 410");

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
