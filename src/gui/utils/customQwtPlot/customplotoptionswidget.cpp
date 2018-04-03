#include "customplotoptionswidget.h"

#include "../signalplotwidgetqwt.h"

CustomPlotOptionsWidget::CustomPlotOptionsWidget(SignalPlotWidgetQwt* plot, QWidget *parent)
    : QTreeWidget(parent),
      plot(plot)
{

}

CustomPlotOptionsWidget::~CustomPlotOptionsWidget()
{

}

