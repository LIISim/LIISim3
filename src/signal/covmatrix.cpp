#include "covmatrix.h"

/**
 * @brief CovMatrix::CovMatrix creates internal storage vector according to matrix shape
 * @param n shape of matrix
 */
CovMatrix::CovMatrix(const int n)
{
    // covar shape
    N = n;

    // internal vector size
    M = (1+N)*N/2;

    data.resize(M);
}


/**
 * @brief CovMatrix::operator () access symmetric matrix
 * @param row
 * @param col
 * @return
 */
double CovMatrix::operator ()(int row, int col)
{
    int idx;

    //if element is in upper triangle, flip indices
    if(col > row)
        idx = col * N + row;
    else
        idx = row * N + col;

    return data.at(idx);
}


/**
 * @brief CovMatrix::set assign data to matrix, index is automatic flipped to write lower triangular matrix
 * @param row
 * @param col
 * @param value
 */
void CovMatrix::set(int row, int col, double value)
{
    int idx;

    //if element is in upper triangle, flip indices
    if(col > row)
        idx = col * N + row;
    else
        idx = row * N + col;

    data.insert(idx, value);
}


/**
 * @brief CovMatrix::max returns maximum value in matrix
 * @return
 */
double CovMatrix::max()
{
    return *std::max_element(data.constBegin(), data.constEnd());
}

