#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QList>
#include <QGridLayout>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QScrollArea>
#include <QToolButton>
#include <QSplitter>

#include "../utils/labeledcombobox.h"
#include "../utils/liisettingscombobox.h"
#include "../../signal/signalmanager.h"

#include "importdialoghelper.h"

class Core;

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


/**
 * @brief The ImportDialog class
 * @ingroup GUI
 */
class ImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportDialog(Core* core, QWidget *parent = 0);
    Core* core;

    QVBoxLayout* lay_main_v;
    QHBoxLayout* lay_main_h;

    QTabWidget* importTabs;

    /// @brief Cobobox for selection of the destination group
    LabeledComboBox* cbDestGroup;

    /// @brief maps combobox indices to group-IDs.
    QMap<int,int> idxToGroupID;

    LabeledComboBox* liiSettingsComboBox;

    QTextBrowser* helptext;

    QSplitter *mainSplitter;

    //auto csv import
    QCheckBox* boxCopyAbsToRaw;
    QLabel* lbCsvAutoDescr;
    QLabel* lbCsvAutoFname;
    QPushButton* butCsvAutoOpenDir;
    QPushButton* butCsvAutoSelect;
    QCheckBox* checkCsvAutoSubDir;
    LabeledComboBox* choicesCsvAutoDelimiter;
    LabeledComboBox* choicesCsvAutoDecimal;
    QCheckBox* checkLoadRaw;
    QCheckBox* checkLoadAbs;
    QCheckBox *checkboxSubDir;

    // custom import
    QLabel* lbCustomImportDescr;
    QLabel* lbCustomImportDirname;
    QPushButton* butCustomImportOpenDir;
    QPushButton* butCustomImportSelect;

    QLabel* labelCustomFnamePattern;
    QLineEdit* inputCustomFname_text_1;
    QLineEdit* inputCustomFname_text_2;
    QLineEdit* inputCustomFname_text_3;
    QLineEdit* inputCustomFname_text_4;
    LabeledComboBox* inputCustomFname_var_1;
    LabeledComboBox* inputCustomFname_var_2;
    LabeledComboBox* inputCustomFname_var_3;
    LabeledComboBox* inputCustomFnameExtension;

    QCheckBox* checkboxChannelPerFile;
    QCheckBox* checkboxAutoHeader;

    QLabel* labelCustomDelimiter;
    LabeledComboBox* choicesCustomDelimiter;
    QLabel* labelCustomDecimal;
    LabeledComboBox* choicesCustomDecimal;    
    QLabel* labelCustomTimeUnit;
    LabeledComboBox* choicesCustomTimeUnit;
    QCheckBox* checkboxIncludeTime;

    // general
    QLabel* info;

    //csv import
    QList<QLabel*> labels;
    QList<QPushButton*> buttons;

    // ok,cancel buttons
    QHBoxLayout* lay_buts;
    QPushButton* butOk;
    QPushButton* butCancel;

    QScrollArea *scrollAreaImportInfo;
    QWidget *widgetImportInfo;
    QVBoxLayout *layoutImportInfo;
    QWidget *selectorButtonsWidget;
    QPushButton *buttonSelectAllFiles;
    QPushButton *buttonSelectNoneFiles;
    QPushButton *buttonCheckFiles;
    QLabel *labelNoFilesFound;

    QGroupBox *groupboxImportInfo;

    QGridLayout *layoutCSVImport;

    QWidget *widgetGeneralSettings;
    QWidget *widgetCSVAutoImport;
    QWidget *widgetCustomImport;
    QWidget *widgetCSVImport;
    //QWidget *widgetMATImport;


private:
    QLabel* getLabelFromButtonName(QString bname);
    QString getLastDirectory(int idx);
    LIISettings currentLIISettings();

    bool checkImportRequestSuccessful;
    int currentImportType;

    QList<SignalImportInfoElement*> listSIIE;

    static int importGroupID;

signals:
    void signalGuiImportRequest(SignalIORequest irq);
    void signalCanceled();

    void checkImportRequest(SignalIORequest irq);
    void loadImportRequests(QList<SignalIORequest> irq);

public slots:
    void onSelectFile();
    void onOpenFile();

    void onOk();
    void onCancel();

    void onCheckImportRequestResults(QList<SignalIORequest> results);

    void onIOImportSuccess(SignalIORequest source, SignalFileInfoList fileList);
    void onIOImportError(SignalIORequest source, SignalFileInfo file, QString error);

private slots:
    void onGuiStateChangedCombobox(int idx);
    void onGuiStateChanged();
    void onLiiSettingsSelectionChanged(int idx);
    void onCopyRawToAbsChanged(int state);

    void onButtonCheckFilesClicked();
    void onButtonSelectFilesClicked();

    void onCurrentImportTabChanged(int current);

    void onImportGroupChanged(int idx);

};

#endif // IMPORTDIALOG_H
