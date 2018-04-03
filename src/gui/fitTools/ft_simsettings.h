#ifndef FT_SIMSETTINGS_H
#define FT_SIMSETTINGS_H

#include <QWidget>

#include "../utils/numberlineedit.h"

class FT_SimSettings : public QWidget
{
    Q_OBJECT
public:
    FT_SimSettings(QWidget *parent = 0);
    ~FT_SimSettings();

    double stepSize();
    double length();
    double startTime();

private:
    const double unitConversion_time;

    double def_startTime;
    double def_length;
    double def_stepSize;

    NumberLineEdit *leLength;
    NumberLineEdit *leStepSize;
    NumberLineEdit *leStartTime;

    const QString identifierGroup;
    const QString identifierStepSize;    
    const QString identifierStartTime;
    const QString identifierLength;

private slots:
    void onGUISettingsChanged();

};

#endif // FT_SIMSETTINGS_H
