#ifndef DA_EXPORTSETTINGSWIDGET_H
#define DA_EXPORTSETTINGSWIDGET_H

#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>

#include "../../io/signaliorequest.h"

class MRun;

/**
 * @brief The DA_ExportSettingsWidget class provides a user
 * interface allowing the user to specify export format,
 * runname, data location etc. for runs acquired by the PicoscopeApi
 * (see DataAcquisitionWindow)
 */
class DA_ExportSettingsWidget : public QTableWidget
{
    Q_OBJECT

public:

    explicit DA_ExportSettingsWidget(QWidget *parent = 0);
    ~DA_ExportSettingsWidget();

    SignalIORequest generateExportRequest(MRun* run, bool stdev = false);

    void generateRunName();

    SignalIOType exportType();
    QString runName();
    QString exportDirectory();

    bool autoSave();
    void autoRefresh();

    bool saveStdev();

private:

    QComboBox* cbExpType;
    QCheckBox* checkBoxAutoSave;
    QCheckBox* checkBoxAutoRefreshFileName;
    QCheckBox* checkboxSaveStdev;

signals:

public slots:

private slots:

    void onGuiSettingsChanged();
    void onItemDoubleClicked(QTableWidgetItem* item);
    void onExportTypeSelectionChanged(const QString & text);
    void onAutoSaveStateChanged(int state);
    void onAutoRefreshStateChanged(int state);
    void refreshRunName(int r,int c);
};

#endif // DA_EXPORTSETTINGSWIDGET_H
