#include "signalpair.h"

#include <QDebug>

SignalPair::SignalPair()
{
    raw.type = Signal::RAW;
    absolute.type = Signal::ABS;
}

SignalPair::~SignalPair()
{

}

