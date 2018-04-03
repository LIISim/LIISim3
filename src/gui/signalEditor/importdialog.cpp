#include "importdialog.h"
#include "../../core.h"
#include <QFileDialog>
#include <QDebug>
#include <QHBoxLayout>
#include <QDesktopServices>
#include "../utils/helpmanager.h"
#include "../../signal/mrungroup.h"
#include <QHeaderView>

/*
 * NOTICE: If you want to change the tab order or add another import type/tab,
 * adjust the values in the onCurrentImportTabChanged-method accordingly.
 */

int ImportDialog::importGroupID = -1;

ImportDialog::ImportDialog(Core* core,QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Import Signals"));
    setModal(true);
    setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    resize(900,350);

    this->core = core;

    checkImportRequestSuccessful = false;

    lay_main_h = new QHBoxLayout;
    lay_main_h->setMargin(7);
    setLayout(lay_main_h);

    lay_main_v = new QVBoxLayout;
    lay_main_v->setMargin(0);

    mainSplitter = new QSplitter();
    lay_main_h->addWidget(mainSplitter);
    QWidget *widgetLeft = new QWidget(this);
    widgetLeft->setLayout(lay_main_v);

    mainSplitter->addWidget(widgetLeft);

    // help box (right)
    helptext = new QTextBrowser;
    mainSplitter->addWidget(helptext);
    helptext->setMinimumWidth(500);

    // widget for general settings

    widgetGeneralSettings = new QWidget(this);
    QVBoxLayout *layoutGeneralSettings = new QVBoxLayout;
    layoutGeneralSettings->setMargin(0);
    widgetGeneralSettings->setLayout(layoutGeneralSettings);

    // import type tab widget

    importTabs = new QTabWidget;
    layoutGeneralSettings->addWidget(importTabs);

    // destination group combobox

    cbDestGroup = new LabeledComboBox(tr("Destination group: "));
    layoutGeneralSettings->addWidget(cbDestGroup);

    int destGroupIndex = 0;

    idxToGroupID.insert(0,-1);
    cbDestGroup->addStringItem("Create new group");

    QList<MRunGroup*> groups = core->dataModel()->groupList();
    for(int i = 0; i < groups.size();i++)
    {
        if(groups.at(i)->id() == importGroupID)
            destGroupIndex = i+1;
        idxToGroupID.insert(i+1,groups.at(i)->id());
        cbDestGroup->addStringItem(groups.at(i)->title());
    }

    cbDestGroup->setCurrentIndex(destGroupIndex);

    // combobox for selection of LiiSettings

    liiSettingsComboBox = new LabeledComboBox("Default LIISettings (if settings are not available):");
    layoutGeneralSettings->addWidget(liiSettingsComboBox);
    QList<DatabaseContent*>* ls = Core::instance()->getDatabaseManager()->getLIISettings();
    liiSettingsComboBox->setDatabaseContent(ls);
    liiSettingsComboBox->slot_onDBcontentChanged();
    connect(liiSettingsComboBox,
            SIGNAL(currentIndexChanged(int)),
            SLOT(onLiiSettingsSelectionChanged(int)));

    lay_main_v->addWidget(widgetGeneralSettings);
    lay_main_v->setAlignment(widgetGeneralSettings, Qt::AlignTop);

    // info label

    info = new QLabel("info");
    layoutGeneralSettings->addWidget(info);

    // button 'Scan Files'

    QHBoxLayout *buttonCheckFilesLayout = new QHBoxLayout;
    buttonCheckFilesLayout->addStretch(-1);
    buttonCheckFilesLayout->setMargin(0);
    buttonCheckFiles = new QPushButton("Scan Files");
    buttonCheckFiles->setAutoDefault(true);
    buttonCheckFilesLayout->addWidget(buttonCheckFiles);
    lay_main_v->addLayout(buttonCheckFilesLayout);

    // box which shows found measurement runs

    groupboxImportInfo = new QGroupBox("Measurement runs available for import");
    QVBoxLayout *layoutGroupboxImportInfo = new QVBoxLayout;
    layoutGroupboxImportInfo->setContentsMargins(QMargins(0, 5, 0, 3));
    groupboxImportInfo->setLayout(layoutGroupboxImportInfo);
    lay_main_v->addWidget(groupboxImportInfo);

    selectorButtonsWidget = new QWidget;
    QHBoxLayout *selectorButtonsLayout = new QHBoxLayout;
    selectorButtonsWidget->setLayout(selectorButtonsLayout);
    selectorButtonsLayout->setContentsMargins(QMargins(12, 0, 0, 0));
    buttonSelectAllFiles = new QPushButton("Select All  ");
    buttonSelectAllFiles->setDefault(false);
    buttonSelectAllFiles->setMaximumHeight(18);
    buttonSelectNoneFiles = new QPushButton("  Select None  ");
    buttonSelectNoneFiles->setDefault(false);
    buttonSelectNoneFiles->setMaximumHeight(18);
    buttonSelectNoneFiles->setStyleSheet("QPushButton { border-color : green }");
    selectorButtonsLayout->addWidget(buttonSelectAllFiles);
    selectorButtonsLayout->addWidget(buttonSelectNoneFiles);
    layoutGroupboxImportInfo->addWidget(selectorButtonsWidget);
    layoutGroupboxImportInfo->setAlignment(selectorButtonsWidget, Qt::AlignLeft);

    scrollAreaImportInfo = new QScrollArea();
    scrollAreaImportInfo->setStyleSheet("QScrollArea { border : 0px }");
    scrollAreaImportInfo->setMinimumSize(200, 200);
    layoutGroupboxImportInfo->addWidget(scrollAreaImportInfo);

    widgetImportInfo = new QWidget();
    layoutImportInfo = new QVBoxLayout();
    layoutImportInfo->setSizeConstraint(QLayout::SetFixedSize);
    widgetImportInfo->setLayout(layoutImportInfo);

    scrollAreaImportInfo->setWidget(widgetImportInfo);

    labelNoFilesFound = new QLabel("  *** No files found. Check your import settings and try again. ***  ");
    layoutImportInfo->addWidget(labelNoFilesFound);

    // buttons box (bottom)

    lay_buts = new QHBoxLayout;
    lay_main_v->addLayout(lay_buts);
    lay_main_v->setAlignment(lay_buts, Qt::AlignBottom);
    butOk = new QPushButton(tr("Import"));
    butCancel = new QPushButton(tr("Close"));
    lay_buts->addStretch(-1);
    lay_buts->addWidget(butOk);
    lay_buts->addWidget(butCancel);

    connect(butOk, SIGNAL(released()), SLOT(onOk()));
    connect(butCancel, SIGNAL(released()), SLOT(onCancel()));

    // initialize standard settings

    LIISettings liiSettings = core->modelingSettings->defaultLiiSettings();

    int nochannels = liiSettings.channels.size();

    info->setText(tr("LIISettings supports zero channels! "));

    QString val;
    val.sprintf("%d",nochannels);
    info->setText("LIISettings: " + liiSettings.name + " ("+val+" channels)");

    // fill up all import boxes with required number of inputs

    //*********
    // CSV BOX
    //*********

    layoutCSVImport = new QGridLayout;
    widgetCSVImport = new QWidget();
    widgetCSVImport->setLayout(layoutCSVImport);
    //importTabs->addTab(widgetCSVImport, "CSV");

    QLabel* rawLabel = new QLabel("raw signals: ");
    layoutCSVImport->addWidget(rawLabel,0,0);
    labels.append(rawLabel);
    for(int i=0; i< nochannels; i++)
    {
        QString str;
        str.sprintf("    channel %d (%d nm):",i+1,liiSettings.channels.at(i).wavelength);

        QLabel* txt = new QLabel(str);
        labels.append(txt);

        layoutCSVImport->addWidget(txt,i+1,0);

        QLabel* lb = new QLabel("not set");
        str.sprintf("csvrawtxt%d",i+1);
        lb->setObjectName(str);
        labels.append(lb);
        layoutCSVImport->addWidget(lb,i+1,1,1,1,Qt::AlignLeft);

        QPushButton* b = new QPushButton("Set");
        b->setMaximumWidth(50);
        str.sprintf("csvrawbut%d",i+1);
        b->setObjectName(str);
        buttons.append(b);
        layoutCSVImport->addWidget(b,i+1,2);
        connect(b,SIGNAL(released()),this,SLOT(onSelectFile()));
    }

    QLabel* absLabel = new QLabel("absolute signals: ");
    layoutCSVImport->addWidget(absLabel,1+nochannels,0);
    labels.append(absLabel);

    for(int i=0; i< nochannels; i++)
    {
        QString str;
        str.sprintf("    channel %d (%d nm):",i+1,liiSettings.channels.at(i).wavelength);

        QLabel* txt = new QLabel(str);
        labels.append(txt);
        layoutCSVImport->addWidget(txt,i+1+1+nochannels,0);

        QLabel* lb = new QLabel("not set");
        str.sprintf("csvabstxt%d",i+1);
        lb->setObjectName(str);
        labels.append(lb);
        layoutCSVImport->addWidget(lb,i+1+1+nochannels,1,1,1,Qt::AlignLeft);

        QPushButton* b = new QPushButton("Set");
        b->setMaximumWidth(50);
        str.sprintf("csvabsbut%d",i+1);
        b->setObjectName(str);
        buttons.append(b);
        layoutCSVImport->addWidget(b,i+1+1+nochannels,2);
        connect(b,SIGNAL(released()),this,SLOT(onSelectFile()));
    }

    //*********
    // CSV AUTO IMPORT BOX
    //*********
    QGridLayout *layoutCSVAutoimport = new QGridLayout;
    widgetCSVAutoImport = new QWidget();
    widgetCSVAutoImport->setLayout(layoutCSVAutoimport);
    importTabs->addTab(widgetCSVAutoImport,"CSV AUTO");

    lbCsvAutoDescr = new QLabel("Data directory:");
    lbCsvAutoFname = new QLabel(getLastDirectory(SignalIOType::CSV_SCAN));
    butCsvAutoOpenDir = new QPushButton(QIcon(Core::rootDir + "resources/icons/folder_explore.png"), "");
    butCsvAutoOpenDir->setMaximumWidth(100);
    butCsvAutoOpenDir->setObjectName("butCsvAutoOpenDir");
    butCsvAutoSelect = new QPushButton("Choose Directory");
    butCsvAutoSelect->setMaximumWidth(95);
    butCsvAutoSelect->setObjectName("butCsvAuto");

    checkCsvAutoSubDir = new QCheckBox("Include subdirectories for scan");
    checkCsvAutoSubDir->setObjectName("CsvAutoSubDir");
    checkCsvAutoSubDir->setChecked(
                core->guiSettings->value("idiag/csv","subdirs").toBool());

    connect(checkCsvAutoSubDir,SIGNAL(clicked()),SLOT(onGuiStateChanged()));

    choicesCsvAutoDelimiter = new LabeledComboBox(tr("Select data delimiter: "));
    choicesCsvAutoDelimiter->setObjectName("CsvAutoDelimiter");
    choicesCsvAutoDelimiter->addStringItem(";");
    choicesCsvAutoDelimiter->addStringItem(",");
    choicesCsvAutoDelimiter->setCurrentIndex(
                core->guiSettings->value("idiag/csv","delimiter").toInt());

    connect(choicesCsvAutoDelimiter,SIGNAL(currentIndexChanged(int)),  SLOT(onGuiStateChangedCombobox(int)));

    choicesCsvAutoDecimal = new LabeledComboBox(tr("Select decimal mark: "));
    choicesCsvAutoDecimal->setObjectName("CsvAutoDecimal");
    choicesCsvAutoDecimal->addStringItem(",");
    choicesCsvAutoDecimal->addStringItem(".");
    choicesCsvAutoDecimal->setCurrentIndex(
                core->guiSettings->value("idiag/csv","decimal").toInt());

    connect(choicesCsvAutoDecimal,SIGNAL(currentIndexChanged(int)),
            SLOT(onGuiStateChangedCombobox(int)));

    connect(butCsvAutoOpenDir,SIGNAL(released()),this,SLOT(onOpenFile()));
    connect(butCsvAutoSelect,SIGNAL(released()),this,SLOT(onSelectFile()));
    labels.push_back(lbCsvAutoFname);
    buttons.push_back(butCsvAutoSelect);

    checkLoadRaw = new QCheckBox("Load raw signals");
    checkLoadRaw->setToolTip("import of raw signal data (if available)");

    if(core->guiSettings->hasEntry("idiag/csv","loadRaw"))
        checkLoadRaw->setChecked(core->guiSettings->value("idiag/csv","loadRaw").toBool());
    else
        checkLoadRaw->setChecked(true);

    checkLoadAbs = new QCheckBox("Load absolute signals");
    checkLoadAbs->setToolTip("import of absolute signal data (if available)");
    if(core->guiSettings->hasEntry("idiag/csv","loadAbs"))
        checkLoadAbs->setChecked(core->guiSettings->value("idiag/csv","loadAbs").toBool());
    else
        checkLoadAbs->setChecked(true);

    boxCopyAbsToRaw = new QCheckBox("Copy raw data to absolute data");
    boxCopyAbsToRaw->setChecked(core->guiSettings->value("idiag/csv","copyRawToAbs").toBool());
    connect(boxCopyAbsToRaw,SIGNAL(stateChanged(int)),SLOT(onCopyRawToAbsChanged(int)));

    QLabel *labelCSVFR = new QLabel("<br><b>Caution: Import requires a specific file pattern, please check the right info box!</b>", this);

    QHBoxLayout *layoutAutoCsvDirectory = new QHBoxLayout;
    layoutAutoCsvDirectory->addWidget(lbCsvAutoDescr);
    layoutAutoCsvDirectory->addWidget(lbCsvAutoFname);
    layoutAutoCsvDirectory->addStretch(-1);
    layoutAutoCsvDirectory->addWidget(butCsvAutoOpenDir);
    layoutAutoCsvDirectory->addWidget(butCsvAutoSelect);

    layoutCSVAutoimport->addLayout(layoutAutoCsvDirectory, 0, 0, 1, 3);
    layoutCSVAutoimport->addWidget(checkCsvAutoSubDir,1,0);
    layoutCSVAutoimport->addWidget(choicesCsvAutoDelimiter,2,0);
    layoutCSVAutoimport->addWidget(choicesCsvAutoDecimal,3,0);
    layoutCSVAutoimport->addWidget(checkLoadRaw,4,0);
    layoutCSVAutoimport->addWidget(checkLoadAbs,5,0);
    layoutCSVAutoimport->addWidget(boxCopyAbsToRaw,6,0);
    layoutCSVAutoimport->setRowStretch(7, 100);
    layoutCSVAutoimport->addWidget(labelCSVFR, 9, 0);
    layoutCSVAutoimport->setRowStretch(10, 100);

    //*********
    // CUSTOM IMPORT BOX
    //*********

    // single file or directory (TODO: single file selection)

    QGridLayout *layoutCustomImport = new QGridLayout;
    widgetCustomImport = new QWidget();
    widgetCustomImport->setLayout(layoutCustomImport);
    importTabs->addTab(widgetCustomImport, "CUSTOM");

    lbCustomImportDescr = new QLabel("Data directory:");
    lbCustomImportDirname = new QLabel(getLastDirectory(SignalIOType::CUSTOM));
    butCustomImportOpenDir = new QPushButton(QIcon(Core::rootDir + "resources/icons/folder_explore.png"), "");
    butCustomImportOpenDir->setMaximumWidth(100);
    butCustomImportOpenDir->setObjectName("butCustomImportOpenDir");
    butCustomImportSelect = new QPushButton("Choose Directory");
    butCustomImportSelect->setMaximumWidth(95);
    butCustomImportSelect->setObjectName("butCustomImport");

    connect(butCustomImportOpenDir,SIGNAL(released()),this,SLOT(onOpenFile()));
    connect(butCustomImportSelect,SIGNAL(released()),this,SLOT(onSelectFile()));
    labels.push_back(lbCustomImportDirname);
    buttons.push_back(butCustomImportSelect);

    checkboxSubDir = new QCheckBox("Include subdirectories for scan");
    checkboxSubDir->setToolTip("If checked, scan will also include files in subdirectories");
    checkboxSubDir->setChecked(core->guiSettings->value("idiag/custom", "subdirs").toBool());
    connect(checkboxSubDir, SIGNAL(clicked()), SLOT(onGuiStateChanged()));

    //!!! TODO add tooltips
    labelCustomFnamePattern = new QLabel(tr("Filename:"));
    labelCustomFnamePattern->setMaximumWidth(100);

    inputCustomFname_text_1    = new QLineEdit("C");
    inputCustomFname_var_1  = new LabeledComboBox();
    inputCustomFname_text_2  = new QLineEdit("_Run2_");
    inputCustomFname_var_2  = new LabeledComboBox();
    inputCustomFname_text_3  = new QLineEdit("");
    inputCustomFname_var_3  = new LabeledComboBox();
    inputCustomFname_text_4  = new QLineEdit("");
    inputCustomFnameExtension  = new LabeledComboBox();

    //inputCustomFname_text_1->setMaximumWidth(40);
    //inputCustomFname_text_2->setMaximumWidth(40);
    //inputCustomFname_text_3->setMaximumWidth(40);
    //inputCustomFname_text_4->setMaximumWidth(40);

    inputCustomFname_text_1->setMinimumWidth(40);
    inputCustomFname_text_2->setMinimumWidth(40);
    inputCustomFname_text_3->setMinimumWidth(40);
    inputCustomFname_text_4->setMinimumWidth(40);

    //inputCustomFname_var_1->setMaximumWidth(100);
    //inputCustomFname_var_2->setMaximumWidth(100);
    //inputCustomFname_var_3->setMaximumWidth(100);
    inputCustomFname_var_1->setMinimumWidth(100);
    inputCustomFname_var_2->setMinimumWidth(100);
    inputCustomFname_var_3->setMinimumWidth(100);
    inputCustomFnameExtension->setMaximumWidth(60);

    inputCustomFname_text_1->setAlignment(Qt::AlignRight);
    inputCustomFname_text_2->setAlignment(Qt::AlignCenter);
    inputCustomFname_text_3->setAlignment(Qt::AlignCenter);
    inputCustomFname_text_4->setAlignment(Qt::AlignCenter);

    inputCustomFname_text_1->setObjectName("CustomFname_T1");
    inputCustomFname_text_2->setObjectName("CustomFname_T2");
    inputCustomFname_text_3->setObjectName("CustomFname_T3");
    inputCustomFname_text_4->setObjectName("CustomFname_T4");
    inputCustomFname_var_1->setObjectName("CustomFname_V1");
    inputCustomFname_var_2->setObjectName("CustomFname_V2");
    inputCustomFname_var_3->setObjectName("CustomFname_V3");
    inputCustomFnameExtension->setObjectName("CustomFname_Ext");

    inputCustomFname_var_1->addStringItem(" ");
    inputCustomFname_var_1->addStringItem("<CHANNEL>");
    inputCustomFname_var_1->addStringItem("<RUN>");
    inputCustomFname_var_1->addStringItem("<SIGNAL>");
    //inputCustomFname_var_1->addStringItem("<PMT_GAIN>");
    //inputCustomFname_var_1->addStringItem("<FLUENCE>");
    inputCustomFname_var_1->addStringItem("<INTEGER>");
    inputCustomFname_var_1->addStringItem("<STRING>");
    inputCustomFname_var_1->setCurrentIndex(1);

    inputCustomFname_var_2->addStringItem(" ");
    inputCustomFname_var_2->addStringItem("<CHANNEL>");
    inputCustomFname_var_2->addStringItem("<RUN>");
    inputCustomFname_var_2->addStringItem("<SIGNAL>");
    //inputCustomFname_var_2->addStringItem("<PMT_GAIN>");
    //nputCustomFname_var_2->addStringItem("<FLUENCE>");
    inputCustomFname_var_2->addStringItem("<INTEGER>");
    inputCustomFname_var_2->addStringItem("<STRING>");
    inputCustomFname_var_2->setCurrentIndex(3);

    inputCustomFname_var_3->addStringItem(" ");
    inputCustomFname_var_3->addStringItem("<CHANNEL>");
    inputCustomFname_var_3->addStringItem("<RUN>");
    inputCustomFname_var_3->addStringItem("<SIGNAL>");
    //inputCustomFname_var_3->addStringItem("<PMT_GAIN>");
    //inputCustomFname_var_3->addStringItem("<FLUENCE>");
    inputCustomFname_var_3->addStringItem("<INTEGER>");
    inputCustomFname_var_3->addStringItem("<STRING>");
    inputCustomFname_var_3->setCurrentIndex(0);

    inputCustomFnameExtension->addStringItem(".txt");
    inputCustomFnameExtension->addStringItem(".dat");
    inputCustomFnameExtension->addStringItem(".csv");
    inputCustomFnameExtension->setCurrentIndex(0);

    //labelSkipHeaderlines = new QLabel(tr("Number of header lines: "));
    //labelSkipHeaderlines->setMaximumWidth(130);
    //inputSkipHeaderlines = new QLineEdit("0");
    //inputSkipHeaderlines->setObjectName("CustomSkipHeader");
    //inputSkipHeaderlines->setMaximumWidth(30);

    // TODO: number of channels (if less than LII-Settings shows)

    // TODO: load/save GUIsettings
    checkboxChannelPerFile = new QCheckBox("Only one channel per file");
    checkboxChannelPerFile->setToolTip("File contains only data for one channel");
    checkboxChannelPerFile->setObjectName("CustomChannelPerFile");
    checkboxChannelPerFile->setChecked(core->guiSettings->value("idiag/custom","oneChannelPerFile").toBool());

    connect(checkboxChannelPerFile,SIGNAL(clicked()),SLOT(onGuiStateChanged()));

    checkboxAutoHeader = new QCheckBox("Detect header lines automatically");
    checkboxAutoHeader->setToolTip("Skips lines if it contains any non-digit characters (except for E)");
    checkboxAutoHeader->setObjectName("CustomAutoHeader");
    checkboxAutoHeader->setChecked(true);
    //user should not switch that on yet: needs additional security checks to prevent runtime error
    checkboxAutoHeader->setEnabled(false);

    labelCustomDelimiter = new QLabel(tr("Select data delimiter: "));
    choicesCustomDelimiter = new LabeledComboBox();
    choicesCustomDelimiter->setObjectName("CustomDelimiter");
    choicesCustomDelimiter->addStringItem(";");
    choicesCustomDelimiter->addStringItem(",");
    choicesCustomDelimiter->addStringItem("tab");
    choicesCustomDelimiter->setMaximumWidth(60);
    choicesCustomDelimiter->setCurrentIndex(core->guiSettings->value("idiag/custom","delimiter").toInt());

    labelCustomDecimal = new QLabel(tr("Select decimal mark: "));
    choicesCustomDecimal = new LabeledComboBox();
    choicesCustomDecimal->setObjectName("CustomDecimal");
    choicesCustomDecimal->addStringItem(".");
    choicesCustomDecimal->addStringItem(",");
    choicesCustomDecimal->setMaximumWidth(60);
    choicesCustomDecimal->setCurrentIndex(core->guiSettings->value("idiag/custom","decimal").toInt());

    labelCustomTimeUnit = new QLabel(tr("Select time unit:"));
    choicesCustomTimeUnit = new LabeledComboBox();
    choicesCustomTimeUnit->setObjectName("CustomTimeUnit");
    choicesCustomTimeUnit->addStringItem("ns");
    choicesCustomTimeUnit->addStringItem("µs");
    choicesCustomTimeUnit->addStringItem("ms");
    choicesCustomTimeUnit->addStringItem("s");
    choicesCustomTimeUnit->setMaximumWidth(60);
    choicesCustomTimeUnit->setCurrentIndex(core->guiSettings->value("idiag/custom","timeunit").toInt());

    checkboxIncludeTime = new QCheckBox("Data include time-axis");
    checkboxIncludeTime->setObjectName("CustomIncludeTime");
    checkboxIncludeTime->setChecked(true);
    checkboxIncludeTime->setEnabled(false); //!!!TODO needs additional input for delta time

    QHBoxLayout *layoutCustomDirectory = new QHBoxLayout;
    layoutCustomDirectory->addWidget(lbCustomImportDescr);
    layoutCustomDirectory->addWidget(lbCustomImportDirname);
    layoutCustomDirectory->addStretch(-1);
    layoutCustomDirectory->addWidget(butCustomImportOpenDir);
    layoutCustomDirectory->addWidget(butCustomImportSelect);

    layoutCustomImport->addLayout(layoutCustomDirectory, 0, 0, 1, 3);
    layoutCustomImport->addWidget(checkboxSubDir,            1,0);

    layoutCustomImport->addWidget(checkboxChannelPerFile,    2,0,1,3);

    layoutCustomImport->addWidget(labelCustomFnamePattern,   3,0,1,3); //colspan=3

    QHBoxLayout *laycustom_fname = new QHBoxLayout;
    laycustom_fname->setContentsMargins(QMargins(0, 0, 0, 0));
    laycustom_fname->addWidget(inputCustomFname_text_1);
    laycustom_fname->addWidget(inputCustomFname_var_1);
    laycustom_fname->addWidget(inputCustomFname_text_2);
    laycustom_fname->addWidget(inputCustomFname_var_2);
    laycustom_fname->addWidget(inputCustomFname_text_3);
    laycustom_fname->addWidget(inputCustomFname_var_3);
    laycustom_fname->addWidget(inputCustomFname_text_4);
    laycustom_fname->addWidget(inputCustomFnameExtension);

    QWidget *boxCustomImport_fname = new QWidget;
    boxCustomImport_fname->setLayout(laycustom_fname);
    layoutCustomImport->addWidget(boxCustomImport_fname,     4,0,1,3); //colspan=3
    //layoutCustomImport->setAlignment(boxCustomImport_fname, Qt::AlignLeft);

    //layoutCustomImport->addWidget(labelSkipHeaderlines,      5,0);
    //layoutCustomImport->addWidget(inputSkipHeaderlines,      5,1,Qt::AlignLeft);

    layoutCustomImport->addWidget(checkboxAutoHeader,        6,0,1,3);

    layoutCustomImport->addWidget(labelCustomDelimiter,      7,0);
    layoutCustomImport->addWidget(choicesCustomDelimiter,    7,1,Qt::AlignLeft);

    layoutCustomImport->addWidget(labelCustomDecimal,        8,0);
    layoutCustomImport->addWidget(choicesCustomDecimal,      8,1,Qt::AlignLeft);


    layoutCustomImport->addWidget(labelCustomTimeUnit,        9,0);
    layoutCustomImport->addWidget(choicesCustomTimeUnit,      9,1,Qt::AlignLeft);

    layoutCustomImport->addWidget(checkboxIncludeTime,       10,0,1,3); //colspan=3

    // get default values from GUI settings

    inputCustomFname_var_1->setCurrentIndex(core->guiSettings->value("idiag/custom","var_1").toInt());
    inputCustomFname_var_2->setCurrentIndex(core->guiSettings->value("idiag/custom","var_2").toInt());
    inputCustomFname_var_3->setCurrentIndex(core->guiSettings->value("idiag/custom","var_3").toInt());
    inputCustomFnameExtension->setCurrentIndex(core->guiSettings->value("idiag/custom","ext").toInt());

    inputCustomFname_text_1->setText(core->guiSettings->value("idiag/custom","text_1").toString());
    inputCustomFname_text_2->setText(core->guiSettings->value("idiag/custom","text_2").toString());
    inputCustomFname_text_3->setText(core->guiSettings->value("idiag/custom","text_3").toString());
    inputCustomFname_text_4->setText(core->guiSettings->value("idiag/custom","text_4").toString());

    // connect GUI update functions

    connect(inputCustomFname_var_1,SIGNAL(currentIndexChanged(int)),  SLOT(onGuiStateChangedCombobox(int)));
    connect(inputCustomFname_var_2,SIGNAL(currentIndexChanged(int)),  SLOT(onGuiStateChangedCombobox(int)));
    connect(inputCustomFname_var_3,SIGNAL(currentIndexChanged(int)),  SLOT(onGuiStateChangedCombobox(int)));
    connect(inputCustomFnameExtension,SIGNAL(currentIndexChanged(int)),  SLOT(onGuiStateChangedCombobox(int)));
    connect(inputCustomFname_text_1,SIGNAL(editingFinished()),SLOT(onGuiStateChanged()));
    connect(inputCustomFname_text_2,SIGNAL(editingFinished()),SLOT(onGuiStateChanged()));
    connect(inputCustomFname_text_3,SIGNAL(editingFinished()),SLOT(onGuiStateChanged()));
    connect(inputCustomFname_text_4,SIGNAL(editingFinished()),SLOT(onGuiStateChanged()));

    connect(choicesCustomDelimiter,SIGNAL(currentIndexChanged(int)),SLOT(onGuiStateChangedCombobox(int)));
    connect(choicesCustomDecimal,SIGNAL(currentIndexChanged(int)),SLOT(onGuiStateChangedCombobox(int)));
    connect(choicesCustomTimeUnit,SIGNAL(currentIndexChanged(int)),SLOT(onGuiStateChangedCombobox(int)));

    connect(checkLoadRaw,SIGNAL(clicked()),SLOT(onGuiStateChanged()));
    connect(checkLoadAbs,SIGNAL(clicked()),SLOT(onGuiStateChanged()));
    connect(boxCopyAbsToRaw,SIGNAL(clicked()),SLOT(onGuiStateChanged()));

    QString lastFname = core->guiSettings->value("idiag/liisettings","lastFname").toString();
    int index = Core::instance()->getDatabaseManager()->indexOfLIISettings(lastFname);;
    if(index == -1)
        liiSettingsComboBox->setCurrentItem(Core::instance()->modelingSettings->defaultLiiSettings().ident);
    else
        liiSettingsComboBox->setCurrentItem(Core::instance()->getDatabaseManager()->getLIISetting(index)->ident);

    connect(buttonCheckFiles, SIGNAL(clicked(bool)), SLOT(onButtonCheckFilesClicked()));

    connect(buttonSelectAllFiles, SIGNAL(clicked(bool)), SLOT(onButtonSelectFilesClicked()));
    connect(buttonSelectNoneFiles, SIGNAL(clicked(bool)), SLOT(onButtonSelectFilesClicked()));

    connect(importTabs, SIGNAL(currentChanged(int)), SLOT(onCurrentImportTabChanged(int)));

    connect(cbDestGroup, SIGNAL(currentIndexChanged(int)), SLOT(onImportGroupChanged(int)));

    // recall last import type
    int itype = core->guiSettings->value("idiag","itype").toInt();
    importTabs->blockSignals(true);
    importTabs->setCurrentIndex(itype);
    importTabs->blockSignals(false);

    onCurrentImportTabChanged(importTabs->currentIndex());
}


/**
 * @brief ImportDialog::onLiiSettingsSelectionChanged This slot
 * is executed when the user has changed the current LIISettings
 * @param idx
 */
void ImportDialog::onLiiSettingsSelectionChanged(int idx)
{
    LIISettings liiSettings = currentLIISettings();

    int nochannels = liiSettings.channels.size();
    if(nochannels == 0)
    {
        butOk->setEnabled(false);
        info->setText(tr("LIISettings supports zero channels! "));
    }
    else
    {
        butOk->setEnabled(checkImportRequestSuccessful);

        //QString val;
        //val.sprintf("%d",nochannels);
        //info->setText("LIISettings: " + liiSettings.name + " ("+val+" channels)");
        info->setText(QString("LIISettings: %0 (%1 channels) - %2").arg(liiSettings.name)
                      .arg(nochannels).arg(liiSettings.description));

        // clear csv import -> clear labels/buttons
        labels.removeOne(lbCsvAutoFname);
        buttons.removeOne(butCsvAutoSelect);
        labels.removeOne(lbCustomImportDirname);
        buttons.removeOne(butCustomImportSelect);

        for(int i = 0; i < labels.size(); i++)
            delete labels[i];
        for(int i = 0; i < buttons.size(); i++)
            delete buttons[i];

        labels.clear();
        buttons.clear();

        // add new labels, readd buttons/labels

        labels.push_back(lbCsvAutoFname);
        buttons.push_back(butCsvAutoSelect);
        labels.push_back(lbCustomImportDirname);
        buttons.push_back(butCustomImportSelect);

        QLabel* rawLabel = new QLabel("raw signals: ");
        layoutCSVImport->addWidget(rawLabel,0,0);
        labels.append(rawLabel);
        for(int i=0; i< nochannels; i++)
        {
            QString str;
            str.sprintf("    channel %d (%d nm):",i+1,liiSettings.channels.at(i).wavelength);

            QLabel* txt = new QLabel(str);
            labels.append(txt);

            layoutCSVImport->addWidget(txt,i+1,0);

            QLabel* lb = new QLabel("not set");
            str.sprintf("csvrawtxt%d",i+1);
            lb->setObjectName(str);
            labels.append(lb);
            layoutCSVImport->addWidget(lb,i+1,1,1,1,Qt::AlignLeft);

            QPushButton* b = new QPushButton("Set");
            b->setMaximumWidth(50);
            str.sprintf("csvrawbut%d",i+1);
            b->setObjectName(str);
            buttons.append(b);
            layoutCSVImport->addWidget(b,i+1,2);
            connect(b,SIGNAL(released()),this,SLOT(onSelectFile()));
        }

        QLabel* absLabel = new QLabel("absolute signals: ");
        layoutCSVImport->addWidget(absLabel,1+nochannels,0);
        labels.append(absLabel);

        for(int i=0; i< nochannels; i++)
        {
            QString str;
            str.sprintf("    channel %d (%d nm):",i+1,liiSettings.channels.at(i).wavelength);

            QLabel* txt = new QLabel(str);
            labels.append(txt);
            layoutCSVImport->addWidget(txt,i+1+1+nochannels,0);

            QLabel* lb = new QLabel("not set");
            str.sprintf("csvabstxt%d",i+1);
            lb->setObjectName(str);
            labels.append(lb);
            layoutCSVImport->addWidget(lb,i+1+1+nochannels,1,1,1,Qt::AlignLeft);

            QPushButton* b = new QPushButton("Set");
            b->setMaximumWidth(50);
            str.sprintf("csvabsbut%d",i+1);
            b->setObjectName(str);
            buttons.append(b);
            layoutCSVImport->addWidget(b,i+1+1+nochannels,2);
            connect(b,SIGNAL(released()),this,SLOT(onSelectFile()));
        }
        core->guiSettings->setValue("idiag/liisettings","lastFname",liiSettings.filename);
    }
    onCopyRawToAbsChanged(0);

    for(int i = 0; i < listSIIE.size(); i++)
        listSIIE.at(i)->setGlobalLIISettings(liiSettings);
}


/**
 * @brief ImportDialog::onGuiStateChangedCombobox update GUIsettings
 * @param idx
 */
void ImportDialog::onGuiStateChangedCombobox(int idx)
{
    QObject* sender = QObject::sender();

    if(sender == choicesCsvAutoDecimal)
        core->guiSettings->setValue("idiag/csv","decimal",idx);

    else if(sender == choicesCsvAutoDelimiter)
        core->guiSettings->setValue("idiag/csv","delimiter",idx);

    else if(sender == choicesCustomDecimal)
        core->guiSettings->setValue("idiag/custom","decimal",idx);

    else if(sender == choicesCustomDelimiter)
        core->guiSettings->setValue("idiag/custom","delimiter",idx);

    else if(sender == choicesCustomTimeUnit)
        core->guiSettings->setValue("idiag/custom","timeunit",idx);

    else if(sender == inputCustomFname_var_1)
        core->guiSettings->setValue("idiag/custom","var_1",idx);

    else if(sender == inputCustomFname_var_2)
        core->guiSettings->setValue("idiag/custom","var_2",idx);

    else if(sender == inputCustomFname_var_3)
        core->guiSettings->setValue("idiag/custom","var_3",idx);

    else if(sender == inputCustomFnameExtension)
        core->guiSettings->setValue("idiag/custom","ext",idx);
}


/**
 * @brief ImportDialog::onGuiStateChanged update GUIsettings
 */
void ImportDialog::onGuiStateChanged()
{
    QObject* sender = QObject::sender();

    if(sender == importTabs)
        core->guiSettings->setValue("idiag","itype",importTabs->currentIndex());

    if(sender == checkCsvAutoSubDir)
        core->guiSettings->setValue("idiag/csv","subdirs",checkCsvAutoSubDir->isChecked());

    if(sender == checkLoadRaw)
        core->guiSettings->setValue("idiag/csv","loadRaw",checkLoadRaw->isChecked());

    if(sender == checkLoadAbs)
        core->guiSettings->setValue("idiag/csv","loadAbs",checkLoadAbs->isChecked());

    if(sender == boxCopyAbsToRaw)
        core->guiSettings->setValue("idiag/csv","copyRawToAbs",boxCopyAbsToRaw->isChecked());

    else if(sender == inputCustomFname_text_1)
        core->guiSettings->setValue("idiag/custom","text_1",inputCustomFname_text_1->text());

    else if(sender == inputCustomFname_text_2)
        core->guiSettings->setValue("idiag/custom","text_2",inputCustomFname_text_2->text());

    else if(sender == inputCustomFname_text_3)
        core->guiSettings->setValue("idiag/custom","text_3",inputCustomFname_text_3->text());

    else if(sender == inputCustomFname_text_4)
        core->guiSettings->setValue("idiag/custom","text_4",inputCustomFname_text_4->text());

    else if(sender == checkboxChannelPerFile)
        core->guiSettings->setValue("idiag/custom","oneChannelPerFile",checkboxChannelPerFile->isChecked());

    else if(sender == checkboxSubDir)
        core->guiSettings->setValue("idiag/custom", "subdirs", checkboxSubDir->isChecked());
}


void ImportDialog::onCopyRawToAbsChanged(int state)
{
    if(boxCopyAbsToRaw->isChecked())
    {
        checkLoadAbs->setEnabled(0);
        checkLoadAbs->setChecked(0);
    }
    else
    {
        checkLoadAbs->setChecked(1);
        checkLoadAbs->setEnabled(1);
    }
}


/**
 * @brief ImportDialog::onButtonCheckFilesClicked
 * Sends a request to the SignalManager to scan the selected directory with
 * the selected import routine.
 */
void ImportDialog::onButtonCheckFilesClicked()
{
    checkImportRequestSuccessful = false;

    while(!listSIIE.isEmpty())
    {
        layoutImportInfo->removeWidget(listSIIE.last());
        delete listSIIE.last();
        listSIIE.removeLast();
    }

    int idx = currentImportType;

    SignalFileInfoList flist;

    SignalIORequest irq;

    LIISettings liiSettings = currentLIISettings();

    irq.userData.insert(3,liiSettings.filename);
    irq.noChannels = liiSettings.channels.size();

    irq.group_id = idxToGroupID.value(cbDestGroup->getCurrentIndex());

    if(idx == SignalIOType::CSV_SCAN)
    {
        irq.itype       = CSV_SCAN;
        irq.delimiter   = choicesCsvAutoDelimiter->getCurrentText();
        irq.decimal     = choicesCsvAutoDecimal->getCurrentText();
        irq.datadir     = lbCsvAutoFname->text();
        irq.subdir      = checkCsvAutoSubDir->isChecked();

        irq.userData.insert( 6, checkLoadRaw->isChecked() );
        irq.userData.insert( 7, checkLoadAbs->isChecked() );
        irq.userData.insert(18, boxCopyAbsToRaw->isChecked());
    }
    else if(idx == SignalIOType::CSV)
    {
        irq.itype = CSV;
        for(int i=0; i< labels.size();i++)
        {
           SignalFileInfo curifi;
           QString objName = labels.at(i)->objectName();
           if(objName.contains("csvrawtxt"))
           {
               curifi.itype = CSV;
               curifi.filename = labels.at(i)->text();
               curifi.channelId = objName.remove("csvrawtxt").toInt();;
               curifi.signalType = Signal::RAW;
               if(curifi.filename != "not set")
                   flist.push_back(curifi);
           }
           else if(objName.contains("csvabstxt"))
           {
               curifi.itype = CSV;
               curifi.filename = labels.at(i)->text();
               curifi.channelId = objName.remove("csvabstxt").toInt();;
               curifi.signalType = Signal::ABS;
               if(curifi.filename != "not set")
                   flist.push_back(curifi);
           }
        }
    }
    else if(idx == SignalIOType::MAT)
    {
        qDebug() << "ImportDialog: MAT import not implemented yet!";
        irq.itype = MAT;
    }
    else if(idx == SignalIOType::CUSTOM)
    {
        irq.itype       = CUSTOM;
        irq.delimiter   = choicesCustomDelimiter->getCurrentText();
        irq.decimal     = choicesCustomDecimal->getCurrentText();
        irq.timeunit    = choicesCustomTimeUnit->getCurrentText();

        irq.runname     = "CUSTOM";
        //TODO: irq.noChannels
        irq.datadir     = lbCustomImportDirname->text();
        irq.subdir = checkboxSubDir->isChecked();

        irq.fname_txt_1 = inputCustomFname_text_1->text();
        irq.fname_txt_2 = inputCustomFname_text_2->text();
        irq.fname_txt_3 = inputCustomFname_text_3->text();
        irq.fname_txt_4 = inputCustomFname_text_4->text();
        irq.fname_var_1 = inputCustomFname_var_1->getCurrentText();
        irq.fname_var_2 = inputCustomFname_var_2->getCurrentText();
        irq.fname_var_3 = inputCustomFname_var_3->getCurrentText();
        irq.extension   = inputCustomFnameExtension->getCurrentText();

        //irq.headerlines = inputSkipHeaderlines->text().toInt();
        irq.headerlines = 0;
        irq.autoheader  = checkboxAutoHeader->isChecked();

        irq.channelPerFile = checkboxChannelPerFile->isChecked();

        // TODO: create gui checkbox for this value
        irq.autoName = true;
    }

    irq.flist = flist;

    widgetGeneralSettings->setEnabled(false);
    importTabs->setEnabled(false);

    buttonCheckFiles->setText("Scanning files, please wait...");
    buttonCheckFiles->setEnabled(false);
    butOk->setEnabled(false);

    emit checkImportRequest(irq);
}


/**
 * @brief ImportDialog::onButtonSelectFilesClicked
 */
void ImportDialog::onButtonSelectFilesClicked()
{
    if(QObject::sender() == buttonSelectAllFiles)
    {
        for(SignalImportInfoElement *siie : listSIIE)
            siie->checkboxRunInfo->setChecked(true);
    }
    else if(QObject::sender() == buttonSelectNoneFiles)
    {
        for(SignalImportInfoElement *siie : listSIIE)
            siie->checkboxRunInfo->setChecked(false);
    }
}


/**
 * @brief ImportDialog::onOk if user clicks "import" button
 * sends signal "signalGuiImportRequest(irq)" to SignalManager
 */
void ImportDialog::onOk()
{
    int idx = currentImportType;

    butOk->setEnabled(false);

    QList<SignalIORequest> irq;

    for(int i = 0; i < listSIIE.size(); i++)
    {
        if(!listSIIE.at(i)->checkboxRunInfo->isChecked())
        {
            delete listSIIE.takeAt(i);
            i--;
        }
    }

    if(listSIIE.isEmpty())
    {
        close();
        return;
    }

    int groupID = idxToGroupID.value(cbDestGroup->getCurrentIndex());
    if(groupID == -1)
    {
        MRunGroup *group = new MRunGroup("New Group");
        groupID = group->id();
    }

    if(checkImportRequestSuccessful && idx == SignalIOType::CSV_SCAN)
    {
        for(int i = 0; i < listSIIE.size(); i++)
        {
            listSIIE.at(i)->setImporting();

            irq.push_back(listSIIE.at(i)->request);
            irq.last().group_id = groupID;

            if(!listSIIE.at(i)->comboboxLIISettings->currentIsGlobal())
                irq.last().userData.insert(3, listSIIE.at(i)->comboboxLIISettings->currentLIISettings().filename);
            else
                irq.last().userData.insert(3, currentLIISettings().filename);
        }
    }
    else if(checkImportRequestSuccessful && idx == SignalIOType::CUSTOM)
    {
        for(int i = 0; i < listSIIE.size(); i++)
        {
            listSIIE.at(i)->setImporting();

            irq.push_back(listSIIE.at(i)->request);
            irq.last().group_id = groupID;

            if(!listSIIE.at(i)->comboboxLIISettings->currentIsGlobal())
                irq.last().userData.insert(3, listSIIE.at(i)->comboboxLIISettings->currentLIISettings().filename);
            else
                irq.last().userData.insert(3, currentLIISettings().filename);
        }
    }

    widgetGeneralSettings->setEnabled(false);
    importTabs->setEnabled(false);
    buttonCheckFiles->setEnabled(false);
    butCancel->setEnabled(false);

    selectorButtonsWidget->setVisible(false);
    groupboxImportInfo->setTitle("Loading Data");

    emit loadImportRequests(irq);

    /*SignalFileInfoList flist;

    SignalIORequest irq;

    LIISettings liiSettings = currentLIISettings();

    irq.userData.insert(3,liiSettings.filename);
    irq.noChannels = liiSettings.channels.size();

    irq.group_id = idxToGroupID.value(cbDestGroup->getCurrentIndex());

    if(irq.group_id == -1)
    {
        MRunGroup* gr = new MRunGroup("New group");
        irq.group_id = gr->id();
    }


    if(idx == idx_csvscan)
    {
        irq.itype       = CSV_SCAN;
        irq.delimiter   = choicesCsvAutoDelimiter->getCurrentText();
        irq.decimal     = choicesCsvAutoDecimal->getCurrentText();
        irq.datadir     = lbCsvAutoFname->text();
        irq.subdir      = checkCsvAutoSubDir->isChecked();

        irq.userData.insert( 6, checkLoadRaw->isChecked() );
        irq.userData.insert( 7, checkLoadAbs->isChecked() );
        irq.userData.insert(18, boxCopyAbsToRaw->isChecked());
    }
    else if(idx == idx_csv)
    {
        irq.itype = CSV;
        for(int i=0; i< labels.size();i++)
        {
           SignalFileInfo curifi;
           QString objName = labels.at(i)->objectName();
           if(objName.contains("csvrawtxt"))
           {
               curifi.itype = CSV;
               curifi.filename = labels.at(i)->text();
               curifi.channelId = objName.remove("csvrawtxt").toInt();;
               curifi.signalType = Signal::RAW;
               if(curifi.filename != "not set")
                   flist.push_back(curifi);
           }
           else if(objName.contains("csvabstxt"))
           {
               curifi.itype = CSV;
               curifi.filename = labels.at(i)->text();
               curifi.channelId = objName.remove("csvabstxt").toInt();;
               curifi.signalType = Signal::ABS;
               if(curifi.filename != "not set")
                   flist.push_back(curifi);
           }
        }
    }
    else if(idx == idx_mat)
    {
        qDebug() << "ImportDialog: MAT import not implemented yet!";
        irq.itype = MAT;
    }
    else if(idx == idx_custom)
    {
        irq.itype       = CUSTOM;
        irq.delimiter   = choicesCustomDelimiter->getCurrentText();
        irq.decimal     = choicesCustomDecimal->getCurrentText();
        irq.timeunit    = choicesCustomTimeUnit->getCurrentText();

        irq.runname     = "CUSTOM";
        //TODO: irq.noChannels
        irq.datadir     = lbCustomImportDirname->text();
        //TODO: irq.subdir      = true;

        irq.fname_txt_1 = inputCustomFname_text_1->text();
        irq.fname_txt_2 = inputCustomFname_text_2->text();
        irq.fname_txt_3 = inputCustomFname_text_3->text();
        irq.fname_txt_4 = inputCustomFname_text_4->text();
        irq.fname_var_1 = inputCustomFname_var_1->getCurrentText();
        irq.fname_var_2 = inputCustomFname_var_2->getCurrentText();
        irq.fname_var_3 = inputCustomFname_var_3->getCurrentText();
        irq.extension   = inputCustomFnameExtension->getCurrentText();

        //irq.headerlines = inputSkipHeaderlines->text().toInt();
        irq.headerlines = 0;
        irq.autoheader  = checkboxAutoHeader->isChecked();

        irq.channelPerFile = checkboxChannelPerFile->isChecked();

        // TODO: create gui checkbox for this value
        irq.subdir = true;

        irq.autoName = true;
    }

    irq.flist = flist;
    emit signalGuiImportRequest(irq);
    close();*/
}


/**
 * @brief ImportDialog::currentLIISettings returns the current liisettngs selection or
 * a default selection if the userselection is empty
 * @return
 */
LIISettings ImportDialog::currentLIISettings()
{
    DatabaseContent* dbc = liiSettingsComboBox->getSelectedDbContent();
    if(!dbc)
    {
        LIISettings ls = Core::instance()->modelingSettings->defaultLiiSettings();
        MSG_WARN("ImportDialog: no liisettings selected! Using default LIISettings instead: '"
                 +ls.name+"'!");
        return ls;
    }
    LIISettings ls = *Core::instance()->getDatabaseManager()->liiSetting(dbc->ident);
    return ls;
}

/**
 * @brief ImportDialog::onCancel
 */
void ImportDialog::onCancel()
{
    emit signalCanceled();
    close();
}


/**
 * @brief ImportDialog::onSelectFile
 */
void ImportDialog::onSelectFile()
{
    int idx = currentImportType;

    QObject *sender = QObject::sender();
    QLabel* lb = getLabelFromButtonName(sender->objectName());

    if(lb != NULL)
    {
        QString datDir;

        if(idx == SignalIOType::CSV_SCAN || idx == SignalIOType::CUSTOM)
        {
            if(idx == SignalIOType::CSV_SCAN)
                datDir = core->guiSettings->value("idiag/csv","lastDir").toString();
            else if(idx == SignalIOType::CUSTOM)
                datDir = core->guiSettings->value("idiag/custom","lastDir").toString();

            if(datDir.isEmpty())
                datDir = core->generalSettings->dataDirectory();

            QString dirname = QFileDialog::getExistingDirectory(this,tr("Select Data Directory"),datDir,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

            if(dirname.isEmpty()) return;
            if(!dirname.endsWith("/"))
                dirname.append("/");
            lb->setText(dirname);

            if(idx == SignalIOType::CSV_SCAN)
                core->guiSettings->setValue("idiag/csv","lastDir",dirname);
            if(idx == SignalIOType::CUSTOM)
                core->guiSettings->setValue("idiag/custom","lastDir",dirname);
        }
        else if(idx == SignalIOType::CSV)
        {
            datDir = core->generalSettings->dataDirectory();
            QString fname = QFileDialog::getOpenFileName(this,"import .csv",datDir,".csv (*.csv)");

            // get directory the file is located in
            QString dirname = fname;
            while(!dirname.endsWith('/') && !dirname.isEmpty())
            {
                dirname.remove(dirname.size()-1,1);
            }

            // set default directory
            core->guiSettings->setValue("idiag/csv","lastDir",dirname);

            if(fname.isEmpty())return;
            lb->setText(fname);
        }

    }
}


/**
 * @brief ImportDialog::getLabelFromButtonName
 * @param bname
 * @return
 */
QLabel* ImportDialog::getLabelFromButtonName(QString bname)
{
    for(int i = 0; i< buttons.size();i++)
    {
        if(buttons.at(i)->objectName() == bname)
            return labels.at(i);
    }
    return NULL;
}


/**
 * @brief getLastDirectory retrieves last used directory for specific import method
 * @param idx
 * @return
 */
QString ImportDialog::getLastDirectory(int idx)
{
   QString lastDir = "";

   if(idx == SignalIOType::CUSTOM)
   {
       lastDir = core->guiSettings->value("idiag/custom","lastDir").toString();
   }
   else if(idx == SignalIOType::CSV_SCAN)
   {
      lastDir = core->guiSettings->value("idiag/csv","lastDir").toString();
   }

   if(lastDir.isEmpty())
       lastDir = core->generalSettings->dataDirectory();

    return lastDir;
}


/**
 * @brief ImportDialog::onOpenFile opens dir or file with external application
 */
void ImportDialog::onOpenFile()
{
    QUrl url;
    QString lastDir;

    //int idx = importSelector->getCurrentIndex();
    int idx = currentImportType;

    if(idx == SignalIOType::CUSTOM)
    {
        lastDir = core->guiSettings->value("idiag/custom","lastDir").toString();
        if(lastDir.isEmpty())
            lastDir = core->generalSettings->dataDirectory();
        url = QUrl::fromLocalFile(lastDir);
    }
    else if(idx == SignalIOType::CSV_SCAN)
    {
        lastDir = core->guiSettings->value("idiag/csv","lastDir").toString();
        if(lastDir.isEmpty())
            lastDir = core->generalSettings->dataDirectory();
        url = QUrl::fromLocalFile(lastDir);
    }
    else if(idx == SignalIOType::CSV)
    {
        // dummy for future usage
        QObject *sender = QObject::sender();
        QLabel* lb = getLabelFromButtonName(sender->objectName());

        if(lb != NULL)
        {
        }
        return;
    }
    else
    {
        return;
    }

    // opens file/directory with external application
    QDesktopServices::openUrl(url);
}


/**
 * @brief ImportDialog::onCheckImportRequestResults
 * @param results
 * Called by the import routine with the results of the directory scan.
 */
void ImportDialog::onCheckImportRequestResults(QList<SignalIORequest> results)
{
    buttonCheckFiles->setText("Scan files");
    buttonCheckFiles->setEnabled(true);
    widgetGeneralSettings->setEnabled(true);
    importTabs->setEnabled(true);

    for(int i = 0; i < results.size(); i++)
    {
        SignalImportInfoElement *siie = new SignalImportInfoElement;
        siie->setRequest(results.at(i), currentLIISettings());
        listSIIE.push_back(siie);
        layoutImportInfo->addWidget(siie);
    }

    checkImportRequestSuccessful = !results.isEmpty();
    butOk->setEnabled(checkImportRequestSuccessful);

    labelNoFilesFound->setVisible(!checkImportRequestSuccessful);

    selectorButtonsWidget->setVisible(true);
    groupboxImportInfo->setTitle("Measurement runs available for import");
}


/**
 * @brief ImportDialog::onIOImportSuccess
 * @param source
 * @param fileList
 * Called by the import routine (through SignalManager) after the files in @param fileList
 * have been loaded. Checks if all files of a run (info element) have been loaded and sets
 * the state of the run to "Loaded".
 */
void ImportDialog::onIOImportSuccess(SignalIORequest source, SignalFileInfoList fileList)
{
    for(int i = 0; i < listSIIE.size(); i++)
    {
        if(listSIIE.at(i)->request.runname.contains(source.runname) || (source.userData.contains(31)
                    && listSIIE.at(i)->request.userData.value(31).toString().contains(source.runname)))
        {
            for(int fli = 0; fli < fileList.size(); fli++)
            {
                for(int fie = 0; fie < listSIIE.at(i)->listFileElements.size(); fie++)
                {
                    if(fileList.at(fli).filename == listSIIE.at(i)->listFileElements.at(fie)->file.filename)
                    {
                        listSIIE.at(i)->listFileElements.at(fie)->setLoaded(true);
                    }
                }
            }
            bool allLoaded = true;
            for(int fie = 0; fie < listSIIE.at(i)->listFileElements.size(); fie++)
            {
                if(!listSIIE.at(i)->listFileElements.at(fie)->loaded)
                    allLoaded = false;
            }
            if(allLoaded)
            {
                listSIIE.at(i)->setImported(true);
            }
        }
    }

    bool allFinished = true;
    bool allLoaded = true;
    for(int i = 0; i < listSIIE.size(); i++)
    {
        if(!listSIIE.at(i)->imported)
            allLoaded = false;
        if(!listSIIE.at(i)->imported && !listSIIE.at(i)->error)
            allFinished = false;
    }

    if(allFinished)
    {
        widgetGeneralSettings->setEnabled(true);
        importTabs->setEnabled(true);

        butCancel->setEnabled(true);
    }

    if(allLoaded)
        close();
}


/**
 * @brief ImportDialog::onIOImportError
 * @param source
 * @param file
 * @param error Error message to display
 * Called by the import routine (through SignalManager) when an error occures. Searches the list
 * of requests/info elements for the element with the same runname - and if found for the same
 * filename and sets the error.
 */
void ImportDialog::onIOImportError(SignalIORequest source, SignalFileInfo file, QString error)
{
    for(int i = 0; i < listSIIE.size(); i++)
    {
        if(listSIIE.at(i)->request.runname.contains(source.runname))
        {
            for(int fie = 0; fie < listSIIE.at(i)->listFileElements.size(); fie++)
            {
                if(file.filename == listSIIE.at(i)->listFileElements.at(fie)->file.filename)
                {
                    listSIIE.at(i)->listFileElements.at(fie)->setError(error);
                    listSIIE.at(i)->addError(error);
                    listSIIE.at(i)->setImportError();
                    listSIIE.at(i)->showDetails();
                }
            }
        }
    }
}


/**
 * @brief ImportDialog::onCurrentImportTabChanged Connected to the 'currentChanged'-slot from the
 * QTabWidget. Sets the right import type, help text and clears the found mruns.
 *
 * !!! Needs to be edited if the tab order is changed or a new tab is introduced!!!
 *
 * @param current Current tab index
 */
void ImportDialog::onCurrentImportTabChanged(int current)
{
    checkImportRequestSuccessful = false;

    while(!listSIIE.isEmpty())
    {
        layoutImportInfo->removeWidget(listSIIE.last());
        delete listSIIE.last();
        listSIIE.removeLast();
    }

    //if(current == 0)
    //    currentImportType = SignalIOType::CSV;
    if(current == 0)
        currentImportType = SignalIOType::CSV_SCAN;
    else if(current == 1)
        currentImportType = SignalIOType::CUSTOM;

    QString lastDir = "";

    if(currentImportType == SignalIOType::CSV)
        helptext->setHtml(HelpManager::getHelpHTML("importCSV"));
    else if(currentImportType == SignalIOType::CSV_SCAN)
    {
        helptext->setHtml(HelpManager::getHelpHTML("importCSVauto"));
        lastDir = core->guiSettings->value("idiag/csv","lastDir").toString();
        if(lastDir.isEmpty())
            lastDir = core->generalSettings->dataDirectory();
        lbCsvAutoFname->setText(lastDir);
    }
    else if(currentImportType == SignalIOType::MAT)
        helptext->setHtml(HelpManager::getHelpHTML("importMAT"));
    else if(currentImportType == SignalIOType::CUSTOM)
    {
        helptext->setHtml(HelpManager::getHelpHTML("importCustom"));
        lastDir = core->guiSettings->value("idiag/custom","lastDir").toString();
        if(lastDir.isEmpty())
            lastDir = core->generalSettings->dataDirectory();
        lbCustomImportDirname->setText(lastDir);
    }

    labelNoFilesFound->setVisible(!checkImportRequestSuccessful);
    butOk->setEnabled(checkImportRequestSuccessful);

    onGuiStateChanged();
}


void ImportDialog::onImportGroupChanged(int idx)
{
    importGroupID = idxToGroupID.value(idx);
}
