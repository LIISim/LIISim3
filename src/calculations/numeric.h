#ifndef NUMERIC_H
#define NUMERIC_H

#include "../signal/signal.h"
#include "../calculations/heattransfermodel.h"
#include "../calculations/temperature.h"

#include <vector>
#include <functional> //rk4 bind()
#include <numeric> // accumulate()

#include <boost/multi_array.hpp>

#include "../signal/covmatrix.h"

//#include <opencv2/core/core.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include <QList>

#include "../../calculations/fit/fitrun.h"
#include "../../calculations/fit/fititerationresult.h"

// cannot use standard array here, this needs to be done dynamically:
// error: cannot allocate an array of constant size
typedef boost::multi_array<double,2> MatDoub;
typedef boost::multi_array<double,1> VecDoub;
typedef boost::multi_array<bool,1> VecBool;


class Numeric
{
    public:
        Numeric();

        typedef std::vector< double > state_type;

        /** @brief The ODE_Algorithm enum  defines which ODE solver is used,
         * SolverCount is used to determine number of enum elements */
        enum ODE_Algorithm {EULER, RK4, RKCK54, RKD5, RKD5_OPT, RKF78, SolverCount};

        static ODE_Algorithm defaultODESolver();
        static QString getODEName(int idx);
        static QString getODEDescription(int idx);
        static QStringList getAvailableODENameList();
        static QStringList getAvailableODEDescriptionList();

        static bool canceled;
        static double EPS;
        static double TOL;
        static int iterationsDefault;
        static double stepSizeDefault;


        //Signal euler(double T_start, double dp_start, HeatTransferModel &ht_object);
        static Signal solveODE(double T_start,
                               double dp_start,
                               HeatTransferModel &ht_object,                               
                               int noDataPoints,
                               double dt,
                               NumericSettings *ns);


        /** @brief levmar universal fitting routine (modeledData() provides individual models (FitMode) */
        static void levmar(FitRun::FitMode mode,
                           FitData* fd,
                           ModelingSettings* ms,
                           FitSettings* fs,
                           NumericSettings* ns);

        static void levmar_test(FitRun::FitMode mode,
                           FitData* fd,
                           ModelingSettings* ms,
                           FitSettings* fs,
                           NumericSettings* ns);

        static QVector<double> modeledData(FitRun::FitMode mode,
                                                    VecDoub a,
                                                    FitData *fd,
                                                    ModelingSettings *ms,
                                                    FitSettings *fs,
                                                    NumericSettings *ns);


        state_type filterMedian(int size, state_type channel_values);
        state_type filterPrewitt(state_type channel_values);

        // Helper
        static double integrate_trapezoid(Property prop, const double T0, const double T1);

        // define number of data points (n)
        static double integrate_trapezoid_func(std::function<double(double)> func, const double T0, const double T1, const int n);

        // define accuracy of integration (acc)
        static double integrate_trapezoid_func(std::function<double(double)> func, const double T0, const double T1, const double acc);

        static bool swapLine(MatDoub mat, size_t line1, size_t line2);
        static bool invertMatrix(const MatDoub mat, MatDoub &inv);

        static bool cholesky_LLT(const MatDoub A, MatDoub &L);
        static bool cholesky_solve_LLT(const MatDoub L, const VecDoub b, VecDoub &x);
        static bool cholesky_inverse_A(const MatDoub L, MatDoub &Ainv);

        static bool cholesky_LDLT(const MatDoub A, MatDoub &L, MatDoub &D);
        static bool cholesky_solve_LDLT(const MatDoub L, const MatDoub D, const VecDoub b, VecDoub &x);


        static void debugCholesky(MatDoub alpha_lambda, MatDoub L, MatDoub Linv, MatDoub D, MatDoub covar, VecDoub da, VecDoub solution, VecDoub beta);
        static void debugCovarianceMatrix();

};

#endif // NUMERIC_H
