#include "streampoint.h"
#include "core.h"

#include "gui/utils/signalplotwidgetqwt.h"

#include <QMap>

StreamPoint::StreamPoint()
{
    averageBufferFilling = 0;

    overflowA = false;
    overflowB = false;
    overflowC = false;
    overflowD = false;
}


void StreamPoint::draw(SignalPlotWidgetQwt *plot)
{
    QList<unsigned int> signalPlotOrder;

    signalPlotOrder << 1 << 2 << 3 << 4;

    draw(plot, signalPlotOrder);

    /*//Add channel A to plot
    if(Core::instance()->guiSettings->value("picoscopewidget", "channelAvisible", true).toBool())
    {
        if(single.contains(1) && average.contains(1))
        {
            plot->setCurrentColor(QColor("blue").lighter());
            plot->addSignal(single.value(1), "Channel A", false);
            plot->setCurrentColor(QColor("blue"));
            plot->addSignal(average.value(1), "Average Channel A", false);
        }
        else
        {
            if(single.contains(1))
            {
                plot->setCurrentColor(QColor("blue").lighter());
                plot->addSignal(single.value(1), "Channel A", false);
            }
            if(average.contains(1))
            {
                plot->setCurrentColor(QColor("blue").lighter());
                plot->addSignal(average.value(1), "Average Channel A", false);
            }
        }
    }

    //Add channel B to plot
    if(Core::instance()->guiSettings->value("picoscopewidget", "channelBvisible", true).toBool())
    {
        if(single.contains(2) && average.contains(2))
        {
            plot->setCurrentColor(QColor("red").lighter());
            plot->addSignal(single.value(2), "Channel B", false);
            plot->setCurrentColor(QColor("red"));
            plot->addSignal(average.value(2), "Average Channel B", false);
        }
        else
        {
            if(single.contains(2))
            {
                plot->setCurrentColor(QColor("red").lighter());
                plot->addSignal(single.value(2), "Channel B", false);
            }
            if(average.contains(2))
            {
                plot->setCurrentColor(QColor("red").lighter());
                plot->addSignal(average.value(2), "Average Channel B", false);
            }
        }
    }

    //Add channel C to plot
    if(Core::instance()->guiSettings->value("picoscopewidget", "channelCvisible", true).toBool())
    {
        if(single.contains(3) && average.contains(3))
        {
            plot->setCurrentColor(QColor("green").lighter());
            plot->addSignal(single.value(3), "Channel C", false);
            plot->setCurrentColor(QColor("green"));
            plot->addSignal(average.value(3), "Average Channel C", false);
        }
        else
        {
            if(single.contains(3))
            {
                plot->setCurrentColor(QColor("green").lighter());
                plot->addSignal(single.value(3), "Channel C", false);
            }
            if(average.contains(3))
            {
                plot->setCurrentColor(QColor("green").lighter());
                plot->addSignal(average.value(3), "Average Channel C", false);
            }
        }
    }

    //Add channel D to plot
    if(Core::instance()->guiSettings->value("picoscopewidget", "channelDvisible", true).toBool())
    {
        if(single.contains(4) && average.contains(4))
        {
            plot->setCurrentColor(QColor("yellow"));
            plot->addSignal(single.value(4), "Channel D", false);
            plot->setCurrentColor(QColor("yellow").dark(150));
            plot->addSignal(average.value(4), "Average Channel D", false);
        }
        else
        {
            if(single.contains(4))
            {
                plot->setCurrentColor(QColor("yellow"));
                plot->addSignal(single.value(4), "Channel D", false);
            }
            if(average.contains(4))
            {
                plot->setCurrentColor(QColor("yellow"));
                plot->addSignal(average.value(4), "Average Channel D", false);
            }
        }
    }*/
}


void StreamPoint::draw(SignalPlotWidgetQwt *plot, QList<unsigned int> signalPlotOrder)
{
    for(int i = 0; i < signalPlotOrder.size(); i++)
    {
        switch(signalPlotOrder[i])
        {
        case 1:
        {
            //Add channel A to plot
            if(Core::instance()->guiSettings->value("picoscopewidget", "channelAvisible", true).toBool())
            {
                if(single.contains(1) && average.contains(1))
                {
                    plot->setCurrentColor(QColor("blue").lighter());
                    plot->addSignal(single.value(1), "Channel A", false);
                    plot->setCurrentColor(QColor("blue"));
                    plot->addSignal(average.value(1), "Average Channel A", false);
                }
                else
                {
                    if(single.contains(1))
                    {
                        plot->setCurrentColor(QColor("blue").lighter());
                        plot->addSignal(single.value(1), "Channel A", false);
                    }
                    if(average.contains(1))
                    {
                        plot->setCurrentColor(QColor("blue").lighter());
                        plot->addSignal(average.value(1), "Average Channel A", false);
                    }
                }
            }
        } break;
        case 2:
        {
            //Add channel B to plot
            if(Core::instance()->guiSettings->value("picoscopewidget", "channelBvisible", true).toBool())
            {
                if(single.contains(2) && average.contains(2))
                {
                    plot->setCurrentColor(QColor("red").lighter());
                    plot->addSignal(single.value(2), "Channel B", false);
                    plot->setCurrentColor(QColor("red"));
                    plot->addSignal(average.value(2), "Average Channel B", false);
                }
                else
                {
                    if(single.contains(2))
                    {
                        plot->setCurrentColor(QColor("red").lighter());
                        plot->addSignal(single.value(2), "Channel B", false);
                    }
                    if(average.contains(2))
                    {
                        plot->setCurrentColor(QColor("red").lighter());
                        plot->addSignal(average.value(2), "Average Channel B", false);
                    }
                }
            }
        } break;
        case 3:
        {
            //Add channel C to plot
            if(Core::instance()->guiSettings->value("picoscopewidget", "channelCvisible", true).toBool())
            {
                if(single.contains(3) && average.contains(3))
                {
                    plot->setCurrentColor(QColor("green").lighter());
                    plot->addSignal(single.value(3), "Channel C", false);
                    plot->setCurrentColor(QColor("green"));
                    plot->addSignal(average.value(3), "Average Channel C", false);
                }
                else
                {
                    if(single.contains(3))
                    {
                        plot->setCurrentColor(QColor("green").lighter());
                        plot->addSignal(single.value(3), "Channel C", false);
                    }
                    if(average.contains(3))
                    {
                        plot->setCurrentColor(QColor("green").lighter());
                        plot->addSignal(average.value(3), "Average Channel C", false);
                    }
                }
            }
        } break;
        case 4:
        {
            //Add channel D to plot
            if(Core::instance()->guiSettings->value("picoscopewidget", "channelDvisible", true).toBool())
            {
                if(single.contains(4) && average.contains(4))
                {
                    plot->setCurrentColor(QColor("yellow"));
                    plot->addSignal(single.value(4), "Channel D", false);
                    plot->setCurrentColor(QColor("yellow").dark(150));
                    plot->addSignal(average.value(4), "Average Channel D", false);
                }
                else
                {
                    if(single.contains(4))
                    {
                        plot->setCurrentColor(QColor("yellow"));
                        plot->addSignal(single.value(4), "Channel D", false);
                    }
                    if(average.contains(4))
                    {
                        plot->setCurrentColor(QColor("yellow"));
                        plot->addSignal(average.value(4), "Average Channel D", false);
                    }
                }
            }
        } break;
        }
    }
}
