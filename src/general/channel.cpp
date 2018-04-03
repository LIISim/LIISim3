#include "channel.h"

#include <qmath.h>

Channel::Channel() {}
Channel::Channel(int wavelength) : wavelength(wavelength)
{
    this->bandwidth = 0;
    this->calibration = 1.0;
    this->filter_transmission = 100.0;
    this->offset = 0.0;

    this->pmt_gain = 500.0;
    this->pmt_gain_formula_A = 0.0;
    this->pmt_gain_formula_B = 0.0;
}



int Channel::getHalfBandwidth()
{
     return qFloor(bandwidth / 2.0);
}

int Channel::getMinWavelength()
{
     return wavelength - qFloor(bandwidth / 2.0);
}


int Channel::getMaxWavelength()
{
    return wavelength + qFloor(bandwidth / 2.0);
}
