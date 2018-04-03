#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QVector>

class Spectrum
{

public:

    Spectrum();
    Spectrum(int lambda, int bandwidth);

    QVector<double> xData;
    QVector<double> yData;


    double integrate(double xStart, double xEnd);

    Spectrum getSection(double xStart, double xEnd);

    int getIndexOf(double x);
    double getYofX(double x);

};

#endif // SPECTRUM_H
