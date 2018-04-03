#ifndef CHANNEL_H
#define CHANNEL_H

class Channel
{
public:
    Channel();
    Channel(int wavelength);

    int wavelength;             // center wavelength [nm]
    int bandwidth;              // width of bandpass filter [nm]
    double calibration;         // calibration factor for absolute calibration [-]
    double filter_transmission; // currently not used [%]
    double offset;              // [-]

    // y = exp(A*ln(x/pmt_gain))
    double pmt_gain;            // [V] - reference voltage value for gain correction formula

    // relative gain formula
    // y = 10^(A*log_10(x)+B)
    double pmt_gain_formula_A;  // [-]
    double pmt_gain_formula_B;  // [-]

    int getHalfBandwidth();     // half bandpass wavelength
    int getMinWavelength();     // min bandpass wavelength
    int getMaxWavelength();     // max bandpass wavelength


};

#endif // CHANNEL_H
