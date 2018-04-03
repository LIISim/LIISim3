#include "movingaverage.h"

#include "../../mrun.h"
#include <QDebug>

QString MovingAverage::descriptionFileName = "movingaverage.html"; // TODO
QString MovingAverage::iconFileName = "iconfile"; // TODO
QString MovingAverage::pluginName = "Moving average";

QList<Signal::SType> MovingAverage::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW
        << Signal::ABS
        << Signal::TEMPERATURE;


/**
 * @brief MovingAverage::MovingAverage
 * @param parentChain
 */
MovingAverage::MovingAverage(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Calculates moving average on each datapoint";

    // standard values
    boundary    = "reflect";
    chId        = "all";
    window      = "5";

    /* Boundary behavior*/
    ProcessingPluginInput cbBoundary;
    cbBoundary.type = ProcessingPluginInput::COMBOBOX;
    cbBoundary.value = boundary + ";reflect";
    cbBoundary.labelText = "Boundary";
    cbBoundary.identifier = "cbBoundary";
    cbBoundary.tooltip = "Reflect boundaries: beginning and end of signal are extended by its reflection";

    inputs << cbBoundary; // add value to input list

    /* Channel */
    ProcessingPluginInput cbChannel;
    cbChannel.type = ProcessingPluginInput::COMBOBOX;
    cbChannel.labelText = "Channel";
    cbChannel.identifier = "cbChannel";
    cbChannel.tooltip = "Select channel, which should be processed";

    // fill combobox with values
    int numCh = channelCount();

    QString str,res;

    // add option default + "all"
    res.append(chId);
    res.append(";all");

    for(int i=0; i < numCh; i++)
    {
        res.append(str.sprintf(";%d", i+1));
    }
    cbChannel.value = res;

    inputs << cbChannel;

    /* Window size*/
    ProcessingPluginInput cbWindow;
    cbWindow.type = ProcessingPluginInput::COMBOBOX;
    cbWindow.labelText = "Window size: ";
    cbWindow.identifier = "cbWindow";
    cbWindow.tooltip = "Window size used for averaging";

    // fill combobox with values
    str = "";
    res = "";

    // add default option
    res.append(window);

    // window size needs to be odd and larger than 2 (3,5,7,9,11,13,...)
    for(int i=3; i < 102; i = i+2)
    {
        res.append(str.sprintf(";%d", i));
    }
    cbWindow.value = res;

    inputs << cbWindow;
}


/**
 * @brief MovingAverage::getName implementation of virtual function
 * @return name of plugin
 */
QString MovingAverage::getName()
{
    return pluginName;
}


/**
 * @brief MovingAverage::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void MovingAverage::setFromInputs()
{
    // assign members
    boundary    = inputs.getValue("cbBoundary").toString();
    chId        = inputs.getValue("cbChannel").toString();
    window      = inputs.getValue("cbWindow").toString();
}


/**
 * @brief MovingAverage::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool MovingAverage::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    // process selected channel only
    if(chId == "all" || (chId != "all" && in.channelID == chId.toInt()))
    {
        int noPts = in.data.size();
        int win = window.toInt();
        double sum;
        int j;

        // use arithmetic operation on every datapoint
        for(int i = 0; i < noPts; i++)
        {
            sum = 0.0;
            j = 0;

            // get sum of all data points within window
            for(int k = i-win/2; k <= i+win/2; k++)
            {
                // left boundary
                if(k < 0)
                {
                    j = abs(k); // reflect index at boundaries
                }
                //right boundary
                else if(k >= noPts)
                {
                    j = (noPts-1)-(k-(noPts-1)); // reflect index at boundaries
                }
                else
                {
                    j = k;
                }
                sum += in.data.at(j);
                //qDebug() << mpIdx << " " << in.channelID << " " << i << " " << j << " " << in.data.at(j)<< " " << sum;
            }
            out.data.append(sum/win);
        }
    }
    else
    {
        // don't change other channels
        out = in;
    }

    return true; // we do not make any validation here
}


/**
 * @brief MovingAverage::getParameterPreview
 * @return
 */
QString MovingAverage::getParameterPreview()
{
    QString str = "(Ch %1;window:%2-;%3";
    return str.arg(chId).arg(window).arg(boundary);
}
