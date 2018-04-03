#ifndef STANDARDDEVIATION_H
#define STANDARDDEVIATION_H

#include <QVector>

class StandardDeviation
{
public:
    StandardDeviation();

    void reset();

    void addValue(double value);
    void addVector(QVector<double> vector);

    double getStandardDeviation();
    double getMean();

private:

    /** @brief _sum sum of data SUM(x)   */
    double _sum;
    /** @brief _sum2 sum of squares of data SUM(x2)*/
    double _sum2;

    /** @brief _N number of values  */
    int _N;
};

#endif // STANDARDDEVIATION_H
