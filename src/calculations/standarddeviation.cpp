#include "standarddeviation.h"

#include <math.h>


// Calculate standard deviation
//  https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
//  https://en.wikipedia.org/wiki/Algebraic_formula_for_the_variance
//  https://de.wikipedia.org/wiki/Verschiebungssatz_(Statistik)
// sigma^2 = 1/(N-1) * [sum of squares - sum^2/N]

StandardDeviation::StandardDeviation()
{
    _sum  = 0.0;
    _sum2 = 0.0;

    _N = 0;
}


void StandardDeviation::reset()
{
    _sum  = 0.0;
    _sum2 = 0.0;

    _N = 0;
}


void StandardDeviation::addValue(double value)
{
    _sum  += value;
    _sum2 += pow(value, 2);

    _N++;
}


void StandardDeviation::addVector(QVector<double> vector)
{
    for(int k=0; k < vector.size(); k++)
    {
        _sum += vector.at(k);
        _sum2 += pow(vector.at(k), 2);
        _N++;
    }
}


double StandardDeviation::getStandardDeviation()
{
    if(_N > 0)
        return sqrt(1.0 / (_N - 1.0) * (_sum2 - pow(_sum, 2) / _N));
    else
        return 0.0;
}


double StandardDeviation::getMean()
{
    if(_N > 0)
        return _sum / _N;
    else
        return 0.0;
}
