#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QGridLayout>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QMap>
#include <QRadioButton>
#include "../dataItemViews/treeview/dataitemtreeview.h"
#include "../utils/labeledcombobox.h"
#include "../../signal/signalmanager.h"


// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


/**
 * @brief The ExportDialog class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class ExportDialog : public QDialog
{
    Q_OBJECT


public:
    explicit ExportDialog(QWidget *parent = 0);

    QVBoxLayout* lay_main_v;

    LabeledComboBox* exportSelector;    
    DataItemTreeView* runTree;
    QHBoxLayout* layDirSelect;
    QLabel* lbDirDescr;
    QLabel* lbDir;
    QPushButton* butDir;
    QCheckBox *checkboxMatCompression;

    // group-/check-/radioboxes for data selection
    QGroupBox* boxDataSel;
    QCheckBox* dscb_raw;
    QCheckBox* dscb_abs;
    QCheckBox* dscb_tmp;
    QRadioButton* dsrb_raw_pre;
    QRadioButton* dsrb_raw_post;
    QRadioButton* dsrb_abs_pre;
    QRadioButton* dsrb_abs_post;
    QRadioButton* dsrb_temp_pre;
    QRadioButton* dsrb_temp_post;

    QCheckBox *checkboxRawUnprocessed;
    QCheckBox *checkboxRawProcessed;
    QCheckBox *checkboxRawStdev;

    QCheckBox *checkboxAbsUnprocessed;
    QCheckBox *checkboxAbsProcessed;
    QCheckBox *checkboxAbsStdev;

    QCheckBox *checkboxTempUnprocessed;
    QCheckBox *checkboxTempProcessed;
    QCheckBox *checkboxTempStdev;

    QList<QLabel*> labels;
    QList<QPushButton*> buttons;


    // ok,cancel buttons
    QHBoxLayout* lay_buts;
    QPushButton* butOk;
    QPushButton* butCancel;

private:

    QMap<int,int> indexToIdMap;

signals:

    void signalExportRequest(SignalIORequest irq);
    void signalCanceled();

private slots:

    void onSelectionChanged(int idx);    
    void onSelectDir();

    void onOk();
    void onCancel();

    void onDataSelCheckBoxToggled(bool state);
    void onDataSelRadioButtonToggled(bool state);
};

#endif // EXPORTDIALOG_H
