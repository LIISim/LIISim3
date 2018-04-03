#ifndef CUSTOMPLOTOPTIONSWIDGET_H
#define CUSTOMPLOTOPTIONSWIDGET_H

#include <QTreeWidget>

class SignalPlotWidgetQwt;

class CustomPlotOptionsWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CustomPlotOptionsWidget(SignalPlotWidgetQwt* plot, QWidget *parent = 0);
    ~CustomPlotOptionsWidget();


private:
    SignalPlotWidgetQwt* plot;
signals:

public slots:
};

#endif // CUSTOMPLOTOPTIONSWIDGET_H
