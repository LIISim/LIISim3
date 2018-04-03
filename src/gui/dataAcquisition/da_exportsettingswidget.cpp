#include "da_exportsettingswidget.h"

#include "../../core.h"
#include <QHeaderView>
#include <QFileDialog>

/**
 * @brief DA_ExportSettingsWidget::DA_ExportSettingsWidget Constructor
 * @param parent Parent Widget
 */
DA_ExportSettingsWidget::DA_ExportSettingsWidget(QWidget *parent) : QTableWidget(parent)
{

    // ------------------
    // setup QTableWidget
    // ------------------

    int row = 0;
    setRowCount(6);
    setColumnCount(2);
    setShowGrid(false);
    for(int i = 0; i < rowCount(); i++)
        setRowHeight(i,17);
    setColumnWidth(0,200);
    setColumnWidth(1,400);
    setMinimumHeight(150);
    verticalHeader()->setVisible(false);
    QStringList mt_horizontalHeaders;
    mt_horizontalHeaders << "Export Settings" << " ";
    setHorizontalHeaderLabels(mt_horizontalHeaders);

    // ----------------------
    // row 0: autosave checkbox
    // ----------------------

    checkBoxAutoSave = new QCheckBox("Automatically save signal data");

    setSpan(row,0,1,2);
    setCellWidget(row,0,checkBoxAutoSave);

    row++;

    // ----------------------
    // row 1: auto refresh file name
    // ----------------------

    setSpan(row,0,1,2);
    checkBoxAutoRefreshFileName = new QCheckBox("Automatically refresh file name");
    setCellWidget(row,0,checkBoxAutoRefreshFileName);

    row++;

    // ----------------------
    // row 2: standard deviation
    // ----------------------

    //setSpan(row, 0, 1, 2);
    checkboxSaveStdev = new QCheckBox("Save standard deviation");
    setCellWidget(row, 0, checkboxSaveStdev);

    row++;

    // ------------------
    // row 3: runname
    // ------------------

    setItem(row,0,new QTableWidgetItem("Runname"));
    item(row,0)->setFlags(Qt::ItemIsEnabled);
    item(row,0)->setIcon(QIcon(Core::rootDir + "resources/icons/arrow_refresh.png"));

    setItem(row,1,new QTableWidgetItem("name"));
    item(row,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

    row++;

    // -----------------------
    // row 4: export directory
    // -----------------------

    setItem(row,0,new QTableWidgetItem("Save to directory"));
    item(row,0)->setFlags(Qt::ItemIsEnabled);

    setItem(row,1,new QTableWidgetItem("  "));
    item(row,1)->setFlags(Qt::ItemIsEnabled);

    row++;

    // -----------------------
    // row 5: export type
    // -----------------------

    setItem(row,0,new QTableWidgetItem("Output format"));
    item(row,0)->setFlags(Qt::ItemIsEnabled);

    cbExpType = new QComboBox;
    cbExpType->addItem("csv");
    cbExpType->addItem("matlab");
    setCellWidget(row,1,cbExpType);

    row++;

    // -----------
    // CONNECTIONS
    // -----------

    connect(Core::instance()->guiSettings,
            SIGNAL(settingsChanged()),
            SLOT(onGuiSettingsChanged()));

    connect(this,
            SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            SLOT(onItemDoubleClicked(QTableWidgetItem*)));

    connect(this,
            SIGNAL(cellClicked(int,int)),
            SLOT(refreshRunName(int,int)));

    connect(cbExpType,
            SIGNAL(currentTextChanged(QString)),
            SLOT(onExportTypeSelectionChanged(QString)));

    connect(checkBoxAutoSave,
            SIGNAL(stateChanged(int)),
            SLOT(onAutoSaveStateChanged(int)));

    connect(checkBoxAutoRefreshFileName,
            SIGNAL(stateChanged(int)),
            SLOT(onAutoRefreshStateChanged(int)));

    generateRunName();
    onAutoSaveStateChanged(0);
    onAutoRefreshStateChanged(0);
}


/**
 * @brief DA_ExportSettingsWidget::~DA_ExportSettingsWidget Destructor
 */
DA_ExportSettingsWidget::~DA_ExportSettingsWidget()
{

}


/**
 * @brief DA_ExportSettingsWidget::generateExportRequest generates
 * informations which are needed to save and load the mrun data
 * @param run
 * @return SignalIORequest for signal export
 */
SignalIORequest DA_ExportSettingsWidget::generateExportRequest(MRun *run, bool stdev)
{

    // --------------------------
    // generate export request
    // --------------------------

    SignalIORequest rq;

    rq.itype = exportType();
    rq.datadir = exportDirectory();

    QList<QVariant> run_ids = QList<QVariant>() << run->id();

    rq.userData.insert(8,run_ids);

    // set export flags
    rq.userData.insert(13, true); // [true] save raw data
    rq.userData.insert(14, false); // [false] don't save absolute data
    rq.userData.insert(15, false); // [false] don't save temperature data
    rq.userData.insert(16, false); // raw data [false] pre- or [true] postprocessed
    rq.userData.insert(17, false); // absolute data [false] pre- or [true] postprocessed
    rq.userData.insert(28, stdev); // [true] save standard deviation data

    // --------------------------
    // generate import request
    // --------------------------

    // create information which is needed to import the run
    //(e.g. on next program startup)

    if(exportType() == CSV)
    {
        SignalIORequest irq;
        irq.itype = CSV;

        irq.userData.insert(3,run->liiSettings().filename);
        irq.userData.insert(18, true); // [true] copy raw data to absolute data during import
        irq.noChannels = run->liiSettings().channels.size();
        irq.runsettings_dirpath = exportDirectory();

        for(int i=0; i < irq.noChannels; i++)
        {
            SignalFileInfo fi;
            fi.itype = CSV;
            fi.filename = QString("%0%1.Signal.Volts.Ch%2.csv")
                    .arg(exportDirectory())
                    .arg(run->getName())
                    .arg(i+1);
            fi.channelId = i+1;
            fi.signalType = Signal::RAW;
            irq.flist << fi;
        }
        if(stdev)
        {
            for(int i = 0; i < irq.noChannels; i++)
            {
                SignalFileInfo fi;
                fi.itype = CSV;
                fi.filename = QString("%0%1.StandardDeviation.Raw.Ch%2.csv").arg(exportDirectory()).arg(run->getName()).arg(i+1);
                fi.channelId = i+1;
                fi.signalType = Signal::RAW;
                fi.stdevFile = true;
                irq.flist.push_back(fi);
            }
        }

        run->setImportRequest(irq);
    }
    else if(exportType() == MAT)
    {
        // !!! no import implemented !!!
    }

    return rq;
}


/**
 * @brief DA_ExportSettingsWidget::exportType Retruns the selected export type
 * @return
 */
SignalIOType DA_ExportSettingsWidget::exportType()
{
    if(cbExpType->currentIndex() == 0)
        return CSV;
    else
        return MAT;
}


/**
 * @brief DA_ExportSettingsWidget::runName Returns runname
 * @return
 */
QString DA_ExportSettingsWidget::runName()
{
    return item(3,1)->text();
}


/**
 * @brief DA_ExportSettingsWidget::exportDirectory Returns export directory
 * @return
 */
QString DA_ExportSettingsWidget::exportDirectory()
{
    QString dir = item(4,1)->text();
    if(!dir.endsWith('/'))
        dir.append('/');
    return dir;
}


/**
 * @brief DA_ExportSettingsWidget::generateRunName generates a
 * new measurement run name based on current date/time
 */
void DA_ExportSettingsWidget::generateRunName()
{
    //QString runname = QDateTime::currentDateTime().toString("ddd-MMM-yyyy_hh-mm-ss");
    QString runname = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    item(3,1)->setText(runname);
}


/**
 * @brief DA_ExportSettingsWidget::autoSave returns the checked state
 * of the Autosave Checkbox
 * @return
 */
bool DA_ExportSettingsWidget::autoSave()
{
    return checkBoxAutoSave->isChecked();
}

/**
 * @brief DA_ExportSettingsWidget::autoRefresh checks if auto refresh is checked, if yes refreshes filename
 */
void DA_ExportSettingsWidget::autoRefresh()
{
    if(checkBoxAutoRefreshFileName->isChecked())
        generateRunName();
}


bool DA_ExportSettingsWidget::saveStdev()
{
    return checkboxSaveStdev->isChecked();
}

// --------------------
// PRIVATE SLOTS
// --------------------


/**
 * @brief DA_ExportSettingsWidget::onItemDoubleClicked
 * @param item
 */
void DA_ExportSettingsWidget::onItemDoubleClicked(QTableWidgetItem *i)
{
    // export directory item
    if(i == item(4,1))
    {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Export Directory"),
                                                    item(4,1)->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty())
            return;

        item(4,1)->setText(dir);
        item(4,1)->setToolTip(dir);
        Core::instance()->guiSettings->setValue("da","expdir",dir);
    }
}


void DA_ExportSettingsWidget::onExportTypeSelectionChanged(const QString &text)
{
    Core::instance()->guiSettings->setValue("da","exptype",text);
}



/**
 * @brief DA_ExportSettingsWidget::onGuiSettingsChanged This
 * slot is executed when the global GUISettings (see Core) were
 * modified. Updates values and states of UI Elements
 */
void DA_ExportSettingsWidget::onGuiSettingsChanged()
{
    GuiSettings* g = Core::instance()->guiSettings;

    // export directory
    QString dir = g->value("da","expdir",Core::rootDir + "LIISim3/data/").toString();
    item(4,1)->setText(dir);
    item(4,1)->setToolTip(dir);

    QString type = g->value("da","exptype","matlab").toString();
    cbExpType->setCurrentText(type);

    checkBoxAutoSave->setChecked(g->value("da","autosave",false).toBool());
    checkBoxAutoRefreshFileName->setChecked(g->value("da","autorefresh",false).toBool());

}


/**
 * @brief DA_ExportSettingsWidget::onAutoSaveStateChanged This slot
 * is executed when the checked state of the autosave Checkbox has
 * been changed
 * @param state new state
 */
void DA_ExportSettingsWidget::onAutoSaveStateChanged(int state)
{
    Core::instance()->guiSettings->setValue("da","autosave",checkBoxAutoSave->isChecked());

    // update item flags

    cbExpType->setEnabled(state);
    checkBoxAutoRefreshFileName->setEnabled(state);
    checkboxSaveStdev->setEnabled(state);

    if(checkBoxAutoSave->isChecked())
    {
        item(3,0)->setFlags(Qt::ItemIsEnabled);
        item(3,1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

        item(4,0)->setFlags(Qt::ItemIsEnabled);
        item(4,1)->setFlags(Qt::ItemIsEnabled);

        item(5,0)->setFlags(Qt::ItemIsEnabled);
    }
    else
    {
        item(3,0)->setFlags(0);
        item(3,1)->setFlags(0);

        item(4,0)->setFlags(0);
        item(4,1)->setFlags(0);

        item(5,0)->setFlags(0);
    }
}

/**
 * @brief DA_ExportSettingsWidget::onAutoRefreshStateChangedThis slot
 * is executed when the checked state of the autorefresh Checkbox has
 * been changed
 * @param state new state
 */
void DA_ExportSettingsWidget::onAutoRefreshStateChanged(int state)
{
    Core::instance()->guiSettings->setValue("da","autorefresh",checkBoxAutoRefreshFileName->isChecked());
}


void DA_ExportSettingsWidget::refreshRunName(int r,int c)
{
    if(r == 3 && c == 0)
        generateRunName();
}
