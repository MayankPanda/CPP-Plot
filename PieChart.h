#include <vector>
#include <string>
#include <windows.h>
#include <random>
#include <stdlib.h>
#include <cstdlib>
using namespace std;
struct RGB
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};
struct Sector
{
    double value;           // Value or proportion of the sector
    COLORREF color;         // Color of the sector
    std::string identifier; // Identifier of the sector
};
class PieChart
{
    vector<Sector> sectors;
    int numPoints;
    std::vector<RGB> GenerateRandomColors(int numColors)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        std::vector<RGB> colors(numColors);

        for (int i = 0; i < numColors; ++i)
        {
            RGB color;
            color.red = static_cast<unsigned char>(dis(gen));
            color.green = static_cast<unsigned char>(dis(gen));
            color.blue = static_cast<unsigned char>(dis(gen));

            // Check if the generated color already exists
            bool duplicateColor = false;
            for (int j = 0; j < i; ++j)
            {
                if (colors[j].red == color.red && colors[j].green == color.green && colors[j].blue == color.blue)
                {
                    duplicateColor = true;
                    break;
                }
            }

            if (duplicateColor)
            {
                --i; // Retry generating a color
            }
            else
            {
                colors[i] = color;
            }
        }

        return colors;
    }

    std::vector<Sector> CreateSectors(const std::vector<double> &values, const std::vector<std::string> &identifiers)
    {
        std::vector<Sector> sectors;

        // Generate random colors for the sectors
        std::vector<RGB> colors = GenerateRandomColors(values.size());

        // Create the sectors
        for (size_t i = 0; i < values.size(); ++i)
        {
            Sector sector;
            sector.value = values[i];
            sector.color = RGB(colors[i].red, colors[i].green, colors[i].blue);
            sector.identifier = identifiers[i];
            sectors.push_back(sector);
        }

        return sectors;
    }

public:
    void InitialisePieChart(vector<double> proportions, vector<string> identifiers = {})
    {
        if (identifiers.size() == 0)
        {
            vector<string> temp(proportions.size(), "");
            identifiers = temp;
        }
        numPoints = proportions.size();
        sectors = CreateSectors(proportions, identifiers);
        int centerX = 400;
        int centerY = 300;
        int radius = 200;

        CreatePieChartWindow(sectors, centerX, centerY, radius);
    }

    void CreatePieChartWindow(const std::vector<Sector> &sectors, int centerX, int centerY, int radius)
    {
        // Register the window class
        const char CLASS_NAME[] = "Sample Window Class";

        WNDCLASSA wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;

        RegisterClassA(&wc);

        // Create the window
        HWND hwnd = CreateWindowExA(
            0,
            CLASS_NAME,
            "Pie Chart Window",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            NULL,
            NULL,
            GetModuleHandle(NULL),
            NULL);

        if (hwnd == NULL)
        {
            return;
        }

        ShowWindow(hwnd, SW_SHOW);

        // Get the device context for drawing
        HDC hdc = GetDC(hwnd);

        // Draw the pie chart
        DrawPieChart(hdc, sectors, centerX, centerY, radius);

        // Draw the legend
        int legendX = centerX + radius + 20;
        int legendY = centerY + radius - 20;
        DrawLegend(hdc, sectors, legendX, legendY);

        // Release the device context
        ReleaseDC(hwnd, hdc);

        // Main message loop
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void DrawPieChart(HDC hdc, const std::vector<Sector> &sectors, int centerX, int centerY, int radius)
    {
        // Validate the input vector
        if (sectors.empty())
        {
            return;
        }

        // Calculate the total value of all sectors
        double totalValue = 0.0;
        for (const Sector &sector : sectors)
        {
            totalValue += sector.value;
        }

        // Generate random colors for the sectors
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        // Draw the sectors of the pie chart
        double startAngle = 0.0;
        for (const Sector &sector : sectors)
        {
            // Calculate the sweep angle for the current sector
            double sweepAngle = 360.0 * sector.value / totalValue;

            // Set the brush color for the sector
            HBRUSH brush = CreateSolidBrush(sector.color);
            SelectObject(hdc, brush);

            // Draw the sector as a pie slice
            Pie(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius,
                static_cast<int>(centerX + radius * cos(startAngle * 3.14159265358979323846 / 180.0)),
                static_cast<int>(centerY - radius * sin(startAngle * 3.14159265358979323846 / 180.0)),
                static_cast<int>(centerX + radius * cos((startAngle + sweepAngle) * 3.14159265358979323846 / 180.0)),
                static_cast<int>(centerY - radius * sin((startAngle + sweepAngle) * 3.14159265358979323846 / 180.0)));

            // Update the start angle for the next sector
            startAngle += sweepAngle;
        }
    }

    void DrawLegend(HDC hdc, const std::vector<Sector> &sectors, int legendX, int legendY)
    {
        for (const Sector &sector : sectors)
        {
            // Draw the color box
            HBRUSH colorBoxBrush = CreateSolidBrush(sector.color);
            SelectObject(hdc, colorBoxBrush);
            Rectangle(hdc, legendX, legendY, legendX + 20, legendY + 20);

            // Draw the identifier name
            TextOutA(hdc, legendX + 30, legendY, sector.identifier.c_str(), static_cast<int>(sector.identifier.size()));

            // Update the legend position for the next sector
            legendY += 25;
        }
    }
};