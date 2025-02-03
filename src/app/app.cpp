#include <vector>

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

static bool showDemoWindow = false;
static bool autoRefresh = false;
static float refreshIntervalSec = 5.0f;
static float lastRefreshTime = 0.0f;
static std::string queryStr = "up";
static PrometheusClient *prometheusClient = nullptr;

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

void renderUI()
{
    // Можно сделать главное меню
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                // TODO: Proper exit
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("ImGui Demo", NULL, &showDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Окно с настройками
    ImGui::Begin("Settings");

    ImGui::Text("Prometheus Base URL:");
    static char urlBuffer[128] = "http://localhost:9090";
    ImGui::InputText("##BaseURL", urlBuffer, IM_ARRAYSIZE(urlBuffer));

    // При нажатии кнопки создаём или переинициализируем клиента
    if (ImGui::Button("Connect"))
    {
        if (prometheusClient)
        {
            delete prometheusClient;
            prometheusClient = nullptr;
        }
        prometheusClient = new PrometheusClient(urlBuffer);
    }

    ImGui::Separator();
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

    ImGui::Separator();
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

    ImGui::Separator();
    ImGui::Checkbox("Auto Refresh", &autoRefresh);
    ImGui::SameLine();
    ImGui::PushItemWidth(80);
    ImGui::InputFloat("sec", &refreshIntervalSec, 1.0f, 5.0f, "%.1f");
    ImGui::PopItemWidth();

    ImGui::End();

    // Окно с графиком
    ImGui::Begin("Metrics Viewer");
    if (ImPlot::BeginPlot("Time Series"))
    {
        // Для примера ось X как время
        ImPlot::SetupAxes("Time", "Value");

        // Для каждой серии рисуем
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
                // У ImPlot есть PlotBars, но она требует дополнительного параметра
                // ширины (width). Пусть будет 0.5
                double barWidth = 0.5;
                ImPlot::PlotBars(s.name.c_str(), s.x.data(), s.y.data(), s.x.size(), barWidth);
            }
        }

        ImPlot::EndPlot();
    }
    ImGui::End();

    // Дополнительно можем вывести окно демо ImGui/ImPlot
    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
        ImPlot::ShowDemoWindow(&showDemoWindow);
    }
}

int main(int, char **)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Low budget grafana", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

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
        // glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    if (prometheusClient)
        delete prometheusClient;
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
