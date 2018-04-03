#include "spectrum.h"

#include <QDebug>;

Spectrum::Spectrum()
{
}


/**
 * @brief Spectrum::Spectrum generates empty spectrum containing all wavlength within bandwidth
 * @param lambda [nm]
 * @param bandwidth [nm]
 */
Spectrum::Spectrum(int lambda, int bandwidth)
{
    int b2 = floor(bandwidth / 2);

    for(int i = lambda-b2; i <= lambda+b2; i++)
    {
       xData.append(double(i));
       yData.append(0.0);
    }
}


/**
 * @brief Spectrum::integrate integrates spectrum between x values using Newton-Cotes formula
 * @param xStart
 * @param xEnd
 * @return integration result between xStart and xEnd
 */
double Spectrum::integrate(double xStart, double xEnd)
{

    Spectrum spec = this->getSection(xStart,xEnd);

    // if section is empty / invalid return zero value
    if (spec.xData.size() < 1)
        return 0.0;

    // if range contains only one value return this value
    if (spec.xData.size() == 1)
        return yData.at(0);

    // Newton-Cotes formulas
    // https://en.wikipedia.org/wiki/Newton%E2%80%93Cotes_formulas
    // Trapezoid rule: Single Integral = (b-a)*(f(a) + f(b))/2

    double a, b, y_a, y_b;
    double res = 0.0;

    // loop over (n - 1) values
    for(int i=0; i < spec.xData.size()-1; i++)
    {
        a = spec.xData.at(i);
        b = spec.xData.at(i+1);

        y_a = spec.yData.at(i);
        y_b = spec.yData.at(i+1);

        res = res + (b - a) * ((y_a + y_b) / 2.0);

        //qDebug() << res << " = " << y_a << " " << y_b;
    }

    return res;
}


/**
 * @brief Spectrum::getSection
 * @param xStart
 * @param xEnd
 * @return new spectrum between xStart and xEnd
 */
Spectrum Spectrum::getSection(double xStart, double xEnd)
{
    Spectrum nspec;

    for(int i=0; i < xData.size(); i++)
    {
        if(xData.at(i) >= xStart && xData.at(i) <= xEnd)
        {
            nspec.xData.append(xData.at(i));
            nspec.yData.append(yData.at(i));
        }

    }
    return nspec;
}


/**
 * @brief Spectrum::getIndexOf returns index of x value
 * @param x
 * @return index of elemt in xData vector
 */
int Spectrum::getIndexOf(double x)
{
    for(int i=0; i <= xData.size(); i++)
    {
        if(xData.at(i) == x)
            return i;
    }
    return -1;
}


/**
 * @brief Spectrum::getYofX get Y value at X
 * @param x
 * @return y
 */
double Spectrum::getYofX(double x)
{
    for(int i=0; i < xData.size(); i++)
    {
        if(xData.at(i) == x)
            return yData.at(i);
    }

    qDebug() << "Spectrum: getYofX error (TODO: interpolate value)";
    return 0;
}
