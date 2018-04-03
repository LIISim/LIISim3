#ifndef PICOSCOPESETTINGSWIDGET_H
#define PICOSCOPESETTINGSWIDGET_H

#include <QTableWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QList>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QToolButton>

#include "../../settings/picoscopesettings.h"
#include "../../io/picoscope.h"
#include "../utils/numberlineedit.h"
#include "channelvisualcontrolwidget.h"



/**
 * @brief The PicoScopeSettingsWidget class represents a table-style editor widget
 * for the PicoScopeSettings class. Its part of the DataAcquisition GUI.
 */
class PicoScopeSettingsWidget : public QTableWidget
{
    Q_OBJECT

public:

    explicit PicoScopeSettingsWidget(PicoScopeSettings *psSettings,PicoScope* device, QWidget *parent = 0);
    ~PicoScopeSettingsWidget();

    float gainA();
    float gainB();
    float gainC();
    float gainD();

    QList<double> getGainList();
    void updateGainList(QList<double> list);

    QList<double> getGainReferenceList();

    int getBlockSequenceBlockNumber();
    float getBlockSequenceVoltageDecrement();
    float getBlockSequenceDelaySec();

    bool focusNextPrevChild(bool next);

    bool averageCaptures();

    int getNoCaptures();

    QLabel *labelBlockSequenceTimeLeft;
    QLabel *labelBlockSequenceSamplesLeft;

private:

    PicoScopeSettings* psSettings;
    PicoScope *picoscope;

    ChannelVisualControlWidget *wvcChannelA;
    ChannelVisualControlWidget *wvcChannelB;
    ChannelVisualControlWidget *wvcChannelC;
    ChannelVisualControlWidget *wvcChannelD;

    QCheckBox *channelA;
    QCheckBox *channelB;
    QCheckBox *channelC;
    QCheckBox *channelD;

    QComboBox *cbRangeChannelA;
    QComboBox *cbRangeChannelB;
    QComboBox *cbRangeChannelC;
    QComboBox *cbRangeChannelD;

    QCheckBox *linkChannelSettings;

    QDoubleSpinBox *spinboxOffsetChannelA;
    QDoubleSpinBox *spinboxOffsetChannelB;
    QDoubleSpinBox *spinboxOffsetChannelC;
    QDoubleSpinBox *spinboxOffsetChannelD;

    QComboBox *comboboxGainOUTChannelA;
    QComboBox *comboboxGainOUTChannelB;
    QComboBox *comboboxGainOUTChannelC;
    QComboBox *comboboxGainOUTChannelD;

    QDoubleSpinBox *spinboxGainChannelA;
    QDoubleSpinBox *spinboxGainChannelB;
    QDoubleSpinBox *spinboxGainChannelC;
    QDoubleSpinBox *spinboxGainChannelD;

    QLabel *labelGainINChannelA;
    QLabel *labelGainINChannelB;
    QLabel *labelGainINChannelC;
    QLabel *labelGainINChannelD;

    QComboBox *cbcoupling;

    QLineEdit *lecollectiontime;
    QComboBox *cbcollectiontimemagnitude;
    QComboBox *cbsampleintervall;
    QSpinBox  *sbcaptures;
    QSpinBox  *sbpresample;

    QCheckBox *checkboxAverageCaptures;

    QComboBox *comboboxBlockSequenceBlockNumber;
    QDoubleSpinBox *spinboxBlockSequenceDecrement;
    QDoubleSpinBox *spinboxBlockSequenceDelay;

    bool offsetBoundsInitialUpdated;

    QTableWidgetItem *stepSizeSwitch;

signals:
    void switchStreamSignalLayer(unsigned int curve, bool up);

    void stepSizeChanged(double size);

public slots:

    // public slots to update gui
    void updateGUI();

    void updateGainReferenceVoltage(float gainA, float gainB, float gainC, float gainD);
    void updateGainReferenceVoltage(int channel, float value);

private slots:

    // Settings ui slots
    void channelAchecked(int state);
    void channelBchecked(int state);
    void channelCchecked(int state);
    void channelDchecked(int state);

    void onCellClicked(int r,int c);
    void pasteClipBoard();
    void changeGainIncrement();

    void couplingChanged(int index);
    void collectionTimeChanged(QString text);
    void sampleIntervallChanged(int index);
    void capturesChanged(int value);
    void presampleChanged(int value);
    void rangeChanged2(int index);

    void onLinkSettingsStateChanged(int state);

    void onOffsetEdited(QString text);

    void onOffsetValueChanged(double value);

    // private slots to update gui
    void updateRange(QList<QString> rangeList);
    void updateCouplingCB(QList<QString> couplingList, int index);
    void updateOffsets();
    void updateCollectionTime(double ct);
    void updateSampleIntervall(unsigned int si);
    void updateCaptures(unsigned int captures);
    void updatePresample(unsigned int percentage);

    void updateOffsetBounds(PSChannel channel);

    void onChannelVisibilityChanged();
    void onChannelLayerUp();
    void onChannelLayerDown();

    void onSettingsRangeChanged(PSChannel channel);
    void onAverageCapturesStateChanged();

    void onBlockSequenceBlockNumber(int idx);
    void onBlockSequenceDecrementChanged(double value);
    void onBlockSequenceDelayChanged(double value);

    void onIOSettingsChanged();
    void onGUISettingsChanged();

#ifdef LIISIM_NIDAQMX
    void onDeviceListUpdated();
    void onSpinboxGainValueChanged();
    void onAnalogOutLimitChanged();
    void onComboboxGainChannelChanged();

    void onAnalogInputReset();
#endif

};

#endif // PICOSCOPESETTINGSWIDGET_H
