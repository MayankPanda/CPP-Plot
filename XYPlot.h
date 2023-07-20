#include <windows.h>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <utility>
#include <cmath>
#include <set>
#include <random>
using namespace std;
struct RGBColor
{
    int r;
    int g;
    int b;
};
struct PlotDetails
{
    string legend;
    vector<double> x_coordinates;
    vector<double> y_coordinates;
    int connected;
};
bool isColorDuplicate(const RGBColor &color, const std::vector<RGBColor> &colors)
{
    for (const auto &existingColor : colors)
    {
        if (existingColor.r == color.r && existingColor.g == color.g && existingColor.b == color.b)
        {
            return true;
        }
    }
    return false;
}
std::vector<RGBColor> generateRandomColors(int n)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, 255);

    std::vector<RGBColor> colors;

    while (colors.size() < n)
    {
        RGBColor color = {distribution(gen), distribution(gen), distribution(gen)};
        if (!isColorDuplicate(color, colors))
        {
            colors.push_back(color);
        }
    }

    return colors;
}
class XYPlot
{
private:
    string PlotTitle;
    string XLabel;
    string YLabel;
    vector<PlotDetails *> plots;
    vector<RGBColor> plot_colors;
    vector<double> overall_x_coordinates;
    vector<double> overall_y_coordinates;
    set<double> unique_x_coordinates;
    set<double> unique_y_coordinates;
    bool LegendDisplay;
    int legendX;
    int legendY;

public:
    XYPlot()
    {
        PlotTitle = "";
        XLabel = "";
        YLabel = "";
        LegendDisplay = false;
    }
    void addLinePlot(vector<double> x, vector<double> y, string legendstr = "")
    {
        PlotDetails *p = new (PlotDetails);
        p->x_coordinates = x;
        p->y_coordinates = y;
        for (auto it : x)
        {
            overall_x_coordinates.push_back(it);
            unique_x_coordinates.insert(it);
        }
        for (auto it : y)
        {
            overall_y_coordinates.push_back(it);
            unique_y_coordinates.insert(it);
        }
        p->legend = legendstr;
        p->connected = 1;
        plots.push_back(p);
    }
    void addScatterPlot(vector<double> x, vector<double> y, string legendstr = "")
    {
        PlotDetails *p = new (PlotDetails);
        p->x_coordinates = x;
        p->y_coordinates = y;
        for (auto it : x)
        {
            overall_x_coordinates.push_back(it);
        }
        for (auto it : y)
        {
            overall_y_coordinates.push_back(it);
        }
        p->legend = legendstr;
        p->connected = 0;
        plots.push_back(p);
    }
    void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, int color)
    {
        // Set the line color
        SetDCPenColor(hdc, color);

        // Draw the line
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
    }

    void DrawColoredLine(HDC hdc, int x1, int y1, int x2, int y2, int r, int g, int b, int linewidth = 1)
    {
        // Create a custom pen with the desired color
        HPEN hPen = CreatePen(PS_SOLID, linewidth, RGB(r, g, b));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        // Draw the line
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);

        // Clean up: restore the old pen and delete the custom pen
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    void DrawText(HDC hdc, int x, int y, const std::string &text)
    {
        TextOutA(hdc, x, y, text.c_str(), static_cast<int>(text.length()));
    }

    std::string doubleToString(double value)
    {
        // Use stringstream to format the number
        std::stringstream ss;

        // Set the maximum size of the string to 5 characters
        ss << std::setprecision(4);

        // If the absolute value of the number is less than 0.0001, use scientific notation
        if (std::abs(value) < 0.0001 && std::abs(value) > 0.0000)
        {
            ss << std::scientific << std::setprecision(2) << value;
        }
        // If the absolute value of the number is greater than or equal to 1000, use scientific notation
        else if (std::abs(value) >= 1000)
        {
            ss << std::scientific << std::setprecision(2) << value;
        }
        // Otherwise, use fixed-point notation with 2 decimal places
        else
        {
            ss << std::fixed << std::setprecision(2) << value;
        }

        // Get the formatted string
        std::string result = ss.str();

        // If the number is in scientific notation
        if (result.find('E') != std::string::npos)
        {
            // Find the position of the 'E'
            size_t ePos = result.find('E');

            // Get the exponent part of the number
            std::string exponent = result.substr(ePos + 1);

            // If the exponent has more than two characters, remove the extra ones
            if (exponent.size() > 2)
            {
                result.erase(ePos + 3, std::string::npos);
            }
        }

        // Remove trailing zeros from the decimal part
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);

        // Remove the decimal point if it is at the end
        if (result.back() == '.')
            result.pop_back();

        return result;
    }

    void DrawTextWeight(HDC hdc, int x, int y, const std::string &text, int fontWeight)
    {
        // Create a font with the desired font weight
        LOGFONT logFont = {};
        logFont.lfWeight = fontWeight;
        HFONT hFont = CreateFontIndirect(&logFont);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        // Draw the text
        TextOutA(hdc, x, y, text.c_str(), static_cast<int>(text.length()));

        // Clean up: restore the old font and delete the custom font
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }
    void DrawBoundedAxes(HDC hdc, int x1, int y1, int x2, int y2)
    {
        // Calculate the inner offset at the bottom of the bounding box (10%)
        int yOffset = (y2 - y1) * 0.1;
        y2 -= yOffset;

        // Draw the box without background color
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, x1, y1, x2, y2);

        // Set the origin to the bottom-left corner of the box
        int centerX = x1;
        int centerY = y2;

        // Draw the x-axis (lower horizontal line)
        DrawLine(hdc, x1, centerY, x2, centerY, RGB(255, 0, 0));
        DrawText(hdc, x2 - 10, centerY + 10, "X");

        // Draw the y-axis (left vertical line)
        DrawLine(hdc, centerX, y1, centerX, y2, RGB(0, 255, 0));
        DrawText(hdc, centerX + 10, y1 + 10, "Y");

        // Draw numbers on the x-axis
        for (int i = 1; i <= 10; ++i)
        {
            int x = x1 + i * ((x2 - x1) / 10);
            DrawLine(hdc, x, centerY - 5, x, centerY + 5, RGB(0, 0, 0));

            // Convert the number to string
            std::string num = std::to_string(i);

            // Draw the number
            DrawText(hdc, x - 5, centerY + 10, num);
        }

        // Draw numbers on the y-axis
        for (int i = 1; i <= 10; ++i)
        {
            int y = y2 - i * ((y2 - y1) / 10); // Reverse order for y-axis
            DrawLine(hdc, centerX - 5, y, centerX + 5, y, RGB(0, 0, 0));

            // Convert the number to string
            std::string num = std::to_string(i);

            // Draw the number
            DrawText(hdc, centerX + 10, y - 5, num);
        }
    }

    void markcoordinates(HDC hdc, vector<double> x_coordinates, vector<double> y_coordinates, double x_range, double y_range, double x_lower_limit, double y_lower_limit)
    {
        for (auto it : x_coordinates)
        {
            // x_range: 640
            // y_range: 480
            double x_proportion = (it - x_lower_limit) / x_range;
            double x = 80.0 + x_proportion * 640.0;
            DrawLine(hdc, x, 480.0 - 5, x, 480.0 + 5, RGB(0, 0, 0));
            string num = doubleToString(it);
            // std::string num = std::to_string(it);
            LOGFONT logFont = {};
            logFont.lfWeight = FW_ULTRALIGHT;
            HFONT hFont = CreateFontIndirect(&logFont);
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            DrawText(hdc, x, 480.0 + 15.0, num);
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
        }
        for (auto it : y_coordinates)
        {
            double y_proportion = (it - y_lower_limit) / y_range;
            double x = 480.0 - y_proportion * 420.0;
            DrawLine(hdc, 75.0, x, 85.0, x, RGB(0, 0, 0));
            string num = doubleToString(it);
            LOGFONT logFont = {};
            logFont.lfWeight = FW_ULTRALIGHT;
            HFONT hFont = CreateFontIndirect(&logFont);
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            DrawText(hdc, 50.0, x, num);
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
        }
    }

    void DrawGridlines(HDC hdc, vector<double> x_coordinates, vector<double> y_coordinates, double x_range, double y_range, double x_lower_limit, double y_lower_limit)
    {
        for (auto it : x_coordinates)
        {
            double x_proportion = (it - x_lower_limit) / x_range;
            double x = 80.0 + x_proportion * 640.0;
            DrawColoredLine(hdc, x, 480.0, x, 60.0, 150, 150, 150);
            // DrawLine(hdc, x, 480.0, x, 60.0, RGB(255, 0, 0));
        }
        for (auto it : y_coordinates)
        {
            double y_proportion = (it - y_lower_limit) / y_range;
            double y = 480.0 - y_proportion * 420.0;
            DrawColoredLine(hdc, 80.0, y, 720.0, y, 150, 150, 150);
            // DrawLine(hdc, 80.0, y, 720.0, y, RGB(255, 0, 0));
        }
    }
    void DrawDot(HDC hdc, int x, int y, int r, int g, int b)
    {
        HBRUSH hBrush = CreateSolidBrush(RGB(r, g, b));
        SelectObject(hdc, hBrush);
        Ellipse(hdc, x - 4, y - 4, x + 4, y + 4);
        DeleteObject(hBrush);
    }

    void plotcoordinates(HDC hdc, vector<double> x_coordinates, vector<double> y_coordinates, double x_lower_limit, double y_lower_limit, double x_range, double y_range, RGBColor color)
    {
        for (int i = 0; i < x_coordinates.size(); i++)
        {
            double x_proportion = (x_coordinates[i] - x_lower_limit) / x_range;
            double x = 80.0 + x_proportion * 640.0;
            double y_proportion = (y_coordinates[i] - y_lower_limit) / y_range;
            double y = 480.0 - y_proportion * 420.0;
            DrawDot(hdc, x, y, color.r, color.g, color.b);
        }
    }

    void AddHeading(HDC hdc, int x, int y, const std::string &text)
    {
        // Get the width and height of the text
        LOGFONT logFont = {};
        logFont.lfWeight = FW_SEMIBOLD;
        HFONT hFont = CreateFontIndirect(&logFont);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SIZE textSize;
        GetTextExtentPoint32A(hdc, text.c_str(), static_cast<int>(text.length()), &textSize);

        // Calculate the position to center the text at the given coordinate
        int textWidth = textSize.cx;
        int textHeight = textSize.cy;
        int textX = x - textWidth / 2;
        int textY = y - textHeight / 2;

        // Draw the text at the calculated position
        TextOutA(hdc, textX, textY, text.c_str(), static_cast<int>(text.length()));
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }

    void AddXLabel(HDC hdc, int x, int y, const std::string &text)
    {
        // Get the width and height of the text
        LOGFONT logFont = {};
        logFont.lfWeight = FW_DEMIBOLD;
        HFONT hFont = CreateFontIndirect(&logFont);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SIZE textSize;
        GetTextExtentPoint32A(hdc, text.c_str(), static_cast<int>(text.length()), &textSize);

        // Calculate the position to center the text at the given coordinate
        int textWidth = textSize.cx;
        int textHeight = textSize.cy;
        int textX = x - textWidth / 2;
        int textY = y - textHeight / 2;

        // Draw the text at the calculated position
        TextOutA(hdc, textX, textY, text.c_str(), static_cast<int>(text.length()));
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }

    // void AddXLabel(HDC hdc, int centerX, int centerY, const std::string &text)
    // {
    //     // Create a RECT to calculate the text dimensions

    //     LOGFONT logFont = {};
    //     logFont.lfWeight = FW_DEMIBOLD;
    //     HFONT hFont = CreateFontIndirect(&logFont);
    //     HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    //     RECT rect = {0, 0, 0, 0};

    //     // Calculate the dimensions of the text
    //     DrawTextA(hdc, text.c_str(), static_cast<int>(text.length()), &rect, DT_CALCRECT);

    //     // Calculate the position to center the text at the given coordinate
    //     int textWidth = rect.right - rect.left;
    //     int textHeight = rect.bottom - rect.top;
    //     int textX = centerX - textWidth / 2;
    //     int textY = centerY - textHeight / 2;

    //     // Draw the text at the calculated position
    //     TextOutA(hdc, textX, textY, text.c_str(), static_cast<int>(text.length()));

    //     SelectObject(hdc, hOldFont);
    //     DeleteObject(hFont);
    // }

    void AddYLabel(HDC hdc, int x, int y, const std::string &text)
    {
        // Set the graphics mode to advanced
        LOGFONT logFont = {};
        logFont.lfWeight = FW_DEMIBOLD;
        HFONT hFont = CreateFontIndirect(&logFont);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SetGraphicsMode(hdc, GM_ADVANCED);
        SIZE textSize;
        GetTextExtentPoint32A(hdc, text.c_str(), static_cast<int>(text.length()), &textSize);
        int textWidth = textSize.cx;
        int textHeight = textSize.cy;
        int textX = x + textHeight / 2;
        int textY = y + textWidth / 2;
        // Define a transformation matrix for rotation
        XFORM transform;
        transform.eM11 = 0.0;
        transform.eM12 = -1.0;
        transform.eM21 = 1.0;
        transform.eM22 = 0.0;
        transform.eDx = static_cast<float>(textX);
        transform.eDy = static_cast<float>(textY);

        // Set the rotation transformation
        SetWorldTransform(hdc, &transform);

        // Draw the text
        TextOutA(hdc, 0, 0, text.c_str(), static_cast<int>(text.length()));

        // Reset the world transform
        SetWorldTransform(hdc, nullptr);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
    }

    void plotlines(HDC hdc, vector<double> x_coordinates, vector<double> y_coordinates, double x_lower_limit, double y_lower_limit, double x_range, double y_range, RGBColor color)
    {
        vector<pair<double, double>> points;
        for (int i = 0; i < x_coordinates.size(); i++)
        {
            points.push_back({x_coordinates[i], y_coordinates[i]});
        }
        sort(points.begin(), points.end());
        for (int i = 0; i < points.size() - 1; i++)
        {
            double x_proportion = (x_coordinates[i] - x_lower_limit) / x_range;
            double x1 = 80.0 + x_proportion * 640.0;
            x_proportion = (x_coordinates[i + 1] - x_lower_limit) / x_range;
            double x2 = 80.0 + x_proportion * 640.0;
            double y_proportion = (y_coordinates[i] - y_lower_limit) / y_range;
            double y1 = 480.0 - y_proportion * 420.0;
            y_proportion = (y_coordinates[i + 1] - y_lower_limit) / y_range;
            double y2 = 480.0 - y_proportion * 420.0;
            DrawColoredLine(hdc, x1, y1, x2, y2, color.r, color.g, color.b, 2);
            // DrawLine(hdc, x1, y1, x2, y2, RGB(255, 0, 0));
        }
    }
    vector<double> get_coordinates(double lower, double upper)
    {
        vector<double> vector_of_int;
        for (int i = ceil(lower); i <= floor(upper); i++)
        {
            cout << i << " ";
            vector_of_int.push_back(i);
        }
        cout << endl;
        return vector_of_int;
    }
    void SetPlotTitle(string plot_title)
    {
        PlotTitle = plot_title;
    }
    void SetXLabel(string x_label)
    {
        XLabel = x_label;
    }
    void SetYLabel(string y_label)
    {
        YLabel = y_label;
    }
    void DrawBoundingBox(HWND hwnd)
    {
        int X1 = 0.1 * 800;
        int X2 = 0.9 * 800;
        int Y1 = 0.1 * 600;
        int Y2 = 0.8 * 600;
        DrawLine(GetDC(hwnd), X1, Y1, X2, Y1, RGB(255, 0, 0));
        DrawLine(GetDC(hwnd), X1, Y2, X2, Y2, RGB(255, 0, 0));
        DrawLine(GetDC(hwnd), X1, Y1, X1, Y2, RGB(255, 0, 0));
        DrawLine(GetDC(hwnd), X2, Y1, X2, Y2, RGB(255, 0, 0));
    }
    void InitialiseCoordinateSpace(HWND hwnd)
    {
        double minX = *std::min_element(overall_x_coordinates.begin(), overall_x_coordinates.end());
        double maxX = *std::max_element(overall_x_coordinates.begin(), overall_x_coordinates.end());
        double minY = *std::min_element(overall_y_coordinates.begin(), overall_y_coordinates.end());
        double maxY = *std::max_element(overall_y_coordinates.begin(), overall_y_coordinates.end());

        double x_range = maxX - minX;
        double y_range = maxY - minY;

        double x_lower_lim = minX - (0.1 * x_range);
        double x_upper_lim = maxX + (0.1 * x_range);
        double y_lower_lim = minY - (0.1 * y_range);
        double y_upper_lim = maxY + (0.1 * y_range);
        vector<double> x_marked_coordinates, y_marked_coordinates;
        if (x_upper_lim - x_lower_lim >= 8.0)
        {
            vector<double> newx;
            double interval_size = (x_upper_lim - x_lower_lim) / 8;
            for (int i = 1; i <= 8 - 1; i++)
            {
                newx.push_back(x_lower_lim + i * interval_size);
            }
            x_marked_coordinates = newx;
        }
        else
        {
            x_marked_coordinates = get_coordinates(x_lower_lim, x_upper_lim);
        }
        if (y_upper_lim - y_lower_lim >= 8.0)
        {
            vector<double> newx;
            double interval_size = (y_upper_lim - y_lower_lim) / 8;
            for (int i = 1; i <= 8 - 1; i++)
            {
                newx.push_back(y_lower_lim + i * interval_size);
            }
            y_marked_coordinates = newx;
        }
        else
        {
            y_marked_coordinates = get_coordinates(y_lower_lim, y_upper_lim);
        }

        // if number of marked coordinates is less than n-1 where n is the number of points then uniform divide
        if (x_marked_coordinates.size() < unique_x_coordinates.size() - 1)
        {
            vector<double> newx;
            double interval_size = (x_upper_lim - x_lower_lim) / (unique_x_coordinates.size());
            for (int i = 1; i <= unique_x_coordinates.size() - 1; i++)
            {
                newx.push_back(x_lower_lim + i * interval_size);
            }
            x_marked_coordinates = newx;
        }
        if (y_marked_coordinates.size() < unique_y_coordinates.size() - 1)
        {
            vector<double> newy;
            double interval_size = (y_upper_lim - y_lower_lim) / (unique_y_coordinates.size());
            for (int i = 1; i <= unique_y_coordinates.size() - 1; i++)
            {
                newy.push_back(y_lower_lim + i * interval_size);
            }
            y_marked_coordinates = newy;
        }
        markcoordinates(GetDC(hwnd), x_marked_coordinates, y_marked_coordinates, x_upper_lim - x_lower_lim, y_upper_lim - y_lower_lim, x_lower_lim, y_lower_lim);
        DrawGridlines(GetDC(hwnd), x_marked_coordinates, y_marked_coordinates, x_upper_lim - x_lower_lim, y_upper_lim - y_lower_lim, x_lower_lim, y_lower_lim);
        plot_colors = generateRandomColors(plots.size());
        for (int i = 0; i < plots.size(); i++)
        {
            if (plots[i]->connected == 0)
            {
                plotcoordinates(GetDC(hwnd), plots[i]->x_coordinates, plots[i]->y_coordinates, x_lower_lim, y_lower_lim, x_upper_lim - x_lower_lim, y_upper_lim - y_lower_lim, plot_colors[i]);
            }
            else
            {
                plotlines(GetDC(hwnd), plots[i]->x_coordinates, plots[i]->y_coordinates, x_lower_lim, y_lower_lim, x_upper_lim - x_lower_lim, y_upper_lim - y_lower_lim, plot_colors[i]);
            }
        }
    }
    void SetTextDisplay(HWND hwnd)
    {
        AddHeading(GetDC(hwnd), 400.0, 30.0, PlotTitle);
        AddYLabel(GetDC(hwnd), 0.0, 300.0, YLabel);
        AddXLabel(GetDC(hwnd), 400.0, 530.0, XLabel);
    }
    void DrawSquare(HDC hdc, int x, int y, int r, int g, int b)
    {
        // Calculate the square size (width and height)
        int squareSize = 20;

        // Create a color brush for the square
        HBRUSH hBrush = CreateSolidBrush(RGB(r, g, b));
        SelectObject(hdc, hBrush);

        // Draw the square
        Rectangle(hdc, x, y, x + squareSize, y + squareSize);

        // Clean up: delete the color brush
        DeleteObject(hBrush);
    }
    void DrawLegends(HWND hwnd, int legendX = 600, int legendY = 80)
    {
        for (int i = 0; i < plots.size(); i++)
        {
            // Draw the color dot
            // DrawDot(GetDC(hwnd), legendX, legendY, plot_colors[i].r, plot_colors[i].g, plot_colors[i].b);
            DrawSquare(GetDC(hwnd), legendX, legendY, plot_colors[i].r, plot_colors[i].g, plot_colors[i].b);

            // Draw the identifier name
            TextOutA(GetDC(hwnd), legendX + 25, legendY, plots[i]->legend.c_str(), static_cast<int>(plots[i]->legend.size()));

            // Update the legend position for the next entry
            legendY += 25;
        }
    }
    void DisplayLegends(int legendX_coordinate = 600, int legendY_coordinate = 80)
    {
        LegendDisplay = true;
        legendX = legendX_coordinate;
        legendY = legendY_coordinate;
    }

    void DisplayPlot()
    {
        // Register the window class
        const char CLASS_NAME[] = "Sample Window Class";

        WNDCLASSA wc = {}; // Use WNDCLASSA for narrow character strings
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;

        RegisterClassA(&wc); // Use RegisterClassA for narrow character strings

        // Create the window
        HWND hwnd = CreateWindowExA(
            0,
            CLASS_NAME,
            "XY Plot Window",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
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

        DrawBoundingBox(hwnd);
        InitialiseCoordinateSpace(hwnd);
        SetTextDisplay(hwnd);
        if (LegendDisplay)
            DrawLegends(hwnd, legendX, legendY);
        // addLinePlot({1,3,5,4,2},{1,3,5,4,2});
        //  Message loop
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
};