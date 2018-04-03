#ifndef DA_TRIGGERDIALOG_H
#define DA_TRIGGERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include "../utils/numberlineedit.h"
#include "./settings/picoscopesettings.h"

class TriggerWidget : public QWidget
{
    Q_OBJECT

public:
    TriggerWidget(QString description, enPS6000TriggerState state, enPS6000ThresholdMode mode,
                  enPS6000ThresholdDirection direction, double upperThreshold, double upperHysteresis,
                  double lowerThreshold, double lowerHysteresis, PSRange range, QWidget *parent = 0);


    QComboBox *comboxCondition;
    QComboBox *comboxType;
    QComboBox *comboxDirection;

    NumberLineEdit *numUpperThreshold;
    NumberLineEdit *numUpperHysteresis;
    NumberLineEdit *numLowerThreshold;
    NumberLineEdit *numLowerHysteresis;

private:
    unsigned int stateToInt(enPS6000TriggerState state);
    unsigned int modeToInt(enPS6000ThresholdMode mode);
    unsigned int directionToInt(enPS6000ThresholdMode mode, enPS6000ThresholdDirection direction);

    QString rangeToString(PSRange range);

    QLabel *labelUpperThreshold;
    QLabel *labelUpperThresholdHysteresis;
    QLabel *labelLowerThreshold;
    QLabel *labelLowerThresholdHysteresis;

private slots:
    void onConditionChanged(int index = 0);
    void onTypeChanged(int index = 0);
    void onDirectionChanged(int index = 0);

};

class DA_TriggerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DA_TriggerDialog(PicoScopeSettings *settings, QWidget *parent = 0);


private:
    PicoScopeSettings *settings;

    TriggerWidget *widgetChannelA;
    TriggerWidget *widgetChannelB;
    TriggerWidget *widgetChannelC;
    TriggerWidget *widgetChannelD;
    TriggerWidget *widgetAUX;

    QPushButton *buttonOK;
    QPushButton *buttonCancel;

    unsigned int stateToInt(enPS6000TriggerState state);
    unsigned int modeToInt(enPS6000ThresholdMode mode);
    unsigned int directionToInt(enPS6000ThresholdMode mode, enPS6000ThresholdDirection direction);

    enPS6000TriggerState intToState(unsigned int state);
    enPS6000ThresholdMode intToMode(unsigned int mode);
    enPS6000ThresholdDirection intToDirection(unsigned int direction);

private slots:
    void onOKClicked(bool checked = false);
    void onCancelClicked(bool checked = false);

};

#endif // DA_TRIGGERDIALOG_H
