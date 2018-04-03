#ifndef MRUNDETAILSWIDGET_H
#define MRUNDETAILSWIDGET_H

#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

class MRun;
class NumberLineEdit;
class LIISettingsComboBox;
class UDPEditor;

class MRunDetailsWidget : public QTreeWidget
{
    Q_OBJECT
public:
    MRunDetailsWidget(QWidget *parent = 0);

    void setRun(MRun *run);

private:
    MRun *run;

    QTreeWidgetItem *saveSettings;
    QCheckBox *checkboxOverwrite;
    QPushButton *buttonForceSave;

    QTreeWidgetItem *runName;
    QLineEdit *editRunName;

    QTreeWidgetItem *runDescription;
    QLineEdit *editRunDescription;

    QTreeWidgetItem *laserFluence;
    NumberLineEdit *editLaserFluence;

    QTreeWidgetItem *liiSettings;
    LIISettingsComboBox *cbLiisettings;

    QTreeWidgetItem *ndFilter;
    QComboBox *cbNDFilter;

    QTreeWidgetItem *pmtChannelGain;
    QMap<int, QPair<QTreeWidgetItem*, NumberLineEdit*>> editPMTGainList;

    QTreeWidgetItem *pmtChannelMeasured;
    QMap<int, QTreeWidgetItem*> pmtMeasuredGainList;

    QTreeWidgetItem *acquisitionMode;

    QTreeWidgetItem *laserSetpoint;

    QTreeWidgetItem *laserPosition;

    QTreeWidgetItem *importDirectory;

    QTreeWidgetItem *loadedFiles;

    QTreeWidgetItem *userDefinedParameters;
    QPushButton *buttonEditUDP;

    UDPEditor *udpEditor;

    void updateName();
    void updateDescription();
    void updateLIISettings();
    void updateFilter();
    void updateLaserFluence();
    void updatePMTGain();
    void updateSignalIO();
    void updateUDP();

private slots:
    void onCheckboxOverwriteStateChanged(int state);
    void onButtonForceSaveClicked();
    void onButtonEditUDPClicked();
    void onLineEditingFinished();
    void onLIISettingsChanged();
    void onFilterChanged();
    void onPMTGainEditingFinished();

    void onMRunDataChanged(int pos, QVariant value);
    void onMRunDestroyed();

    void onGUISettingsChanged();
};

#endif // MRUNDETAILSWIDGET_H
