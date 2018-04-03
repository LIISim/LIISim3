#include "numeric.h"
#include "../general/LIISimException.h"

#include <boost/numeric/odeint.hpp> // runge_kutta4

#include <algorithm>

#include "../core.h"
#include "../gui/utils/signalplotwidgetqwt.h"

#include "../../general/profiler.h"

namespace odeint = boost::numeric::odeint;
namespace pl = std::placeholders;

/*******************************
 *      DEFAULT VALUES
 *******************************/
bool Numeric::canceled = false;
double Numeric::EPS = 1E-15;
double Numeric::TOL = 1E-15; // 1E-5

int Numeric::iterationsDefault = 200;
double Numeric::stepSizeDefault = 1E-9;


Numeric::Numeric()
{
}

Numeric::ODE_Algorithm Numeric::defaultODESolver()
{
    return ODE_Algorithm::RK4;
}


QString Numeric::getODEName(int idx)
{
    if(idx > ODE_Algorithm::SolverCount || idx < 0)
        return QString("ODE solver not found");
    else if(idx == ODE_Algorithm::EULER)
        return QString("Explicit Euler");
    else if(idx == ODE_Algorithm::RK4)
        return QString("Runge-Kutta 4");
    else if(idx == ODE_Algorithm::RKCK54)
        return QString("Cash-Karp");
    else if(idx == ODE_Algorithm::RKD5)
        return QString("Dormand-Prince 5");
    else if(idx == ODE_Algorithm::RKD5_OPT)
        return QString("Dormand-Prince 5 (optimized for LII)");
    else if(idx == ODE_Algorithm::RKF78)
        return QString("Fehlberg 78");
    else
        return QString("not defined: Numeric::getODEName");
}


QString Numeric::getODEDescription(int idx)
{
    if(idx > ODE_Algorithm::SolverCount || idx < 0)
        return QString("ODE solver not found");
    else if(idx == ODE_Algorithm::EULER)
        return QString("Explicit Euler (euler) \n "
                       "Type: Dense Output Stepper \n"
                       "Order: 1 \n"
                       "Very simple, only for demonstrating purpose");
    else if(idx == ODE_Algorithm::RK4)
        return QString("Runge-Kutta 4 (runge_kutta4) - Stepper \n "
                       "Type: Dense Output Stepper \n"
                       "Order: 4 \n"
                       "The classical Runge-Kutta scheme, good general scheme "
                       "without error control");
    else if(idx == ODE_Algorithm::RKCK54)
        return QString("Cash-Karp (runge_kutta_cash_karp54) - Error Stepper \n "
                       "Type: Dense Output Stepper \n"
                       "Order: 5 \n"
                       "Standard method with error control and dense output");
    else if(idx == ODE_Algorithm::RKD5)
        return QString("Dormand-Prince 5 (runge_kutta_dopri5) - Error Stepper \n "
                       "Type: Dense Output Stepper \n"
                       "Order: 5 \n"
                       "Standard method with error control and dense output");
    else if(idx == ODE_Algorithm::RKD5_OPT)
        return QString("Dormand-Prince 5 (runge_kutta_dopri5) - Error Stepper \n "
                       "Type: Dense Output Stepper \n"
                       "Order: 5 \n"
                       "Standard method with error control and dense output\n\n"
                       "This stepper increases stepper accuracy in the beginning of the LII signal.");
    else if(idx == ODE_Algorithm::RKF78)
        return QString("Fehlberg 78 (runge_kutta_fehlberg78) - Error Stepper \n "
                       "Type: Dense Output Stepper \n"
                       "Order: 8 \n"
                       "Good high order method with error estimation");
    else
        return QString("not defined: Numeric::getODEDescription");
}


QStringList Numeric::getAvailableODENameList()
{
    QStringList list;

    for(int i = 0; i < ODE_Algorithm::SolverCount; i++)
        list << getODEName(i);

    return list;
}


QStringList Numeric::getAvailableODEDescriptionList()
{
    QStringList list;

    for(int i = 0; i < ODE_Algorithm::SolverCount; i++)
        list << getODEDescription(i);

    return list;
}


/**
 * @brief Numeric::solveODE
 *        Gives temperature vector for given start parameters and heat transfer model
 * @param T_start start temperature
 * @param dp_start start particle diameter [nm]
 * @param ht_object HeatTransferModel
 * @param noDataPoints number of data points (signal length)
 * @param dt integration step size [s]
 * @param ns NumericSettings
 * @return Signal object containing time and temperature vector
 */
Signal Numeric::solveODE(double T_start,
                         double dp_start,
                         HeatTransferModel &ht_object,
                         int noDataPoints,
                         double dt,
                         NumericSettings *ns)
{

    // convert from [nm] to [m]
    dp_start = dp_start * 1E-9;
    // model gives particle size in meter, convert to nm for GUI
    double unit_m_to_nm = 1E9;

    // BOOST:ODEINT Library
    // http://www.boost.org/doc/libs/1_61_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/getting_started/overview.html

    // Explicit Euler
    odeint::euler<state_type> euler;

    // Runge-Kutta (4th order)
    odeint::runge_kutta4<state_type> rk4;

    // Cash-Karp
    odeint::runge_kutta_cash_karp54<state_type> rkck54;

    // Dormand-Prince 5, also used in MATLAB: ode45
    odeint::runge_kutta_dopri5< state_type > rkd5;

    // Fehlberg 78
    odeint::runge_kutta_fehlberg78<state_type> rkf78;


    // initialization
    Signal signal = Signal(noDataPoints, dt);
    signal.type = Signal::TEMPERATURE;

    double t;
    state_type x(2);
    state_type dxdt(2);
    state_type xout(2);
    state_type dxdtout(2);
    state_type xerr(2);

    int ODEsolver = ns->odeSolverIdx();

    /* StepSizeFactor
     *  - Accuracy of ODE solver step size will be increased by this factor compared to the
     *  data step size
     *  - will only be used for data fit, for simulations this will be equal 1
     */
    int ODEsolverStepSizeFactor = ns->odeSolverStepSizeFactor();
    double dt_internal = dt / double(ODEsolverStepSizeFactor);

    //qDebug() << "Numeric::solver: " << getODEName(ODEsolver) << " - " << ODEsolver;


    // timer for processing performance
    bool timer = true;
    QString timer_str = QString("Numeric::Timer: %0 - %1").arg(T_start).arg(dp_start);
    Profiler p(timer_str.toStdString());
    if(timer)
        p.begin();


    // start values
    t = 0.0;            // start time    
    x[0] = T_start;     // start temperature (peak)

    // type of system function:
    // sysfunc = 0: x[0]:  temperature x[1]:  particle diameter
    // sysfunc = 1: x[0]:  temperature x[1]:  particle mass
    // is defined in HTM class: ht_object.sysfunc = HeatTransferModel::dT_ddp;

    if(ht_object.sysFunc() == HeatTransferModel::dT_dM)
    {
        x[1] = ht_object.calculateMassFromDiameter(T_start, dp_start);    // start particle mass
        // set first value in curve
        signal.data[0]         = x[0];
        signal.dataDiameter[0] = ht_object.calculateDiameterFromMass(x[0], x[1]) * unit_m_to_nm;
    }
    else
    {
        x[1] = dp_start;    // start particle diameter
        // set first value in curve
        signal.data[0]         = x[0];
        signal.dataDiameter[0] = x[1] * unit_m_to_nm;
    }

    // define initial dxdt (x and dxdt will be overwritten)
    ht_object.ode_sys(x ,dxdt, t);

    // now proceed for second value
    t += dt;


    //qDebug() << "Pressure Pa: " << ht_object.getParameter(2) << "GasTemperature: " << ht_object.getParameter(1);
    //qDebug() << "Number of Datapoints: " << noDataPoints;
    //qDebug() << "StepSize: " << dt << "StepSize(internal): " << dt_internal << "StepSizeFactor:" << ODEsolverStepSizeFactor;


    // calculate temperature trace
    for(int i =  1 ; i < noDataPoints; i++)
    {
        // HeatTransferModel::sys(const state_type &x, state_type &dxdt, double /* t */)
        // http://www.boost.org/doc/libs/1_55_0b1/libs/numeric/odeint/doc/html/boost_numeric_odeint/odeint_in_detail/steppers.html        

        switch(ODEsolver)
        {

        case ODE_Algorithm::EULER:

            for(int j = 0; j < ODEsolverStepSizeFactor; j++)
            {
                euler.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                     pl::_1 ,  // x
                                     pl::_2 ,  // dxdt
                                     pl::_3 ), // t
                              x, t, dt_internal);

                t = t + dt_internal;

                //qDebug() << t << " - dt_internal: " << dt_internal << " - " << xout[0];
            }

            break;


        default:
        case ODE_Algorithm::RK4:

            for(int j = 0; j < ODEsolverStepSizeFactor; j++)
            {
                rk4.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                      pl::_1 ,  // x
                                      pl::_2 ,  // dxdt
                                      pl::_3 ), // t
                            x, t, dt_internal);

                t = t + dt_internal;

                //qDebug() << t << " - dt_internal: " << dt_internal << " - " << xout[0];
            }

            break;


        case ODE_Algorithm::RKCK54:

            for(int j = 0; j < ODEsolverStepSizeFactor; j++)
            {
                rkck54.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                      pl::_1 ,  // x
                                      pl::_2 ,  // dxdt
                                      pl::_3 ), // t
                            x, t, dt_internal, xerr);

                // xerr currently not used

                t = t + dt_internal;

                //qDebug() << t << " - dt_internal: " << dt_internal << " - " << x[0] << xerr[0];
            }
            break;


        case ODE_Algorithm::RKD5:

            /***********************************************
             *  Caution: FSAL (first-same-as-last) stepper:
             * *********************************************
             * The FSAL-steppers save the derivative at time t+dt internally
             * if they are called via do_step( sys , in , out , t , dt ).
             *
             * The first call of do_step will initialize dxdt and for all
             * following calls it is assumed that the same system and
             * the same state are used.
             * See the Using steppers section for more details or look into
             * the table below to see which stepper have an internal state.
             * http://www.boost.org/doc/libs/1_55_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/odeint_in_detail/steppers.html#boost_numeric_odeint.odeint_in_detail.steppers.using_steppers
             *
             * Internal derivative is not used, dxdtinout is updated:
             * Syntax: rk.do_step( sys , inout , dxdtinout , t , dt );
             *
             * Internal derivative is used, :
             * Syntax: rk.do_step( sys , inout , t , dt );
             **/

            for(int j = 0; j < ODEsolverStepSizeFactor; j++)
            {
                rkd5.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                    pl::_1 ,  // x
                                    pl::_2 ,  // dxdt
                                    pl::_3 ), // t
                                x, t, dt_internal, xerr);

                // xerr currently not used

                t = t + dt_internal;

                //qDebug() << t << " - dt_internal: " << dt_internal << " - " << x[0] << xerr[0];
            }
            break;


        // Compared to the RKD5, this optimized algorithm additionally increases accuracy in the LII peak region
        case ODE_Algorithm::RKD5_OPT:

            for(int j = 0; j < ODEsolverStepSizeFactor; j++)
            {
                rkd5.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                        pl::_1 ,  // x
                                        pl::_2 ,  // dxdt
                                        pl::_3 ), // t
                              x, dxdt, t, xout, dxdtout, dt_internal, xerr);

                //qDebug() << "call 1: " << x[0] << dxdt[0] << xout[0] << dxdtout[0] << xerr[0];


                // sometimes if gradient and step size are too large, could this lead to nan values
                // decreasing step size to 100 ps, solves the problem in most cases
                if(std::isnan(x[0]) || std::isnan(x[1]) || std::isnan(dxdtout[0]) || std::isnan(dxdtout[1]))
                {
                    MSG_WARN(QString("Numeric: RKD5_OPT: Gradient for initial conditions is too large: "
                             "step size for calculation was decreased to 100 ps at t= t0 + %0").arg(t));

                    rkd5.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                            pl::_1 ,  // x
                                            pl::_2 ,  // dxdt
                                            pl::_3 ), // t
                                  x, dxdt, t, xout, dxdtout, 1E-10, xerr);

                }

                // check if temperature gradient is larger than 20 K
                // if yes, calculate more accurate solution
                if(dxdtout[0] * dt < -20.0)
                {
                    int n = ceil(abs(dxdtout[0] * dt) / 20.0);

                    // avoid odd numbers
                    if(n % 2 != 0) n++;

                    double dt_new = dt_internal / n;

                    //qDebug() << "N: " << n << " - dt_new: " <<  dt_new << "abs: " << abs(dxdtout[0] * dt);
                    //qDebug() << "BeforeLoop: " << t << dt << dxdt[0]*dt << dxdt[1]*dt;

                    int k;

                    for(k = 0; k < n; k++)
                    {
                        rkd5.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                                pl::_1 ,  // x
                                                pl::_2 ,  // dxdt
                                                pl::_3 ), // t
                                      x, dxdt, t, xout, dxdtout, dt_new, xerr);

                        t    = t + dt_new;
                        x    = xout;
                        dxdt = dxdtout;

                        //qDebug() << "IntLoop: " << t << dt << dxdt[0]*dt << dxdt[1]*dt;
                    }

                    //qDebug() << "AfterLoop: " << k << " - " << t << dt << dxdt[0]*dt << dxdt[1]*dt;
                }
                else // accept values
                {
                    x    = xout;
                    dxdt = dxdtout;
                }

                //qDebug() << "Temperature gradient: " << t << dxdt[0]*dt << dxdt[1]*dt;
            }

            t = t + dt_internal;

            //qDebug() << t << " - dt_internal: " << dt_internal << " - " << xout[0];

            break;


        case ODE_Algorithm::RKF78:

            for(int j = 0; j < ODEsolverStepSizeFactor; j++)
            {
                rkf78.do_step( std::bind(&HeatTransferModel::ode_sys, &ht_object,
                                     pl::_1 ,  // x
                                     pl::_2 ,  // dxdt
                                     pl::_3 ), // t
                           x, t, dt_internal);
            }

            t = t + dt_internal;

            //qDebug() << t << " - dt_internal: " << dt_internal << " - " << xout[0];
            //qDebug() << "Err: " << xerr[0] << xerr[1];

            break;
        }

        // x[0] - temperature
        signal.data[i]         = x[0];

//        if(i < 10)
//            qDebug() << "Num: " << t << dt << x[0] << x[1] << signal.data[i-1]-x[0];


        // x[1] - particle mass
        if(ht_object.sysFunc() == HeatTransferModel::dT_dM)
        {
            signal.dataDiameter[i] = ht_object.calculateDiameterFromMass(x[0], x[1]) * unit_m_to_nm;
        }
        // x10] - particle diameter
        else
        {
            signal.dataDiameter[i] = x[1] * unit_m_to_nm;
        }
    }

    if(timer)
    {
        // Timer: End
        p.end();
        //qDebug() << p.getResult().c_str();
    }

    // update progress bar
    Core::instance()->incProgressBar();

    return signal;
}


 /**
 * @brief Numeric::levmar Levenberg-Marquardt algorithm (inspired by "Numerical Recipes Third Edition")
 * @param fd FitData
 * @param ms ModelingSettings
 * @param fs FitSettings
 * @param ns NumericSettings
 */
void Numeric::levmar(FitRun::FitMode mode,
                     FitData *fd,
                     ModelingSettings *ms,
                     FitSettings *fs,
                     NumericSettings *ns)
{
    QList<FitParameter> fparams = fs->fitParameters();

    // weighting (consider standard deviation (stdev) during fitting)
    bool weighting = false;

    if(fs->weightingActive())
        weighting = true;

    // number of iterations
    int IT = ns->iterations();
    int NDONE = 4; // number of iterations with delta_chisquare < tol
    int done = 0; // counter for termination
    QString msg;

    // number of parameters
    int N = fparams.size();

    if(N == 0)
        return;

    // number of free parameters
    int NF = 0;

    // for now: hard-coded; TODO: performance test -> decision what to use or checkbox in GUI
    // SOLVER USED:
    // true:   Cholesky factorization (LDL^T x = b)
    // false:  Gauss-Jordon (A x = b <=> x = A^-1 b)
    bool cholesky = true;


    // free parameter list
    VecBool ia(boost::extents[N]);

    // parameter values
    VecDoub a(boost::extents[N]);
    VecDoub a_min(boost::extents[N]);
    VecDoub a_max(boost::extents[N]);
    VecDoub a_next(boost::extents[N]);


    // inititalize delta_a vector (differences between parameter variation)
    VecDoub da(boost::extents[N]);
    VecDoub da_min(boost::extents[N]);
    VecDoub da_max(boost::extents[N]);
    MatDoub dyda(boost::extents[N][fd->xdata.size()]);

    // data container for ydata from model
    QVector<double> ymod;
    QVector<double> ymod_delta_a;

    // levmar variables

    double h; // temp var for parameter variation
    double dy; // contains difference between model and data

    double chisquare;
    double prev_chisquare = -1.0;

    // damping factor for levmar (default values are set in NumericSettings)
    double lambda       = ns->lambda_init;      // start value
    double lambda_dec   = ns->lambda_decrease;  // if iteration result gets better
    double lambda_inc   = ns->lambda_increase;  // if iteration result gets worse
    double lambda_scale = ns->lambda_scaling;   // if parameter variation exceeds maxDelta, increase lambda

    double sigma = 1.0;
    double sigma2i = 1.0;
    double wt;


    // get initial fit parameter information
    for(size_t j=0; j<N; j++)
    {
        da[j]  = 0.0;

        a[j]        = fparams.at(j).value();
        a_next[j]   = fparams.at(j).value();
        a_min[j]    = fparams.at(j).lowerBound();
        a_max[j]    = fparams.at(j).upperBound();

        // check if free or hold and update number of free parameters (NF)
        ia[j]       = fparams.at(j).enabled();
        if(ia[j]) NF++;

        // maximum length of search direction
        da_max[j]   = fparams.at(j).maxDelta();
    }

    // variables dependent on number of free parameters (size <NFxNF>)
    MatDoub alpha(boost::extents[NF][NF]); // one-half times the Hessian matrix i.e. curvature matrix
    MatDoub alpha_lambda(boost::extents[NF][NF]);
    VecDoub beta(boost::extents[NF]);
    MatDoub covar(boost::extents[NF][NF]); // = alpha_inverse


    // TODO: final covar and alpha should contain all parameters (size <NxN>)
    // currently: only free parameters (size <NFxNF>)


    // iteration loop
    for(size_t iter=0; iter<IT; iter++)
    {

        if(Numeric::canceled)
        {
            MSG_ASYNC("Numeric::levmar canceled!", DEBUG);
            return;
        }

        // last pass, set damping factor to zero
        if(done == NDONE) lambda = 0.0;


        // set new parameters a[j] for this iteration from calculated delta_a[j]
        // this function does nothing for the first iteration (da[j] = 0.0)
        for(size_t j=0; j<N; j++)
        {
            // change only free parameters
            if(ia[j])
            {
                a[j] = a_next[j] - da[j];

               // reset to boundaries (avoid non-physical values)
               if (a[j] < a_min[j])        a[j] = a_min[j];
               else if(a[j] > a_max[j])    a[j] = a_max[j];
            }
        }
        // calculate initial model result (ymod) using parameter a[j]
        // for this iteration (first guess)
        ymod.clear();


        // get modeled data (QVector<double>) dependent on FitMode
        ymod = Numeric::modeledData(/* FitRun::FitMode */ mode,
                                    /* VecDoub */ a,
                                    /* FitData */ fd,
                                    /* ModelingSettings */ ms,
                                    /* FitSettings */ fs,
                                    /* NumericSettings */ ns);

        // calculate derivatives dyda[j] for parameters a[j]
        for(size_t j=0; j<N; j++)
        {
            //only for free parameters to save computational time
            if(ia[j])
            {
                // change parameter a[j] infinitesimally
                h = std::max(Numeric::EPS, std::abs(a[j])) * sqrt(Numeric::EPS); // avoid to small change if a[j] >> 1
                a[j] += h;

                // calculate all ymod

                ymod_delta_a = Numeric::modeledData(/* FitRun::FitMode */ mode,
                                                    /* VecDoub */ a,
                                                    /* FitData */ fd,
                                                    /* ModelingSettings */ ms,
                                                    /* FitSettings */ fs,
                                                    /* NumericSettings */ ns);

                // save dy[i]/da[j]
                for(size_t i = 0 ; i < fd->xdata.size(); i++)
                {
                    dyda[j][i] = (ymod[i] - ymod_delta_a[i]) / h;
                }

                // reset parameter a[j] to previous value
                a[j] -= h;
            }
        }

        // dyda is now the Jacobian (mxn) where m: datapoints n: parameters


        // calculate alpha (one-half times the Hessian matrix i.e. curvature matrix),
        // beta from delta_y[i] and dyda[k],
        // chisquare

        // inititalize alpha and beta and chisquare
        for(int j = 0; j < NF; j++)
        {
            for(int k = 0; k < NF; k++)
                alpha[j][k] = 0.0; // symmetric dim[NFxNF]

            beta[j]     = 0.0;  // vector dim[NF]
        }

        chisquare = 0.0;

        int ysize;

        if(mode == FitRun::TEMP_CAL)
            ysize = fd->ydatalist.size();
        else
            ysize = fd->ydata.size();

        for(int i = 0 ; i < ysize; i++)
        {
            if(mode == FitRun::TEMP_CAL)
                dy = ymod.at(i);
            else
            {
                dy = (fd->ydata.at(i) - ymod.at(i));

                //sigma = ymod.at(i);
            }

            // get standard deviation (default: sigma = 1.0)
            if(weighting)
                sigma = fd->stdev.at(i);

            sigma2i = 1.0 / (sigma*sigma);


            // free parameter index
            int ur = 0; // row
            int uc = 0; // col

            for(size_t j=0; j<N; j++)
            {
                if(ia[j])
                {
                    wt = dyda[j][i] * sigma2i;

                    uc = 0;
                    for(size_t k=0; k<N; k++)
                    {
                        if(ia[k])
                        {
                            //qDebug() << ur << uc;

                            // sum up for all data(i)
                            alpha[ur][uc] += wt * dyda[k][i]; // = alpha[j][k]
                            uc++;
                        }
                    }
                    beta[ur] += dy * wt;
                    ur++;
                }
            }

            chisquare += dy*dy*sigma2i;

        }
        // now we proceed with alpha, beta and chisquare
        // -> alpha is equal to half times the hessian

        // alter linearized fitting matrix by augmenting diagonal elements
        for(size_t j=0; j<NF; j++)
        {
            for(size_t k=0; k<NF; k++)
                alpha_lambda[j][k] = alpha[j][k];

            alpha_lambda[j][j] = alpha[j][j] * (1.0 + lambda);
        }

        // reset delta_a
        for(size_t j=0; j<N; j++)
        {
            //da_data[k] = 0.0;
            da[j]  = 0.0;  // parameter change for this iteration dim[N]
        }


        // solve equation system for Newton direction
        // 1) single equation for one free parameter

        if(NF <= 1)
        {
            for(size_t j=0; j<N; j++)
            {
                // only for free parameters
                if(ia[j])
                {
                    da[j] = beta[0] / alpha_lambda[0][0];
                }
            }
        }
        // 2) Cholesky factorization (LDL^T x = b)
        else if(cholesky)
        {
            VecDoub solution(boost::extents[NF]);

            MatDoub L(boost::extents[NF][NF]);
            MatDoub D(boost::extents[NF][NF]);


            // LDLT (Performance Test: 5.7 s)
            //Numeric::cholesky_LDLT(alpha_lambda, L, D);
            //Numeric::cholesky_solve_LDLT(L, D, beta, solution);

            // LLT (Performance Test: 5.6 s)
            Numeric::cholesky_LLT(alpha_lambda, L);
            Numeric::cholesky_solve_LLT(L, beta, solution);

            int k = 0;
            for(size_t j=0; j<N; j++)
            {
                // only for free parameters
                if(ia[j])
                {
                    da[j] = solution[k];
                    k++;
                }
            }
        }
        // 3) Gauss-Jordon (A x = b <=> x = A^-1 b)
        else
        {
            // calculate inverse matrix for linear solution of equation system
            if(NF <= 1)
                covar[0][0] = 1.0 / alpha_lambda[0][0];
            else if(!Numeric::invertMatrix(alpha_lambda, covar))
            {
                msg = QString("Iteration: %0 - Numeric::levmar_planck: alpha is singular/not invertible")
                        .arg(iter);

                MSG_ASYNC(msg, WARNING);

                // cancel fitting
                return;
            }

            // covar is now the covariance matrix (= alpha inverted)

            // solve equation system: calculate delta_a = alpha^-1 * beta = covar * beta
            for(size_t j=0; j<N; j++)
            {
                // only for free parameters
                if(ia[j])
                {
                    for(size_t k=0; k<NF; k++)
                    {
                        da[j] += covar[j][k] * beta[k];
                    }
                }
            }
        }

        //  evaluate this iteration
        if(prev_chisquare == -1.0)
        {
            // initialization
            prev_chisquare = chisquare;

            for(size_t k=0; k<N; k++)
            {
                a_next[k] = a[k];
            }
        }
        else
        {
            // check if solution converged and increase counter
            if(abs(chisquare - prev_chisquare) < std::max(Numeric::TOL, Numeric::TOL*chisquare))
            {
                done++;
            }

            // evaluate and change lambda
            // for very small lambda, the Levenberg-Marquardt method
            // reverts to the Gauss-Newton Method
            if(chisquare < prev_chisquare)
            {
                // if better, decrease lambda
                //lambda *= chisquare / prev_chisquare; // test
                lambda *= lambda_dec;
                //lambda *= lambda_dec * iter / IT;

                // save (best) chisquare for next iteration
                prev_chisquare = chisquare;

                // accept new solution
                for(size_t k=0; k<N; k++)
                {
                    a_next[k] = a[k];
                }
            }
            else
            {
                // if worse, increase lambda and continue
                lambda *= lambda_inc;
            }
        }


        // limit delta a for stability, to prevent non physical values (i.e. > 6000 K or < 1000 K)
        // if vector da[k] exceeds limits of da_max[k], reduce da[k]
        double da_scaling = 1.0;

        for(size_t k=0; k<N; k++)
        {
            if(ia[k])
            {
                if(abs(da[k]) > da_max[k])
                {
                    if(abs(da_max[k] / da[k]) < da_scaling)
                    {
                        da_scaling = abs(da_max[k] / da[k]);

                        // increase damping constant
                        lambda *= lambda_scale;
                    }
                }
            }
        }

        // multiply all parameters by calculated scaling constant
        for(size_t k=0; k<N; k++)
        {
            if(ia[k]) da[k] = da_scaling * da[k];
        }


        // write iteration result
        FitIterationResult fres = FitIterationResult(2+N*2);
        fres[0] = chisquare;
        fres[1] = lambda;

        for(int n = 0; n < N; n++)
        {
            fres[2+2*n] = a[n];
            fres[2+2*n+1] = -da[n];
        }
        fd->addIterationResult(fres);

        // stop if converged
        if(done == NDONE) break;

    } // iterations
}

/**
 * @brief Numeric::modeledData this function provides to modeled data and is used in levmar
 * @param mode  determines which model is used
 * @param a     parameters used for modeling
 * @param fd    fit data
 * @param ms    modeling settings
 * @param fs    fit settings
 * @param ns    numeric settings
 * @return
 */
QVector<double> Numeric::modeledData(FitRun::FitMode mode,
                                     VecDoub a,
                                     FitData *fd,
                                     ModelingSettings *ms,
                                     FitSettings *fs,
                                     NumericSettings *ns)
{
    if(mode == FitRun::PSIZE)
    {
        // Parameters:
        // a[0]: particle size [m]
        // a[1]: gas temperature [K]
        // a[2]: start temperature (peak) [K]

        double T_start =  a[2];

        ms->heatTransferModel()->setProcessConditions(ms->processPressure(),
                                                     a[1]);

        // integration step size from FitData
        double dt = fd->dataSignal().dt;

        // TODO: if ns->integrationStepSize() > dt -> match data->dt and ns->dt

        Signal sig = Numeric::solveODE(
                                T_start,
                                a[0],
                                *ms->heatTransferModel(),
                                fd->xdata.size(),
                                dt,
                                ns);
        return sig.data;
    }
    // EXPERIMENTAL ROUTINE: used by TemperatureCalculator "Test"
    else if(mode == FitRun::TEMP_CAL)
    {
        // Parameters:
        // a[k]: cal factor CH k [-]


        QVector<double> ymod;

        LIISettings ls          = fd->mrun_LIISettings;
        Material material_spec  = ms->materialSpec();
        QString sourceEm        = fs->sourceEm();

        QVector<int> l;
        for(int j = 0; j < ls.channels.size(); j++)
            l << ls.channels.at(j).wavelength;


        int fpsize = fs->fitParameters().size();

        QVector<double> y, r;

        double sum, mean, stdev;
        double T_spectrum;

        double startTemperature = 2500;
        double startC = 1.0;


        // FitParameters
        QList<FitParameter> fparams;
        fparams << FitParameter(0, "Start temperature", "K", startTemperature, 1000.0, 5000.0, 500.0);
        fparams << FitParameter(1, "C", "-", startC, 1E-30, 1E30, 10000.0);

        // FitData
        FitData* sub_fd= new FitData();
        sub_fd->xdata = fd->xdata; // active wavelengths

        // FitSettings
        FitSettings* sub_fs = new FitSettings;
        sub_fs->setFitParameters(fparams);
        sub_fs->setBandpassIntegrationActive(false);
        sub_fs->setWeightingActive(false);
        sub_fs->setSourceEm(sourceEm);

        NumericSettings* sub_ns = new NumericSettings;
        sub_ns->setIterations(20);
        sub_ns->lambda_init        = 0.1;
        sub_ns->lambda_decrease    = 0.2;
        sub_ns->lambda_increase    = 10.0;
        sub_ns->lambda_scaling     = 10.0;

        for(size_t i = 0 ; i < fd->ydatalist.size(); i++)
        {
            y.clear();
            y = fd->ydatalist.at(i);

            for(int j = 0; j < fpsize; j++)
                y[j] = y[j] * a[j];

            r.clear();

            //---------------------
            // calculate spectrum
            //---------------------

            // init fit data

            // get wavelengths/intensities from general fitData

            sub_fd->ydata = y;

            sub_fd->clearResults();

            Numeric::levmar(FitRun::TEMP,
                            sub_fd,
                            ms,
                            sub_fs,
                            sub_ns);

            r << sub_fd->iterationResultList().last().at(2);

            //---------------------
            // calculate 2C-ratios
            //---------------------

            for(int c1 = 0; c1 < fpsize-1; c1++)
            {
                for(int c2 = 1; c2 < fpsize; c2++)
                {
                    // TODO: check if lambda is equal
                    if(l[c1] == l[c2])
                        continue;

                    r << Temperature::calcTwoColor(y[c1], y[c2], l[c1], l[c2], material_spec, sourceEm);

                }
            }

            // four-color
//            r << Temperature::calcTwoColor(y[0], y[2], l[0], l[2], m, sourceEm);
//            r << Temperature::calcTwoColor(y[0], y[3], l[0], l[3], m, sourceEm);
//            r << Temperature::calcTwoColor(y[1], y[2], l[1], l[2], m, sourceEm);
//            r << Temperature::calcTwoColor(y[1], y[3], l[1], l[3], m, sourceEm);
//            r << Temperature::calcTwoColor(y[2], y[3], l[2], l[3], m, sourceEm);

            // three-color
//            r << Temperature::calcTwoColor(y[0], y[1], l[0], l[1], m, sourceEm);
//            r << Temperature::calcTwoColor(y[0], y[2], l[0], l[2], m, sourceEm);
//            r << Temperature::calcTwoColor(y[1], y[2], l[1], l[2], m, sourceEm);

            sum = 0.0;
            for(int k = 0; k < r.size(); k++)
                sum += r.at(k);

            mean = sum / r.size();

            stdev = 0.0;
            for(int k = 0; k < r.size(); k++)
                stdev += pow((r.at(k)- mean), 2);

            stdev = sqrt(1.0/double(r.size()-1) * stdev);

            //qDebug() << std;

            ymod << (stdev /  mean);

        }
        return ymod;

    }
    else // FitMode::TEMP
    {
        Material material_spec  = ms->materialSpec();
        QString sourceEm        = fs->sourceEm();

        // Parameters:
        // a[0]: temperature [K]
        // a[1]: scaling factor [-]

        QVector<double> ymod;

        for(size_t i = 0 ; i < fd->xdata.size(); i++)
        {
            if(fs->bandpassIntegration() == true)
            {
                ymod.append(Temperature::calcPlanckIntensityBandpass(int(fd->xdata.at(i)),
                                                                     fd->bandwidths.at(i),
                                                                     a[0],
                                                                     a[1],
                                                                     material_spec,
                                                                     sourceEm));
            }
            else
            {
                ymod.append(Temperature::calcPlanckIntensity(fd->xdata.at(i)*1E-9,
                                                             a[0],
                                                             a[1],
                                                             material_spec,
                                                             sourceEm));
            }
        }
        return ymod;
    }
}



/**
* @brief Numeric::levmar Levenberg-Marquardt algorithm (inspired by "Numerical Recipes Third Edition")
* @param fd FitData
* @param ms ModelingSettings
* @param fs FitSettings
* @param ns NumericSettings
*/
void Numeric::levmar_test(FitRun::FitMode mode,
                    FitData *fd,
                    ModelingSettings *ms,
                    FitSettings *fs,
                    NumericSettings *ns)
{
   QList<FitParameter> fparams = fs->fitParameters();

   // weighting (consider standard deviation (stdev) during fitting)
   bool weighting = false;

   if(fs->weightingActive())
       weighting = true;
/**********************************************************************************/
   // number of iterations
   int IT = ns->iterations();
   int NDONE = 4; // number of iterations with delta_chisquare < tol
   int done = 0; // counter for termination
   QString msg;

   // number of parameters
   int N = fparams.size();

   if(N == 0)
       return;

   // number of free parameters
   int NF = 0;

   // for now: hard-coded; TODO: performance test -> decision what to use or checkbox in GUI
   // SOLVER USED:
   // true:   Cholesky factorization (LDL^T x = b)
   // false:  Gauss-Jordon (A x = b <=> x = A^-1 b)
   bool cholesky = true;
/**********************************************************************************/

   int ysize;

   if(mode == FitRun::TEMP_CAL)
       ysize = fd->ydatalist.size();
   else
       ysize = fd->ydata.size();

   // free parameter list
   VecBool ia(boost::extents[N]);

   // parameter values
   VecDoub a(boost::extents[N]);
   VecDoub a_min(boost::extents[N]);
   VecDoub a_max(boost::extents[N]);
   VecDoub a_next(boost::extents[N]);


   // inititalize delta_a vector (differences between parameter variation)
   VecDoub da(boost::extents[N]);
   VecDoub da_min(boost::extents[N]);
   VecDoub da_max(boost::extents[N]);
   MatDoub dyda(boost::extents[N][ysize]);

   // data container for ydata from model
   QVector<double> ymod;
   QVector<double> ymod_delta_a;

   // levmar variables
/**********************************************************************************/
   double h; // temp var for parameter variation
   double dy; // contains difference between model and data

   double chisquare;
   double prev_chisquare = -1.0;

   // damping factor for levmar (default values are set in NumericSettings)
   double lambda       = ns->lambda_init;      // start value
   double lambda_dec   = ns->lambda_decrease;  // if iteration result gets better
   double lambda_inc   = ns->lambda_increase;  // if iteration result gets worse
   double lambda_scale = ns->lambda_scaling;   // if parameter variation exceeds maxDelta, increase lambda

   double sigma = 1.0;
   double sigma2i = 1.0;
   double wt;
/**********************************************************************************/

   // get fit parameter information
   for(size_t j=0; j<N; j++)
   {
       da[j]  = 0.0;

       a[j]        = fparams.at(j).value();
       a_next[j]   = fparams.at(j).value();
       a_min[j]    = fparams.at(j).lowerBound();
       a_max[j]    = fparams.at(j).upperBound();

       // check if free or hold and update number of free parameters (NF)
       ia[j]       = fparams.at(j).enabled();
       if(ia[j]) NF++;

       // maximum length of search direction
       da_max[j]   = fparams.at(j).maxDelta();
   }

   // variables dependent on number of free parameters (size <NFxNF>)
   MatDoub alpha(boost::extents[NF][NF]); // one-half times the Hessian matrix i.e. curvature matrix
   MatDoub alpha_lambda(boost::extents[NF][NF]);
   VecDoub beta(boost::extents[NF]);
   MatDoub covar(boost::extents[NF][NF]); // = alpha_inverse


   // TODO: final covar and alpha should contain all paramters (size <NxN>)

/**********************************************************************************/
   // iteration loop
   for(size_t iter=0; iter<IT; iter++)
   {

       if(Numeric::canceled)
       {
           MSG_ASYNC("Numeric::levmar canceled!", DEBUG);
           return;
       }

       // last pass, set damping factor to zero
       if(done == NDONE) lambda = 0.0;


       // set new parameters a[j] for this iteration from calculated delta_a[j]
       // this function does nothing for the first iteration (da[j] = 0.0)
       for(size_t j=0; j<N; j++)
       {
           // change only free parameters
           if(ia[j])
           {
               a[j] = a_next[j] - da[j];

              // reset to boundaries (avoid non-physical values)
              if (a[j] < a_min[j])        a[j] = a_min[j];
              else if(a[j] > a_max[j])    a[j] = a_max[j];
           }
       }
       // calculate initial model result (ymod) using parameter a[j]
       // for this iteration (first guess)
       ymod.clear();

/**********************************************************************************/
       // get modeled data (QVector<double>) dependent on FitMode
       ymod = Numeric::modeledData(/* FitRun::FitMode */ mode,
                                   /* VecDoub */ a,
                                   /* FitData */ fd,
                                   /* ModelingSettings */ ms,
                                   /* FitSettings */ fs,
                                   /* NumericSettings */ ns);

       // calculate derivatives dyda[j] for parameters a[j]
       for(size_t j=0; j<N; j++)
       {
           //only for free parameters to save computational time
           if(ia[j])
           {
               // change parameter a[j] infinitesimally
               h = std::max(Numeric::EPS, std::abs(a[j])) * sqrt(Numeric::EPS); // avoid to small change if a[j] >> 1
               a[j] += h;

               // calculate all ymod

               ymod_delta_a = Numeric::modeledData(/* FitRun::FitMode */ mode,
                                                   /* VecDoub */ a,
                                                   /* FitData */ fd,
                                                   /* ModelingSettings */ ms,
                                                   /* FitSettings */ fs,
                                                   /* NumericSettings */ ns);

               // save dy[i]/da[j]
               for(size_t i = 0 ; i < ysize; i++)
               {
                   dyda[j][i] = -1* (ymod[i] - ymod_delta_a[i]) / h;
               }

               // reset parameter a[j] to previous value
               a[j] -= h;
           }
       }

       // dyda is now the Jacobian (mxn) where m: datapoints n: parameters
/**********************************************************************************/

       // calculate alpha (one-half times the Hessian matrix i.e. curvature matrix),
       // beta from delta_y[i] and dyda[k],
       // chisquare

       // inititalize alpha and beta and chisquare
       for(int j = 0; j < NF; j++)
       {
           for(int k = 0; k < NF; k++)
               alpha[j][k] = 0.0; // symmetric dim[NFxNF]

           beta[j]     = 0.0;  // vector dim[NF]
       }

       chisquare = 0.0;


       for(int i = 0 ; i < ysize; i++)
       {
           if(mode == FitRun::TEMP_CAL)
               dy = ymod.at(i);
           else
               dy = (fd->ydata.at(i) - ymod.at(i));

           // get standard deviation (default: sigma = 1.0)
           if(weighting)
               sigma = fd->stdev.at(i);

           sigma2i = 1.0 / (sigma*sigma);

/**********************************************************************************/
           // free parameter index
           int ur = 0; // row
           int uc = 0; // col

           for(size_t j=0; j<N; j++)
           {
               if(ia[j])
               {
                   wt = dyda[j][i] * sigma2i;

                   uc = 0;
                   for(size_t k=0; k<N; k++)
                   {
                       if(ia[k])
                       {
                           //qDebug() << ur << uc;

                           // sum up for all data(i)
                           alpha[ur][uc] += wt * dyda[k][i]; // = alpha[j][k]
                           uc++;
                       }
                   }
                   beta[ur] += dy * wt;
                   ur++;
               }
           }

           chisquare += dy*dy*sigma2i;
/**********************************************************************************/
       }
       // now we proceed with alpha, beta and chisquare
       // -> alpha is equal to half times the hessian

       // alter linearized fitting matrix by augmenting diagonal elements
       for(size_t j=0; j<NF; j++)
       {
           for(size_t k=0; k<NF; k++)
               alpha_lambda[j][k] = alpha[j][k];

           alpha_lambda[j][j] = alpha[j][j] * (1.0 + lambda);
       }

       // reset delta_a
       for(size_t j=0; j<N; j++)
       {
           //da_data[k] = 0.0;
           da[j]  = 0.0;  // parameter change for this iteration dim[N]
       }
/**********************************************************************************/

       // solve equation system for Newton direction
       // 1) single equation for one free parameter

       if(NF <= 1)
       {
           for(size_t j=0; j<N; j++)
           {
               // only for free parameters
               if(ia[j])
               {
                   da[j] = beta[0] / alpha_lambda[0][0];
               }
           }
       }
       // 2) Cholesky factorization (LDL^T x = b)
       else if(cholesky)
       {
           VecDoub solution(boost::extents[NF]);

           MatDoub L(boost::extents[NF][NF]);
           MatDoub D(boost::extents[NF][NF]);
/**********************************************************************************/

           // LDLT (Performance Test: 5.7 s)
           //Numeric::cholesky_LDLT(alpha_lambda, L, D);
           //Numeric::cholesky_solve_LDLT(L, D, beta, solution);

           // LLT (Performance Test: 5.6 s)
           Numeric::cholesky_LLT(alpha_lambda, L);
           Numeric::cholesky_solve_LLT(L, beta, solution);

           int k = 0;
           for(size_t j=0; j<N; j++)
           {
               // only for free parameters
               if(ia[j])
               {
                   da[j] = solution[k];
                   k++;
               }
           }
       }
       // 3) Gauss-Jordon (A x = b <=> x = A^-1 b)
       else
       {
           // calculate inverse matrix for linear solution of equation system
           if(NF <= 1)
               covar[0][0] = 1.0 / alpha_lambda[0][0];
           else if(!Numeric::invertMatrix(alpha_lambda, covar))
           {
               msg = QString("Iteration: %0 - Numeric::levmar_planck: alpha is singular/not invertible")
                       .arg(iter);

               MSG_ASYNC(msg, WARNING);
/**********************************************************************************/
               // cancel fitting
               return;
           }

           // covar is now the covariance matrix (= alpha inverted)

           // solve equation system: calculate delta_a = alpha^-1 * beta = covar * beta
           for(size_t j=0; j<N; j++)
           {
               // only for free parameters
               if(ia[j])
               {
                   for(size_t k=0; k<NF; k++)
                   {
                       da[j] += covar[j][k] * beta[k];
                   }
               }
           }
       }
/**********************************************************************************/
       //  evaluate this iteration
       if(prev_chisquare == -1.0)
       {
           // initialization
           prev_chisquare = chisquare;

           for(size_t k=0; k<N; k++)
           {
               a_next[k] = a[k];
           }
       }
       else
       {
           // check if solution converged and increase counter
           if(abs(chisquare - prev_chisquare) < std::max(Numeric::TOL, Numeric::TOL*chisquare))
           {
               done++;
           }
/**********************************************************************************/
           // evaluate and change lambda
           // for very small lambda, the Levenberg-Marquardt method
           // reverts to the Gauss-Newton Method
           if(chisquare < prev_chisquare)
           {
               // if better, decrease lambda               
               lambda *= lambda_dec;

               // save (best) chisquare for next iteration
               prev_chisquare = chisquare;

               //qDebug() << "decrease " << lambda << chisquare << prev_chisquare;

               // accept new solution
               for(size_t k=0; k<N; k++)
               {
                   a_next[k] = a[k];
               }
           }
           else
           {
               // if worse, increase lambda and continue
               lambda *= lambda_inc;
               //qDebug() << "increase " << lambda << chisquare << prev_chisquare;
           }
       }

/**********************************************************************************/
       // limit delta a for stability, to prevent non physical values (i.e. > 6000 K or < 1000 K)
       // if vector da[k] exceeds limits of da_max[k], reduce da[k]
       double da_scaling = 1.0;

       for(size_t k=0; k<N; k++)
       {
           if(ia[k])
           {
               if(abs(da[k]) > da_max[k])
               {
                   if(abs(da_max[k] / da[k]) < da_scaling)
                   {
                       da_scaling = abs(da_max[k] / da[k]);

                       // increase damping constant
                       lambda *= lambda_scale;
                   }
               }
           }
       }
/**********************************************************************************/
       // multiply all parameters by calculated scaling constant
       for(size_t k=0; k<N; k++)
       {
           if(ia[k]) da[k] = da_scaling * da[k];
       }


       // write iteration result
       FitIterationResult fres = FitIterationResult(2+N*2);
       fres[0] = chisquare;
       fres[1] = lambda;

       for(int n = 0; n < N; n++)
       {
           fres[2+2*n] = a[n];
           fres[2+2*n+1] = -da[n];
       }
       fd->addIterationResult(fres);

       // stop if converged
       if(done == NDONE) break;

   } // iterations
}



/**
 * @brief Numeric::debugCholesky debug output for levmar
 * @param alpha_lambda
 * @param L
 * @param D
 * @param covar
 * @param da
 * @param solution
 * @param beta
 */
void Numeric::debugCholesky(MatDoub alpha_lambda, MatDoub L, MatDoub Linv, MatDoub D, MatDoub covar, VecDoub da, VecDoub solution, VecDoub beta)
{
    qDebug() << "Alpha lambda";
    qDebug() << alpha_lambda[0][0] << alpha_lambda[0][1];
    qDebug() << alpha_lambda[1][0] << alpha_lambda[1][1];

    qDebug() << "L";
    qDebug() << L[0][0] << L[0][1];
    qDebug() << L[1][0] << L[1][1];

    qDebug() << "Linv";
    qDebug() << Linv[0][0] << Linv[0][1];
    qDebug() << Linv[1][0] << Linv[1][1];

    qDebug() << "D";
    qDebug() << D[0][0] << D[0][1];
    qDebug() << D[1][0] << D[1][1];

    qDebug() << "covar";
    qDebug() << covar[0][0] << covar[0][1];
    qDebug() << covar[1][0] << covar[1][1];

    qDebug() << "da";
    qDebug() << da[0] << da[1];

    qDebug() << "solution";
    qDebug() << solution[0] << solution[1];

    qDebug() << "beta";
    qDebug() << beta[0] << beta[1];
}


/**
 * @brief Numeric::debugCovarianceMatrix
 */
void Numeric::debugCovarianceMatrix()
{
    int NT = 4;

    CovMatrix test(NT);


    qDebug() << "covmatrix test" << test.shape();

    int k = 1;
    for(int u = 0; u < test.shape(); u++)
    {
        for(int v = 0; v <= u; v++)
        {
            test.set(u,v, double((u+1)*10+(v+1)));
        }
    }

    qDebug() << test(0,0) << test(0,1) << test(0,2) << test(0,3);
    qDebug() << test(1,0) << test(1,1) << test(1,2) << test(1,3);
    qDebug() << test(2,0) << test(2,1) << test(2,2) << test(2,3);
    qDebug() << test(3,0) << test(3,1) << test(3,2) << test(3,3);
}



/*****************************
 *
 *      HELPER FUNCTIONS
 *
 *      see also: Numerical Recipes Chapter 15.4 "General Linear Least Squares"
 *      gaussj - Linear equation solution by Gauss-Jordon elimination (2.1.1)
 *
 *      Numeric::invertMatrix:
 *      http://www.virtual-maxim.de/matrix-invertieren-mit-gaus-jordan-algorithmus/
 *      http://www.virtual-maxim.de/matrix-invertieren-in-c-plus-plus/

 **/


double Numeric::integrate_trapezoid(Property prop, double T0, double T1)
{
    // Newton-Cotes formulas
    // https://en.wikipedia.org/wiki/Newton%E2%80%93Cotes_formulas
    // Trapezoid rule: Single Integral = (b-a)*(f(a) + f(b))/2

    // avoid any NaN values caused by division by zero
    if(T0 <= 0.0)
        T0 = 0.001;

    // init vars
    double a, b, y_a, y_b;
    double res = 0.0;

    // step size (0.1 K accuracy)
    double h = 0.1;

    // determine number of sub intervals
    int n = floor((T1 - T0)  / h);

    // loop over (n - 1) values
    for(int i = 0; i <= (n-1); i++)
    {

        a = T0 + h*i ;
        b = T0 + h*(i+1);

        if(prop(a) < 0)
            continue;

        y_a = prop(a);
        y_b = prop(b);

        res = res + (b - a) * ((y_a + y_b) / 2.0);

        //qDebug() << res << " - " << y_a << " " << y_b << " " << T0;
    }

    return res;
}


double Numeric::integrate_trapezoid_func(std::function<double(double)> func, double T0, double T1, int n)
{
    // Newton-Cotes formulas
    // https://en.wikipedia.org/wiki/Newton%E2%80%93Cotes_formulas
    // Trapezoid rule: Single Integral = (b-a)*(f(a) + f(b))/2

    // at least two datapoints
    if(n <= 1)
        return 0.0;

    // avoid any NaN values caused by division by zero
    if(T0 <= 0.0)
        T0 = 0.001;


    // init vars
    double a, b, y_a, y_b;
    double res = 0.0;

    // step size (accuracy)
    double h = abs((T1 - T0))  / double(n);

    // loop over (n - 1) values
    for(int i = 0; i <= (n-1); i++)
    {

        a = T0 + h*i ;
        b = T0 + h*(i+1);

        if(func(a) < 0)
            continue;

        y_a = func(a);
        y_b = func(b);

        res = res + (b - a) * ((y_a + y_b) / 2.0);

        //qDebug() << res << " - " << y_a << " " << y_b << " " << T0;
    }

    return res;
}


double Numeric::integrate_trapezoid_func(std::function<double(double)> func, double T0, double T1, double acc)
{
    // Newton-Cotes formulas
    // https://en.wikipedia.org/wiki/Newton%E2%80%93Cotes_formulas
    // Trapezoid rule: Single Integral = (b-a)*(f(a) + f(b))/2

    // avoid any NaN values caused by division by zero
    if(T0 <= 0.0)
        T0 = 0.001;

    // init vars
    double a, b, y_a, y_b;
    double res = 0.0;

    // step size (accuracy)
    double h = acc;

    // determine number of sub intervals
    int n = floor(abs((T1 - T0))  / h);

    // loop over (n - 1) values
    for(int i = 0; i <= (n-1); i++)
    {

        a = T0 + h*i ;
        b = T0 + h*(i+1);

        if(func(a) < 0)
            continue;

        y_a = func(a);
        y_b = func(b);

        res = res + (b - a) * ((y_a + y_b) / 2.0);

        //qDebug() << res << " - " << y_a << " " << y_b << " " << T0;
    }

    return res;
}


/**
 * @brief Numeric::swapLine Swaps two lines of a NxM matrix
 * @param mat NxM matrix
 * @param line1 Index of line 1
 * @param line2 Index of line 2
 * @return false if line 1 or line 2 aren't inside the matrix
 */
//template <size_t N,size_t M>
//bool Numeric::swapLine(double mat[N][M], size_t line1, size_t line2)
bool Numeric::swapLine(MatDoub mat, size_t line1, size_t line2)
{
    int N = mat.shape()[0];
    int M = mat.shape()[1];

    if(line1 >= N || line2 >= N)
        return false;

    for(size_t i = 0; i < M; ++i)
    {
        double t = mat[line1][i];
        mat[line1][i] = mat[line2][i];
        mat[line2][i] = t;
    }

    return true;
}


/**
 * @brief Numeric::invertMatrix inverts NxN matrix with Gauß-Jordon-algorithm
 * @param mat Input matrix
 * @param inv Inverse of input matrix
 * @return false if matrix is not invertible
 */
//template <size_t N>
//bool Numeric::invertMatrix(const double mat[N][N], double inv[N][N])
bool Numeric::invertMatrix(const MatDoub mat, MatDoub &inv)
{
    // Create Nx2N matrix for Gauß-Jordan-algorithm
    //double A[N][2*N];
    size_t N = mat.shape()[0];

    MatDoub A(boost::extents[N][2*N]);

    for(size_t i = 0; i < N; ++i)
    {
        for(size_t j = 0; j < N; ++j)
            A[i][j] = mat[i][j];
        for(size_t j = N; j < 2*N; ++j)
            A[i][j] = (i==j-N) ? 1.0 : 0.0;
    }

    // Gauß-algorithm
    for(size_t k = 0; k < N-1; ++k)
    {
        // Swap lines, if Pivot-element is zero
        if(A[k][k] == 0.0)
        {
            for(size_t i = k+1; i < N; ++i)
            {
                if(A[i][k] != 0.0)
                {
                    Numeric::swapLine(A,k,i);
                    break;
                }
                else if(i==N-1)
                    return false; // There is no element != 0
            }
        }

        // Eliminate entries below the Pivot-element
        for(size_t i = k+1; i < N; ++i)
        {
            double p = A[i][k]/A[k][k];
            for(size_t j = k; j < 2*N; ++j)
                A[i][j] -= A[k][j]*p;
        }
    }

    // Calculate determinant of the triangular matrix
    double det = 1.0;
    for(size_t k = 0; k < N; ++k)
        det *= A[k][k];

    if(det == 0.0)	// determinant is == 0 -> matrix not invertible
        return false;

    // Execute Jordan-part of the algorithm
    for(size_t k = N-1; k > 0; --k)
    {
        for(int i = k-1; i >= 0; --i)
        {
            double p = A[i][k]/A[k][k];
            for(size_t j = k; j < 2*N; ++j)
                A[i][j] -= A[k][j]*p;
        }
    }

    // Normalize entries of the left matrix to one and create inverse matrix (inv)
    for(size_t i = 0; i < N; ++i)
    {
        const double f = A[i][i];
        for(size_t j = N; j < 2*N; ++j)
        {
            inv[i][j-N] = A[i][j]/f;           
        }
    }

    return true;
}


/***
 * http://nm.mathforcollege.com/
 *
 * Holistic Numerical Methods
 * Transforming Numerical Methods Education for the STEM Undergraduate
 **/


/**
 * @brief Numeric::cholesky_LLT Cholesky decomposition A = LL^T
 * @param A Matrix0
 * @param L lower triangular matrix
 * @return
 */
bool Numeric::cholesky_LLT(const MatDoub A, MatDoub &L)
{
    size_t N = A.shape()[0];
    size_t M = A.shape()[1];

    // check for symmetry
    if( N != M)
        return false;

    double sum;
    int i,j,k;

    for(i = 0; i < N; i++)
    {
        for(j = 0; j <= i; j++)
        {
             sum = 0.0;

             // this loop is executed for k > 0
             for(k = 0; k < i; k++)
                sum += L[i][k]*L[j][k];


             if(i == j)
             {
                 // check if matrix is positive definite
                if((A[i][i] - sum) <= 0.0)
                     return false;

                L[i][i] = sqrt(A[i][i] - sum);

              }
            else
                L[i][j] = 1.0 / L[j][j] * (A[i][j] - sum);
         }
    }
    return true;
}


/**
 * @brief Numeric::cholesky_solve_LLT solves linear equation system LL^T x = b
 *  1) Forward solution (L y = b)
 *  3) Backward solution ( L^T x = y)
 * @param L lower triangular matrix
 * @param b input vector
 * @param x solution is saved in this vector
 * @return
 */
bool Numeric::cholesky_solve_LLT(const MatDoub L, const VecDoub b, VecDoub &x)
{
    // TODO: include check:
     // - if matrix is positive definite: if yAy^T > 0 for any given vector [y]nx1 != 0

    size_t N = L.shape()[0];
    size_t M = L.shape()[1];

    // check for symmetry
    if( N != M)
        return false;

     VecDoub y(boost::extents[N]);

     int i,k;
     double sum;


     // 1) Forward solution (L y = b)

     for(i = 0; i < N; i++)
     {
         sum = 0.0;
         for(k = 0; k < i; k++)
             sum += L[i][k] * y[k];
         y[i] = (b[i] - sum) / L[i][i];
     }

     // 3) Backward solution ( L^T x = y)
     for(i = N-1; i >= 0; i--)
     {
         sum = 0.0;
         for(k = i; k < N; k++)
             sum += L[k][i] * x[k];
         x[i] = (y[i] - sum) / L[i][i];
     }

//     qDebug() << "LLT";
//     qDebug() << "x";
//     qDebug() << x[0] << x[1];
//     qDebug() << "y";
//     qDebug() << y[0] << y[1];

     return true;

}


/**
 * @brief Numeric::cholesky_inverse_A calculates the inverse of A, the matrix
 * whose Cholesky decomposition has been calculated (A = LL^T)
 * Source: NumericalRecipes - Chapter 2.9 - Cholesky decomposition - inverse
 * @param L lower triangular matrix from Cholesky decomposition
 * @param Ainv inverse of A
 * @return
 */
bool Numeric::cholesky_inverse_A(const MatDoub L, MatDoub &Ainv)
{
    size_t N = L.shape()[0];
    size_t M = L.shape()[1];

    if( N != M)
        return false;

    double sum;
    int i,j,k;

    for(i = 0; i < N; i++)
        for(j = 0; j <= i; j++)
        {
            sum = (i==j ? 1.0 : 0.0);

            for(k = i-1; k >= j; k--)
                sum -= L[i][k] * Ainv[j][k];
            Ainv[j][i] = sum / L[i][i];
        }

    for(i = N-1; i >= 0; i--)
        for(j = 0; j <= i; j++)
        {
            sum = (i<j ? 0.0 : Ainv[j][i]);

            for(k = i+1; k < N; k++)
                sum -= L[k][i] * Ainv[j][k];
            Ainv[i][j] = Ainv[j][i] = sum / L[i][i];
        }

    return true;
}


/**
 * @brief Numeric::cholesky_LDLT Cholesky decomposition A = LDL^T
 * @param A Matrix0
 * @param L lower triangular matrix
 * @param D diagonal matrix
 * @return
 */
bool Numeric::cholesky_LDLT(const MatDoub A, MatDoub &L, MatDoub &D)
{
    size_t N = A.shape()[0];
    size_t M = A.shape()[1];

    if( N != M)
        return false;

    double sum;
    int i,j,k;

    for(k = 0; k < N; k++)
    {
        sum = 0.0;

        // this loop is executed for k > 0
        for(j = 0; j < k; j++)
            sum += L[k][j]*L[k][j]*D[j][j];

        D[k][k] = A[k][k] - sum;
        L[k][k] = 1.0;

        // check if D is positive definite
        if(D[k][k] < Numeric::EPS * A[k][k])
            return false;

        for(i = k+1; i < N; i++)
        {
            sum = 0.0;
            for(j = 0; j < (k-1); j++)
                sum += L[i][j]*D[j][j]*L[k][j];
            L[i][k] = (A[i][k] - sum) / D[k][k];
        }
    }

    return true;
}


/**
 * @brief Numeric::cholesky_solve_LDLT solves linear equation system LDL^T x = b
 *  1) Forward solution (L z = b)
 *  2) Diagonal scaling (D y = z)
 *  3) Backward solution ( L^T x = y)
 * @param L lower triangular matrix
 * @param D diagonal matrix
 * @param b input vector
 * @param x solution is saved in this vector
 * @return
 */
bool Numeric::cholesky_solve_LDLT(const MatDoub L, const MatDoub D, const VecDoub b, VecDoub &x)
{
    // TODO: include check:
    // - if matrix is positive definite: if yAy^T > 0 for any given vector [y]nx1 != 0

    size_t N = L.shape()[0];
    size_t M = L.shape()[1];

    // check for symmetry
    if( N != M)
        return false;

     VecDoub y(boost::extents[N]);
     VecDoub z(boost::extents[N]);

     int i,k;
     double sum;


     // 1) Forward solution (L z = b)

     for(i = 0; i < N; i++)
     {
         sum = 0.0;
         for(k = 0; k < i; k++)
             sum += L[i][k] * z[k];
         z[i] = b[i] - sum;
     }

     // 2) Diagonal scaling (D y = z)
     for(i = 0; i < N; i++)
     {
         y[i] = z[i] / D[i][i];
     }


     // 3) Backward solution ( L^T x = y)
     for(i = N-1; i >= 0; i--)
     {
         sum = 0.0;
         for(k = i+1; k < N; k++)
             sum += L[k][i] * x[k];
         x[i] = y[i] - sum;
     }

//     qDebug() << "LDLT";
//     qDebug() << "x";
//     qDebug() << x[0] << x[1];
//     qDebug() << "y";
//     qDebug() << y[0] << y[1];
//     qDebug() << "z";
//     qDebug() << z[0] << z[1];

     return true;

}


/*****************************
 *
 *      FILTER FUNCTIONS
 *
 **/


/**
 * @brief Numeric::filterMedian
 * @param size
 * @param channel_values
 * @return
 */
Numeric::state_type Numeric::filterMedian(int size, state_type channel_values)
{
    state_type m;
    state_type output(channel_values.size());

    int middle = int( floor(double(size)/2.0));

    for (unsigned int nn=0; nn<channel_values.size(); nn++)
    {
        // clear memory vector
        m.clear();

        // fill memory vector
        for (int ii=0; ii<size; ii++)
        {
            int index = abs( (int)( nn+ii-middle ));
            m.push_back(channel_values[index]);
        }
        // sort memory vector
        std::sort(m.begin(), m.end());

        // determine median value
        output[nn] = m[middle];
    }
    return output;
}


/**
 * @brief Numeric::filterPrewitt
 * @param channel_values
 * @return
 */
Numeric::state_type Numeric::filterPrewitt(state_type channel_values)
{
    state_type input = channel_values;
    state_type output(input.size());

    double sum;

    // initiate filter kernel (Prewitt)
    state_type b = {-1, 0, 1};

    for (int nn=0; nn<input.size(); ++nn)
    {
        sum = 0;

        sum+=b[0]*input[abs( (int) (nn-1))];
        sum+=b[1]*input[abs(nn)];
        sum+=b[2]*input[abs(nn+1)];

        //output[nn] = round(sum*1E2)*1E-2;
        output[nn] = sum;
    }

    return output;
}

/*
 * @brief Numeric::euler
 *        Gives temperature vector for given start parameters and heat transfer model
 *        Using Euler  (1st order)
 * @param ht_object HeatTransferModel
 * @param T_start start temperature
 * @param dp_start start particle diameter
 * @return Signal object containing time and temperature vector
 *
Signal Numeric::euler(double T_start, double dp_start, HeatTransferModel& ht_object)
{
   // if(ht_object == NULL)
   //     throw LIISimException("Numeric::euler: no Heattransfer Model set!",LIISimMessageType::ERR_CALC);


    // initialization
    Signal signal = Signal(signal_length, dt);
    state_type x(2);


    double t;

    // start values
    t = 0.0;            // start time
    x[0] = T_start;     // start temperature (peak)
    x[1] = dp_start;    // start particle diameter

    // set first value
    signal.data[0] = x[0];
    t += dt;



    // calculate temperature trace
    for( int i=1 ; i<signal_length; ++i, t+=dt )
    {


        x[0] = x[0] + ht_object.derivativeT(x[0], x[1]) * dt;   // temperature
        x[1] = x[1] + ht_object.derivativeDp(x[0], x[1]) * dt;  // particle diameter

        signal.data[i] = x[0];

       // if(i==1)
       //     break;
    }


    return signal;
}
*/


// /**
// * @brief Numeric::levmar Levenberg-Marquardt algorithm (inspired by "Numerical Recipes Third Edition")
// * @param fd FitData
// * @param ms ModelingSettings
// * @param fs FitSettings
// * @param ns NumericSettings
// */
//void Numeric::levmar(FitData *fd, ModelingSettings *ms, FitSettings *fs, NumericSettings *ns)
//{
//    Signal signal = fd->dataSignal();
//    HeatTransferModel* ht_object = ms->heatTransferModel();
//    QList<FitParameter> fparams = fs->fitParameters();

//    const int N = 2;     // !!! number of parameters !!!
//    const int IT(ns->iterations());  // number of iterations

//    // levmar variables
//    double delta_y;
//    double chisquare, prev_chisquare;
//    prev_chisquare = 1e100;
//    double alpha[N][N], alpha_inv[N][N];
//    double beta[N];

//    double a[N], a_min[N], a_max[N];

//    int sigSz = signal.data.size();

//    typedef boost::multi_array<double,2> arrayType;
//    arrayType dyda(boost::extents[N][sigSz]);

//    double delta_a[N];
//    for(size_t k=0; k<N; k++)   delta_a[k]  = 0.0;

//    double temp;
//    double h;

//    // signal container
//    Signal signal_model_delta_a[N];

//    // start values for ODE solver (Runge-Kutta)
//    double T_start;

//    // define start conditions
//    double dt             = signal.dt;
//    int signal_length  = signal.data.size();

//    T_start  = signal.getMaxValue();


//    // initialize parameters and boundaries
//    QString msg = QString("Numeric::levmar params: \n\tdt  (of data signal): %0 \n\tT_start (of data signal): %1 \n\titerations: %2 \n\tstepsize: %3")
//            .arg(dt)
//            .arg(T_start)
//            .arg(IT)
//            .arg(ns->integrationStepSize());
//    MSG_ASYNC(msg,DEBUG);

//    for(int i = 0; i < fparams.size(); i++)
//    {
//        a[i] = fparams[i].value();
//        a_min[i] = fparams[i].lowerBound();
//        a_max[i] = fparams[i].upperBound();

//        msg = QString("   fparam%0 (%1): %2 [lower: %3 upper: %4]")
//                    .arg(i)
//                    .arg(fparams[i].name())
//                    .arg(a[i])
//                    .arg(a_min[i])
//                    .arg(a_max[i]);

//        MSG_ASYNC(msg,DEBUG);
//    }

//    //!!!!
//   // a[1] = 1000.0;

//    double lambda = 0.001;
//    Signal signal_model;

//    for(size_t iter=0; iter<IT; iter++)
//    {
//        if(Numeric::canceled)
//        {
//            MSG_ASYNC("Numeric::levmar canceled!",DEBUG);
//            return;
//        }

//        // reset vars and inititalize alpha and beta
//        chisquare = 0.0;

//        for(int i = 0; i<N; i++)
//        {
//            for(int j = 0; j<N; j++)    alpha[i][j] = 0.0; // symmetric dim[NxN]
//            beta[i]     = 0.0;  // vector dim[N]
//        }

//        // set new parameters a[k] for this iteration from calculated delta_a[k]
//        for(size_t k=0; k<N; k++)
//        {
//            temp = a[k] - delta_a[k];

//            if (temp < a_min[k])
//            {
//                a[k] = a_min[k];
//                //delta_a[k] = a[k] - a_min[k];
//            }
//            else if(temp > a_max[k])
//            {
//                a[k] = a_max[k];
//                //delta_a[k] = a[k] - a_max[k];
//            }
//            else
//            {
//                a[k] = temp;
//            }
//        }

//        // calculate temperature decay for given heat transfer model (simulation)

//        // !!! use T_start (first value in datasignal !!!)
//        signal_model = Numeric::runge_kutta( T_start, a[0], *ht_object, dt, signal_length);

//        // !!! use fit parameter
//        // Signal signal_model = Numeric::runge_kutta( a[1], a[0],ht_object);


//        signal_model.start_time = signal.start_time;


//        //  calculate derivatives dyda[k] for parameters a[k]
//        for(size_t k=0; k<N; k++)
//        {
//            // change parameter a[k] infinitesimally
//            h = std::max(1.0, std::abs(a[k])) * Numeric::EPS; // avoid to small change if a[k] >> 1
//            a[k] += h;

//            ht_object->setParameter(a,k);

//            // calculate signal for changed parameter a[k]
//            //signal_model_delta_a[k] = Numeric::runge_kutta( a[1], a[0],ht_object);

//            // calculate signal for changed parameter a[k] !!! ignore temperature fit parameter !!!
//            signal_model_delta_a[k] = Numeric::runge_kutta( T_start, a[0], *ht_object, dt, signal_length);


//            // save dy/da[k]
//            for(size_t i=0 ; i<signal_length; i++)
//            {
//                dyda[k][i] = (signal_model.data[i] - signal_model_delta_a[k].data[i]) / h;
//            }

//            // reset parameter a[k] to previous value
//            a[k] -= h;
//            ht_object->setParameter(a,k);
//        }

//        // calculate alpha and beta from delta_y[i] and dyda[k]
//        for(size_t i=0 ; i<signal_length; i++)
//        {
//            delta_y = signal.data[i] - signal_model.data[i];

//            chisquare += delta_y*delta_y;

//            for(size_t k=0; k<N; k++)
//            {
//                for(size_t j=0; j<N; j++)
//                {
//                    if(j == k)  alpha[k][j] += dyda[k][i] * dyda[j][i] * (1+lambda);
//                    else        alpha[k][j] += dyda[k][i] * dyda[j][i];
//                }
//                beta[k]     += delta_y * dyda[k][i];
//            }
//        }

//        // calculate inverse matrix for linear solution of equation system
//        //if(!Numeric::invertMatrix<N>(alpha, alpha_inv))
//        //    MSG_ASYNC("Numeric::levmar: alpha not invertible",WARNING);

//        // reset delta_a
//        for(size_t k=0; k<N; k++)   delta_a[k]  = 0.0;  // parameter change for this iteration dim[N]

//        // solve equation system: calculate delta_a = alpha^-1 * beta
//        for(size_t k=0; k<N; k++)
//        {
//            for(size_t j=0; j<N; j++)
//                delta_a[k] += alpha_inv[k][j] * beta[j];
//        }


//        // write iteration result
//        FitIterationResult fres = FitIterationResult(6);
//        fres[0] = a[0];
//        fres[1] = a[1];
//        fres[2] = delta_a[0];
//        fres[3] = delta_a[1];
//        fres[4] = chisquare;
//        fres[5] = lambda;
//        fd->addIterationResult(fres);

//        // evaluate this iteration
//        if(chisquare == 0.0)                    prev_chisquare = chisquare; // initialization
//        else if(chisquare < prev_chisquare)     lambda *= init_lambda;              // if better, decrease lambda
//        else                                    lambda *= 1.0/init_lambda;          // if worse, increase lambda

//        // save chisquare for next iteration
//        prev_chisquare = chisquare;

//    }
//}
