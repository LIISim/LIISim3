#include "temperature.h"
#include "../../signal/mpoint.h"
#include "../../signal/spectrum.h"
#include "../../database/structure/liisettings.h"
#include "../../general/channel.h"

#include "../../calculations/numeric.h"
#include "../../general/LIISimException.h"
#include "../core.h"

#include <complex>

double Temperature::limitMin = 300.0;
double Temperature::limitMax = 6000.0;

double Temperature::limitCMin = 1E-30;
double Temperature::limitCMax = 1E30;

// NEVER CHANGE THE ORDER OF THIS LIST
QStringList Temperature::sourceEmOptions = QStringList() << "values"
                                                         << "function"
                                                         << "from Drude theory";


/**
 * @brief Temperature::checkEmSource checks availability of E(m) for selected source
 * @param material
 * @param sourceEm must be listed in Temperature::sourceEmOptions
 * @param channels list of channels to be checked
 * @return
 */
bool Temperature::checkEmSource(Material& material, QString sourceEm, QMap<int, Channel> channels)
{
    // check availability of selected Em source selection
    if(!sourceEmOptions.contains(sourceEm))
    {
        MSG_ERR(QString("Temperature::checkEmSource(): E(m) source selection \"%0\" is not valid: must be one of: \"%1\"")
                .arg(sourceEm)
                .arg(sourceEmOptions.join("\", \"")));

        return false;
    }

    QString msg;
    QString msg_prepend = QString("TemperatureCalculator: \"E(m) source\": ");
    QString msg_append = QString("Please check your Spectroscopic Material database file.");

    // values
    if(sourceEm == Temperature::sourceEmOptions.at(0))
    {
        bool error_occured = false;

        QMapIterator<int, Channel> ch(channels);

        // check all channels
        while (ch.hasNext()) {
            ch.next();

            int lambda = ch.value().wavelength;

            if(!material.Em.exists(lambda))
            {
                msg = QString("%0E(m) values (Material.Em) not available for Channel %1 - lambda=%2 (%3)\n")
                                .arg(msg_prepend)
                                .arg(ch.key())
                                .arg(lambda)
                                .arg(material.name);

                msg.append(msg_append);

                MSG_ONCE("SpectroscopicMaterial", 20 + ch.key(), msg, ERR);
                error_occured = true;
            }
        }

        // make sure to check all Em values before exist function
        if(error_occured)
            return false;
    }
    // function
    else if(sourceEm == Temperature::sourceEmOptions.at(1))
    {
        if(!material.Em_func.available)
        {
            msg = msg_prepend;
            msg.append(QString("E(m) function (Material.Em_func) not available for %0\n%1")
                            .arg(material.name)
                            .arg(msg_append));

            MSG_ONCE("SpectroscopicMaterial", 3, msg, ERR);
            return false;
        }

    }
    // from Drude theory
    else if(sourceEm == Temperature::sourceEmOptions.at(2))
    {
        if(!material.omega_p.available && !material.omega_p.available)
        {
            msg = msg_prepend;
            msg.append(QString("E(m) can not be calculated with Drude theory for %0 "
                          "(Material.omega_p and/or Material.tau not available)\n%1")
                            .arg(material.name)
                            .arg(msg_append));

            MSG_ONCE("SpectroscopicMaterial", 4, msg, ERR);
            return false;
        }
    }

    // E(m) available (no errors found)
    return true;
}


/**
 * @brief Temperature::getEmBySource return Em value for selected source
 * @param lambda_m [meter]
 * @param material
 * @param sourceEm
 * @return
 */
double Temperature::getEmBySource(double lambda_m, Material& material, QString sourceEm)
{
    // see also property.cpp: Property::operator()(double T, double wavelength)
    // temperature-dependence of Em could be implemented in future f.e. to account
    // for phase changes

    double Em;

    // Em(m) is currently temperature independent
    double T_dummy = 0.0;

    // E(m) values
    if(sourceEm == sourceEmOptions.at(0))
    {
        // convert to nanometer
        int lambda = int(lambda_m * 1E9);
        Em = material.Em(T_dummy, lambda);
    }
    // E(m) function
    else if(sourceEm == sourceEmOptions.at(1))
    {
        Em = material.Em_func(T_dummy, lambda_m);
    }
    // E(m) from Drude theory
    else if(sourceEm == sourceEmOptions.at(2))
    {
        // convert to nanometer
        int lambda = int(lambda_m * 1E9);
        // TODO: move this part to main loop, currently Em is calculated for every data point
        Em = calcDrudeEm(lambda, material);
    }

    //QString str = QString("Temperature: getEmBySource() - E(m): %0 (lambda_m = %1)").arg(Em).arg(lambda_m);
    //MSG_ONCE("DebugTemperature", int(lambda_m * 1E9), str, DEBUG);

    return Em;
}


/**
 * @brief Temperature::calcDrudeEm
 * @param lambda [nm]
 * @param material
 * @return
 */
double Temperature::calcDrudeEm(int lambda, Material material)
{
    // check if spectroscopic material contains drude parameters
   if(!material.omega_p.available || !material.tau.available)
   {
       QString msg = QString("Drude parameters not defined in selected Spectroscopic Material (%0)")
               .arg(material.name);

       // Message reset after rescan database or change of spectroscopic model
       MSG_ONCE("SpectroscopicMaterial", 1, msg, WARNING);
       return 0.0;
   }

   // Uses Material: omega_p, tau
   double omega_p = material.omega_p();
   double tau     = material.tau();

   // convert to seconds
   double lambda_m = lambda * 1E-9;

   // frequency of light
   double nu = Constants::c_0 / lambda_m;

   // angular frequency of the electromagnetic wave
   double omega = 2.0 * Constants::pi * nu;

   // dielectric function (wavelength-dependent)
   double epsilon1 = 1.0 - omega_p*omega_p * tau*tau
                           / (omega*omega * tau*tau + 1.0);

   double epsilon2 = omega_p*omega_p * tau
                           / (omega * (omega*omega * tau*tau + 1.0));

   // electrical permittivity of the bulk nanoparticle material
   // ε = εI + iεII
   std::complex<double> eps(epsilon1, epsilon2);

   // perform complex number arithmetics

   std::complex<double> complex_result = (eps - 1.0) / (eps + 2.0);

   // imaginary part of solution
   double Em = complex_result.imag();

   return Em;
}


/**
 * @brief Temperature::calcTemperatureFromTwoColor calculate temperature using two-color LII
 * @param liiSettings LIISettings
 * @param material material
 * @param sourceEm string defines which E(m) values are used for calculation (values, function, from Drude theory)
 * @param mpoint measurement point
 * @param chId1 channel id of first signal
 * @param chId2 channel id of secont signal
 * @param inputSigType type of signals which should be used for calculation (raw or absolute)
 * @return resulting temperature signal
 */
Signal Temperature::calcTemperatureFromTwoColor(LIISettings& liiSettings,
                                                Material& material,
                                                QString sourceEm,
                                                MPoint * mpoint,
                                                int chId1, int chId2,
                                                Signal::SType inputSigType)
{
    // empty signal is returned if error occurs
    Signal emptySignal = Signal();

    // get channels
    Channel channel1 = liiSettings.channels.at(chId1-1);
    Channel channel2 = liiSettings.channels.at(chId2-1);

    // channels to be checked for E(m)
    QMap<int, Channel> channels;
    channels.insert(chId1, channel1);
    channels.insert(chId2, channel2);

    // check if E(m) is available
    if(!checkEmSource(material, sourceEm, channels))
            return emptySignal;

    int lambda1 = channel1.wavelength;
    int lambda2 = channel2.wavelength;

    // get signals
    Signal signal_1 = mpoint->getSignal(chId1, inputSigType);
    Signal signal_2 = mpoint->getSignal(chId2, inputSigType);

    // resample signals to same start time/ number of datapoints if necessary
    if( signal_1.start_time != signal_2.start_time ||
        signal_1.dt != signal_2.dt ||
        signal_1.data.size() != signal_2.data.size() )
    {
        double mint = mpoint->getMinSignalTime(inputSigType);
        double maxt = mpoint->getMaxSignalTime(inputSigType);
        int data_sz = int( (maxt - mint) / signal_1.dt + 1 );

        signal_1.resample(mint, signal_1.dt, data_sz);
        signal_2.resample(mint, signal_1.dt, data_sz);
    }

    Signal signal;
    signal.start_time = signal_1.start_time;
    signal.dt = signal_1.dt;
    signal.type = Signal::TEMPERATURE;
    signal.channelID = 1;

    double v1;
    double v2;

    for(int i=0; i< signal_1.data.size(); i++)
    {
        v1 = signal_1.data[i];
        v2 = signal_2.data[i];

        // use two-color pyrometry
        double T_value = Temperature::calcTwoColor(v1, v2,
                                                   double(lambda1), double(lambda2),
                                                   material,
                                                   sourceEm);

        // if v1 or v2 smaller than zero
        if(T_value == 0.0)
        {
            signal.data.append(0.0);
            continue;
        }

        // thresholding by numerical limits
        if(T_value > limitMax)
            T_value = limitMax;

        if(T_value < limitMin)
            T_value = limitMin;

        signal.data.append(T_value);
    }
    return signal;
}


Signal Temperature::calcTemperatureFromSpectrum(LIISettings& liiSettings,
                                                Material& material,                                                
                                                QString sourceEm,
                                                MPoint * mpoint,
                                                Signal::SType inputSigType,
                                                QList<bool> *activeChannels,
                                                bool bpIntegration,
                                                bool weighting,
                                                bool autoStartC,
                                                int iterations = 60,
                                                double startTemperature = 2500.0,
                                                double startC = 1.0
                                                )
{
    // empty signal is returned if error occurs
    Signal emptySignal = Signal();

    // channels to be checked for E(m)
    QMap<int, Channel> check_channels;

    // first init and check channels
    QVector<double> wavelengths;
    QVector<double> intensities, stdev;
    QVector<int> bandwidths; // half bandpass bandwidths
    QList<Channel> chlist = liiSettings.channels;

    // get wavelength for each channel
    for(int i = 0; i < chlist.size(); i++)
    {
        if(activeChannels->at(i) == true)
        {            
            check_channels.insert(i+1, chlist.at(i));
            wavelengths.append(double(chlist.at(i).wavelength));
            bandwidths.append(chlist[i].getHalfBandwidth());
        }
    }

    // check if E(m) is available
    if(!checkEmSource(material, sourceEm, check_channels))
            return emptySignal;

    // update fit start parameters for each y from previous y (fit converges faster)
    bool updateFitStart = false;

    // FitParameters
    QList<FitParameter> fparams;

    // FitData
    FitData* fitData = new FitData();

    // ModelingSettings
    ModelingSettings* modSettings = new ModelingSettings;
    modSettings->setHeatTransferModel(0); // default value to avoid crash
    modSettings->setMaterialSpec(material.filename);

    // FitSettings
    FitSettings* fitSettings = new FitSettings;
    fitSettings->setBandpassIntegrationActive(bpIntegration);
    fitSettings->setWeightingActive(weighting);
    fitSettings->setSourceEm(sourceEm);

    // NumericSettings
    NumericSettings* numSettings = new NumericSettings;
    numSettings->setIterations(iterations);
    numSettings->lambda_init        = 0.1;
    numSettings->lambda_decrease    = 0.5;
    numSettings->lambda_increase    = 2.0;
    numSettings->lambda_scaling     = 100;


    // Fit results
    QList<FitIterationResult> fitResList;


    // get signal from first channel (assuming all channels having the same length)
    Signal init_signal = mpoint->getSignal(1,inputSigType);

    // create new signal (SType:Temperature) from best fit for each data point
    Signal t_signal;
    t_signal.start_time = init_signal.start_time;
    t_signal.dt = init_signal.dt;
    t_signal.type = Signal::TEMPERATURE;
    t_signal.channelID = 1;


    //------------------------------------
    // Determine initial scaling factor C
    //
    // This methods helps to overcome numerical scaling problems:
    //  1) Determine signal peak from channel 1
    //  2) Calculate Planck intensity at StartTemperature = 3000 with C = 1
    //  3) Calculate ratio of signal intensity and Planck intensity for each channel
    //  4) Initial scaling factor is calculated from average of all ratios
    //------------------------------------
    if(autoStartC)
    {        
        // peak of channel 1: index of data vector
        int peak_x = init_signal.getMaxIndex();

        // init
        double lambda_m, planckIntensity;
        double ratio = 0.0;
        intensities.clear();

        // get intensity for each channel and calculate ratio
        for(int i = 0; i < chlist.size(); i++)
        {
            if(activeChannels->at(i) == true)
            {
                // Planck intensity at startTemperature and scaling factor = 1
                lambda_m = double(chlist.at(i).wavelength) * 1E-9;
                planckIntensity = calcPlanckIntensity(lambda_m, 3000.0, 1.0, material, sourceEm);

                // intensity of signal
                Signal signal = mpoint->getSignal(i+1, inputSigType);
                intensities.append(signal.data.at(peak_x));

                // sum of all ratios from intensity and planck intensity
                ratio = ratio + (intensities.last() / planckIntensity);

//                qDebug() << "Temperature::calcTemperatureFromSpectrum(): lambda: " << chlist.at(i).wavelength
//                         << "Planck" << planckIntensity
//                         << "intensities: " << intensities.last()
//                         << "ratio: " << (intensities.last() / planckIntensity);
            }
        }

        // determine inital scaling factor from average ratio of all channels
        startC = ratio / wavelengths.size();

        // debug output for start fit parameters
        //qDebug() << "Temperature::calcTemperatureFromSpectrum(): Initial peak fit: "<< startTemperature << "K - C: " << startC;
    }


    //----------------------------
    // process all data points (y)
    //----------------------------
    for(int y = 0; y < mpoint->getSignal(1,inputSigType).size(); y++)
    {
        //qDebug() << "Data point: " << y;

        fparams.clear();
        fparams << FitParameter(0, "Start temperature", "K", startTemperature, limitMin, limitMax, 500.0);
        fparams << FitParameter(1, "C", "-", startC, limitCMin, limitCMax, 10000.0);

        fitSettings->setFitParameters(fparams);

        //fparams[1].setEnabled(true);

        // clear data
        intensities.clear();

        double stdev_data = 1.0;

        // get intensity/wavelength for each channel
        for(int i = 0; i < chlist.size(); i++)
        {
            if(activeChannels->at(i) == true)
            {
                Signal signal = mpoint->getSignal(i+1, inputSigType);

                intensities.append(signal.data.at(y));

                // if no stdev available set values to 1.0
                if(signal.stdev.size() > 0)
                    stdev_data = signal.stdev.at(y);

                stdev.append(stdev_data);
            }
        }

        /*
         * process each data point from all channels:
         * least square fit: wavelength(x), intensity (y), planck's law (f(x))
        */

        // clear fitdata results
        fitData->initData(wavelengths, intensities, stdev);
        fitData->initBandwidth(bandwidths);

        fitData->clearResults();

        Numeric::levmar(FitRun::TEMP,
                        fitData,
                        modSettings,
                        fitSettings,
                        numSettings);


        fitResList = fitData->iterationResultList();

        // Results:
        // res[2]: Temperature      // res[3]: Delta Temperature
        // res[4]: Scaling factor   // res[5]: Delta scaling factor

        t_signal.fitData.append(fitResList);

        for(int k = 0; k < activeChannels->size(); k++)
        {
            t_signal.fitActiveChannels.append(activeChannels->at(k));
        }

        // get final temperature        
        t_signal.data.append(fitData->iterationResultLast().at(2));

        // update fit start parameters to new values
        if(updateFitStart)
        {
            startTemperature   = fitData->iterationResultLast().at(2);
            startC             = fitData->iterationResultLast().at(4);
        }
    }

    return t_signal;
}


/**
 * @brief Temperature::calcTemperatureFromSpectrum_Test
 * @param liiSettings
 * @param material
 * @param mpoint data should not be relative channel calibrated
 * @param inputSigType
 * @param activeChannels
 * @param iterations
 * @param startTemperature
 * @param startConstant
 * @return
 */
Signal Temperature::calcTemperatureFromSpectrumTest(LIISettings& liiSettings,
                                                Material& material,
                                                QString sourceEm,
                                                MPoint * mpoint,
                                                Signal::SType inputSigType,
                                                QList<bool> *activeChannels,
                                                int iterations = 60,
                                                double startTemperature = 2500.0,
                                                double startC = 1.0
                                                )
{
    // empty signal is returned if error occurs
    Signal emptySignal = Signal();

    // channels to be checked for E(m)
    QMap<int, Channel> check_channels;

    bool weighting = false;


    // update fit start parameters for each y from previous y (fit converges faster)
    bool updateFitStart = false;

    // numeric parameters
    QList<FitParameter> fparams;

    // FitData
    FitData* fitData = new FitData();
    fitData->mrun_LIISettings = liiSettings;

    //ModelingSettings
    ModelingSettings* modSettings = new ModelingSettings;
    modSettings->setHeatTransferModel(0); // default value to avoid crash
    modSettings->setMaterialSpec(material.filename);

    // FitSettings
    FitSettings* fitSettings = new FitSettings;   
    fitSettings->setBandpassIntegrationActive(false);
    fitSettings->setWeightingActive(false);
    fitSettings->setSourceEm(sourceEm);

    // NumericSettings
    NumericSettings* numSettings = new NumericSettings;
    numSettings->setIterations(iterations);
    numSettings->lambda_init        = 0.1;
    numSettings->lambda_decrease    = 0.2;
    numSettings->lambda_increase    = 10.0;
    numSettings->lambda_scaling     = 10.0;


    // Fit results
    QList<FitIterationResult> fitResList;


    // init vars
    QVector<double> wavelengths;
    QVector<double> intensities, stdev;
    QVector<int> bandwidths; // half bandpass bandwidths
    QList<Channel> chlist = liiSettings.channels;


    // get signal from first channel (assuming all channels having the same length)
    Signal init_signal = mpoint->getSignal(1,inputSigType);

    // create new signal (SType:Temperature)from best fit for each data point
    Signal t_signal;
    t_signal.start_time = init_signal.start_time;
    t_signal.dt = init_signal.dt;
    t_signal.type = Signal::TEMPERATURE;
    t_signal.channelID = 1;


    if(activeChannels->size() < 3)
    {
        qDebug() << "Temperature: at least three independent channels are necessary for this method";
        return t_signal;
    }


    // get wavelength for each channel
    int k = 0;
    for(int i = 0; i < chlist.size(); i++)
    {
        if(activeChannels->at(i) == true)
        {
            check_channels.insert(i+1, chlist.at(i));
            wavelengths.append(double(chlist.at(i).wavelength));
            bandwidths.append(chlist[i].getHalfBandwidth());

            fparams << FitParameter(k, QString("Calibration CH %0").arg(i+1),
                                    "-",
                                    1.0, // startValue = 1.0
                                    0.001, 1000.0, // min, max
                                    5.0); // deltaMax
            k++;
        }
    }


    // check if E(m) is available
    if(!checkEmSource(material, sourceEm, check_channels))
            return emptySignal;

    // decrease degrees of freedom by disabling last channel
    //fparams.last().setEnabled(false);

    // assign fit parameters
    fitSettings->setFitParameters(fparams);


    //----------------------------
    // process all data points (y) and save them in list
    //----------------------------
    for(int y = 0; y < mpoint->getSignal(1,inputSigType).size(); y++)
    {
        //qDebug() << "Data point: " << y;

        // clear data
        intensities.clear();

        double stdev_data = 1.0;

        // get intensity/wavelength for each channel
        for(int i = 0; i < chlist.size(); i++)
        {
            if(activeChannels->at(i) == true)
            {
                Signal signal = mpoint->getSignal(i+1, inputSigType);

                intensities.append(signal.data.at(y));

                if(signal.stdev.size() > 0)
                    stdev_data = signal.stdev.at(y);

                stdev.append(stdev_data);
            }
        }

        fitData->ydatalist.append(intensities);
    }

        /*
         * process each data point from all channels:
         * least square fit: wavelength(x), intensity (y), planck's law (f(x))
        */

        // clear fitdata results
        fitData->initData(wavelengths, intensities, stdev);
        fitData->clearResults();

        Numeric::levmar_test(FitRun::TEMP_CAL,
                        fitData,
                        modSettings,
                        fitSettings,
                        numSettings);


        fitResList = fitData->iterationResultList();

        // Results:
        // res[2]: Temperature      // res[3]: Delta Temperature
        // res[4]: Scaling factor   // res[5]: Delta scaling factor

        t_signal.fitData.append(fitResList);

        for(int k = 0; k < activeChannels->size(); k++)
        {
            t_signal.fitActiveChannels.append(activeChannels->at(k));
        }


        qDebug() << "Iterations: " << fitResList.size();
//        for(int k = 0; k < fitResList.size(); k++)
//        {
//            if((k % 4) == 0) continue;
//            qDebug() << "Temperature: calTest: " << k << ": " << "\t"
//                                          << fitResList.at(k).at(2) << "\t"
//                                          << fitResList.at(k).at(4) << "\t"
//                                          << fitResList.at(k).at(6) << "\t"
//                                          //<< fitResList.at(k).at(8) << "\t"
//                                          << fitResList.at(k).at(0) << "\t"
//                                          << fitResList.at(k).at(1) << "\t"
//                                          << fitResList.at(k).at(3) << "\t"
//                                          << fitResList.at(k).at(5);
//        }

    qDebug() << "Temperature: calTest: Last:" << "\t"
                                      << fitResList.last().at(2) << "\t"
                                      << fitResList.last().at(4) << "\t"
                                      << fitResList.last().at(6) << "\t"
                                      << fitResList.last().at(8) << "\t"
                                      << fitResList.last().at(0) << "\t"
                                      << fitResList.last().at(1);


    MPoint* new_mp = new MPoint(chlist.size());

    double cal;

    for(int i = 0; i < chlist.size(); i++)
    {
        if(activeChannels->at(i) == true)
        {
            Signal signal = mpoint->getSignal(i+1, inputSigType);

            cal = fitResList.last().at(2+ 2*i);

            for(int k = 0; k < signal.data.size(); k++)
                       signal.data[k] = signal.data[k] * cal;

            new_mp->setSignal(signal, i+1, inputSigType);
        }
    }


    t_signal = Temperature::calcTemperatureFromSpectrum(liiSettings,
                                             material,
                                             sourceEm,
                                             new_mp,
                                             inputSigType,
                                             activeChannels,
                                             false,
                                             false,
                                             iterations,
                                             startTemperature,
                                             startC);

    return t_signal;
}


/**
  * @brief Temperature::calcTwoColor two-color pyrometry
  * @param v1 signal1 [-]
  * @param v2 signal2 [-]
  * @param lambda1 wavelength 1 [nm]
  * @param lambda2 [nm]
  * @param material material properties
  * @return
  */
double Temperature::calcTwoColor(double v1, double v2,
                                  double lambda1, double lambda2,
                                  Material& material,
                                  QString sourceEm)
{
     // ratio v1/v2 should not be <= 0 (because of log)
     // v2 should not be == 0 (because of log and division)
     if(v1 <= 0.0 || v2 <= 0.0)
     {
         //ignore this datapoint and set temperature to zero
         return 0.0;
     }

    // convert to meter
    double lambda1_m = lambda1 * 1E-9;
    double lambda2_m = lambda2 * 1E-9;

    // get E(m) dependent on selected source
    double Em1 = getEmBySource(lambda1_m, material, sourceEm);
    double Em2 = getEmBySource(lambda2_m, material, sourceEm);

    return Constants::c_2
               * (1/lambda2_m - 1/lambda1_m)
               / log ( v1 / v2 * Em2 / Em1
                       * pow((lambda1/lambda2), 6));
}


/**
  * @brief Temperature::calcPlanckIntensity
  * @param lambda_m [meter]
  * @param T [K]
  * @param C [-]
  * @param material
  * @return
  */
 double Temperature::calcPlanckIntensity(double lambda_m, double T, double C,
                                         Material& material,
                                         QString sourceEm)
 {          

     // CAUTION: unit of lambda could be meter or nanometer
     // Convention: lambda [nm], lambda_m [m]

    double Em = getEmBySource(lambda_m, material, sourceEm);

    //return C * Em / (lambda_m * 1E18) // scaling value
    return C * Em / lambda_m
             * Constants::c_1
             / pow(lambda_m,5)
             / (exp(Constants::c_2 / lambda_m / T) - 1);
 }


 /**
  * @brief Temperature::calcPlanckIntensityBP calculates integrated planck intensity over bandpass width
  * @param lambda - integer [nm]
  * @param half_bandwidth - integer [nm]
  * @param T
  * @param C
  * @param material
  * @return
  */
 double Temperature::calcPlanckIntensityBandpass(int lambda, int bandwidth, double T, double C,
                                                 Material& material,
                                                 QString sourceEm)
 {
     // create empty spectrum with size of bandpass width
     Spectrum spectrum(lambda, bandwidth);

     double lambda_m;
     double intensity;

     // make sure integration is divided by actual number of data points,
     // could differ to bandwidth due to rounding (see Spectrum class)
     int count = 0;

     // fill spectrum with intensites at wavlenths(i)
     for(int i = 0; i < spectrum.xData.size(); i++)
     {
         lambda_m   = spectrum.xData.at(i) * 1E-9;
         intensity  = calcPlanckIntensity(lambda_m, T, C, material, sourceEm);

         spectrum.yData[i] = intensity;
         count++;
     }

     // integrate over spectrum and normalize
     return spectrum.integrate(spectrum.xData.first(),
                               spectrum.xData.last()) / (count-1);
 }







