#include "xyplot.h"
using namespace std;
int main()
{
    XYPlot p;
    p.addLinePlot({1,2,3,4,5},{1,2,3,4,5},"Line Plot 1");
    p.addLinePlot({-1,-2,-3,-4,-5},{1,2,3,4,5},"Line Plot 2");
    p.addScatterPlot({1,2,3,4,5},{1,2,3,4,5},"Scatter");
    p.SetPlotTitle("Title");
    p.SetXLabel("X");
    p.SetYLabel("Y Label Text");
    p.DisplayLegends();
    p.DisplayPlot();
}