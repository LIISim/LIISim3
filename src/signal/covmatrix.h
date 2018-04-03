#ifndef COVMATRIX_H
#define COVMATRIX_H

#include <QVector>

class CovMatrix
{
    public:

        CovMatrix(const int n);

        double operator()(int row, int col);

        inline int shape() { return N; }

        void set(int row, int col, double value);

        double max();

    private:

        QVector<double> data;
        int N;
        int M;

};

#endif // COVMATRIX_H
