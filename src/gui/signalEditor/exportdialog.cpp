#include "exportdialog.h"

#include <QFileDialog>
#include <QButtonGroup>

#include "../../core.h"

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------

ExportDialog::ExportDialog(QWidget *parent) :  QDialog(parent)
{
    setWindowTitle(tr("Export Signals"));
    setModal(true);
    setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    resize(600,500);

    GuiSettings* gs = Core::instance()->guiSettings;

    lay_main_v = new QVBoxLayout;
    lay_main_v->setSizeConstraint(QLayout::SetMinAndMaxSize);
    setLayout(lay_main_v);

    exportSelector = new LabeledComboBox(tr("Output Type: "));
    exportSelector->setMinimumWidth(200);
    exportSelector->setMaximumWidth(250);

    checkboxMatCompression = new QCheckBox("Enable compression");
    checkboxMatCompression->setChecked(gs->value("ediag/mat", "mat_compression", "false").toBool());

    QHBoxLayout *layoutExportType = new QHBoxLayout;
    layoutExportType->setMargin(0);

    layoutExportType->addWidget(exportSelector);
    layoutExportType->addWidget(checkboxMatCompression);

    lay_main_v->addLayout(layoutExportType);
    lay_main_v->setAlignment(layoutExportType, Qt::AlignLeft);

    // output directory
    layDirSelect = new QHBoxLayout;
    layDirSelect->setSizeConstraint(QLayout::SetMinimumSize);

    lbDirDescr = new QLabel("Output Directory: ");
    layDirSelect->addWidget(lbDirDescr);

    QString datDir;

    if(gs->hasEntry("idiag/csv","exportDir"))
        datDir = gs->value("idiag/csv","exportDir").toString();
    else
        datDir = gs->value("idiag/csv","lastDir").toString();

    //qDebug() << "exportDialog: data dir: " << datDir;

    lbDir = new QLabel(datDir);
    layDirSelect->addWidget(lbDir,0,Qt::AlignLeft);

    butDir = new QPushButton("Set");
    butDir->setMaximumWidth(50);
    connect(butDir,SIGNAL(released()),this,SLOT(onSelectDir()));
    layDirSelect->addWidget(butDir);
    lay_main_v->addLayout(layDirSelect);


    // MRun select tree
    QHBoxLayout* runAndDataLayout = new QHBoxLayout;
    //runAndDataLayout->setMargin(0);
    lay_main_v->addLayout(runAndDataLayout);

    runTree = new DataItemTreeView(DataItemTreeView::EXP_DIAG, Core::instance()->dataModel()->rootItem());
    runTree->setMaximumWidth(350);
    runTree->setHeaderLabel("Export selected Runs:");
    runAndDataLayout->addWidget(runTree);

    // ------------------
    // Data Selection Box
    // ------------------

    boxDataSel = new QGroupBox("Data Selection");
    runAndDataLayout->addWidget(boxDataSel);

    QVBoxLayout* boxDataSelLayout= new QVBoxLayout;
    boxDataSel->setLayout(boxDataSelLayout);


    dscb_raw = new QCheckBox("Save raw signal data");
    dscb_raw->setChecked( gs->value("ediag/datasel","raw_checked_state","true").toBool());
    boxDataSelLayout->addWidget(dscb_raw);

    QVBoxLayout* dsrb_raw_layout = new QVBoxLayout;
    dsrb_raw_layout->setContentsMargins(15,2,2,2);
    boxDataSelLayout->addLayout(dsrb_raw_layout);

    dsrb_raw_pre = new QRadioButton("unprocessed signal data");
    dsrb_raw_pre->setChecked( ! gs->value("ediag/datasel","raw_post_proc","true").toBool());
    dsrb_raw_pre->setEnabled(dscb_raw->isChecked());
    dsrb_raw_layout->addWidget(dsrb_raw_pre);

    dsrb_raw_post = new QRadioButton("processed signal data");
    dsrb_raw_post->setChecked( gs->value("ediag/datasel","raw_post_proc","true").toBool());
    dsrb_raw_post->setEnabled(dscb_raw->isChecked());
    dsrb_raw_layout->addWidget(dsrb_raw_post);

    QButtonGroup* ds_raw_group = new QButtonGroup(this);
    ds_raw_group->addButton(dsrb_raw_pre);
    ds_raw_group->addButton(dsrb_raw_post);

    checkboxRawUnprocessed = new QCheckBox("unprocessed signal data");
    checkboxRawUnprocessed->setChecked(gs->value("ediag/datasel", "raw_unprocessed_state", "true").toBool());
    checkboxRawUnprocessed->setEnabled(dscb_raw->isChecked());
    dsrb_raw_layout->addWidget(checkboxRawUnprocessed);

    checkboxRawProcessed = new QCheckBox("processed signal data");
    checkboxRawProcessed->setChecked(gs->value("ediag/datasel", "raw_processed_state", "true").toBool());
    checkboxRawProcessed->setEnabled(dscb_raw->isChecked());
    dsrb_raw_layout->addWidget(checkboxRawProcessed);

    checkboxRawStdev = new QCheckBox("standard deviation (if available)");
    checkboxRawStdev->setChecked(gs->value("ediag/datasel", "raw_stdev_state", "true").toBool());
    checkboxRawStdev->setEnabled(dscb_raw->isChecked());
    dsrb_raw_layout->addWidget(checkboxRawStdev);


    dscb_abs = new QCheckBox("Save absolute signal data");
    dscb_abs->setChecked( gs->value("ediag/datasel","abs_checked_state","true").toBool());
    boxDataSelLayout->addWidget(dscb_abs);

    QVBoxLayout* dsrb_abs_layout = new QVBoxLayout;
    dsrb_abs_layout->setContentsMargins(15,2,2,2);
    boxDataSelLayout->addLayout(dsrb_abs_layout);

    dsrb_abs_pre = new QRadioButton("unprocessed signal data");
    dsrb_abs_pre->setChecked( ! gs->value("ediag/datasel","abs_post_proc","true").toBool());
    dsrb_abs_pre->setEnabled(dscb_abs->isChecked());
    dsrb_abs_layout->addWidget(dsrb_abs_pre);

    dsrb_abs_post = new QRadioButton("processed signal data");
    dsrb_abs_post->setChecked( gs->value("ediag/datasel","abs_post_proc","true").toBool());
    dsrb_abs_post->setEnabled(dscb_abs->isChecked());
    dsrb_abs_layout->addWidget(dsrb_abs_post);

    QButtonGroup* ds_abs_group = new QButtonGroup(this);
    ds_abs_group->addButton(dsrb_abs_pre);
    ds_abs_group->addButton(dsrb_abs_post);

    checkboxAbsUnprocessed = new QCheckBox("unprocessed signal data");
    checkboxAbsUnprocessed->setChecked(gs->value("ediag/datasel", "abs_unprocessed_state", "true").toBool());
    checkboxAbsUnprocessed->setEnabled(dscb_abs->isChecked());
    dsrb_abs_layout->addWidget(checkboxAbsUnprocessed);

    checkboxAbsProcessed = new QCheckBox("processed signal data");
    checkboxAbsProcessed->setChecked(gs->value("ediag/datasel", "abs_processed_state", "true").toBool());
    checkboxAbsProcessed->setEnabled(dscb_abs->isChecked());
    dsrb_abs_layout->addWidget(checkboxAbsProcessed);

    checkboxAbsStdev = new QCheckBox("standard deviation (if available)");
    checkboxAbsStdev->setChecked(gs->value("ediag/datasel", "abs_stdev_state", "true").toBool());
    checkboxAbsStdev->setEnabled(dscb_abs->isChecked());
    dsrb_abs_layout->addWidget(checkboxAbsStdev);


    dscb_tmp = new QCheckBox("Save temperature signal data");
    dscb_tmp->setChecked( gs->value("ediag/datasel","tmp_checked_state","true").toBool());
    boxDataSelLayout->addWidget(dscb_tmp);

    QVBoxLayout* dsrb_temp_layout = new QVBoxLayout;
    dsrb_temp_layout->setContentsMargins(15,2,2,2);
    boxDataSelLayout->addLayout(dsrb_temp_layout);

    checkboxTempUnprocessed = new QCheckBox("unprocessed signal data");
    checkboxTempUnprocessed->setChecked(gs->value("ediag/datasel", "temp_unprocessed_state", "true").toBool());
    checkboxTempUnprocessed->setEnabled(dscb_tmp->isChecked());
    dsrb_temp_layout->addWidget(checkboxTempUnprocessed);

    checkboxTempProcessed = new QCheckBox("processed signal data");
    checkboxTempProcessed->setChecked(gs->value("ediag/datasel", "temp_processed_state", "true").toBool());
    checkboxTempProcessed->setEnabled(dscb_tmp->isChecked());
    dsrb_temp_layout->addWidget(checkboxTempProcessed);

    checkboxTempStdev = new QCheckBox("standard deviation data");
    checkboxTempStdev->setChecked(gs->value("ediag/datasel", "temp_stdev_state", "true").toBool());
    checkboxTempStdev->setEnabled(dscb_tmp->isChecked());
    dsrb_temp_layout->addWidget(checkboxTempStdev);

    boxDataSelLayout->addStretch(-1);

    connect(dscb_raw,SIGNAL(toggled(bool)),SLOT(onDataSelCheckBoxToggled(bool)));
    connect(dscb_abs,SIGNAL(toggled(bool)),SLOT(onDataSelCheckBoxToggled(bool)));
    connect(dscb_tmp,SIGNAL(toggled(bool)),SLOT(onDataSelCheckBoxToggled(bool)));

    connect(checkboxRawUnprocessed, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxRawProcessed, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxRawStdev, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxAbsUnprocessed, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxAbsProcessed, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxAbsStdev, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxTempUnprocessed, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxTempProcessed, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));
    connect(checkboxTempStdev, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));

    connect(checkboxMatCompression, SIGNAL(toggled(bool)), SLOT(onDataSelCheckBoxToggled(bool)));

    connect(dsrb_raw_pre, SIGNAL(toggled(bool)),SLOT(onDataSelRadioButtonToggled(bool)));
    connect(dsrb_raw_post,SIGNAL(toggled(bool)),SLOT(onDataSelRadioButtonToggled(bool)));
    connect(dsrb_abs_pre, SIGNAL(toggled(bool)),SLOT(onDataSelRadioButtonToggled(bool)));
    connect(dsrb_abs_post,SIGNAL(toggled(bool)),SLOT(onDataSelRadioButtonToggled(bool)));

    // ----------------------
    // xml export, custom gui
    // ----------------------

    // buttons box
    lay_buts = new QHBoxLayout;
    lay_main_v->addLayout(lay_buts);

    butOk = new QPushButton(tr("Export"));
    butCancel = new QPushButton(tr("Cancel"));

    lay_buts->addStretch(-1);
    lay_buts->addWidget(butOk);
    lay_buts->addWidget(butCancel);
    connect(butOk,SIGNAL(released()),this,SLOT(onOk()));
    connect(butCancel,SIGNAL(released()),this,SLOT(onCancel()));


    // fill selectors
    QList<MRun*> mruns = Core::instance()->dataModel()->mrunList();

    int noMruns =  mruns.size();
    if(noMruns == 0) butOk->setEnabled(false);

    for(int i=0; i<noMruns; i++)
        try
        {
            indexToIdMap.insert(i,mruns[i]->id());            
        }
        catch(LIISimException e){
            qDebug() << e.what();
        }



    exportSelector->addStringItem("CSV");
    exportSelector->addStringItem("MAT");

    int index = Core::instance()->guiSettings->value("export_dialog", "export_type", 0).toInt();
    exportSelector->setCurrentIndex(index);
    onSelectionChanged(index);

    connect(exportSelector,SIGNAL(currentIndexChanged(int)),SLOT(onSelectionChanged(int)));
}


void ExportDialog::onSelectionChanged(int idx)
{
    Core::instance()->guiSettings->setValue("export_dialog", "export_type", idx);

    // default data directory (one could also create a "lastExportDirectory" settings entry...)
    QString datDir;
    if(Core::instance()->guiSettings->hasEntry("idiag/csv","exportDir"))
        datDir = Core::instance()->guiSettings->value("idiag/csv","exportDir").toString();
    else
        datDir = Core::instance()->guiSettings->value("idiag/csv","lastDir").toString();

    switch(idx)
    {
        // CSV
        case 0:
            runTree->setVisible(true);
            lbDirDescr->setText("Output Directory: ");
            lbDir->setText(datDir);

            checkboxAbsUnprocessed->setVisible(false);
            checkboxAbsProcessed->setVisible(false);

            checkboxRawUnprocessed->setVisible(false);
            checkboxRawProcessed->setVisible(false);

            checkboxTempUnprocessed->setVisible(false);
            checkboxTempProcessed->setVisible(false);
            checkboxTempStdev->setVisible(false);

            checkboxMatCompression->setVisible(false);

            dsrb_raw_pre->setVisible(true);
            dsrb_raw_post->setVisible(true);
            dsrb_abs_pre->setVisible(true);
            dsrb_abs_post->setVisible(true);
        break;

        // MAT
        case 1:
            runTree->setVisible(true);
            lbDirDescr->setText("Output Directory: ");
            lbDir->setText(datDir);

            dsrb_raw_pre->setVisible(false);
            dsrb_raw_post->setVisible(false);
            dsrb_abs_pre->setVisible(false);
            dsrb_abs_post->setVisible(false);

            checkboxAbsUnprocessed->setVisible(true);
            checkboxAbsProcessed->setVisible(true);

            checkboxRawUnprocessed->setVisible(true);
            checkboxRawProcessed->setVisible(true);

            checkboxTempUnprocessed->setVisible(true);
            checkboxTempProcessed->setVisible(true);
            checkboxTempStdev->setVisible(true);

            checkboxMatCompression->setVisible(true);
        break;

        default:
            runTree->setVisible(false);
            lbDirDescr->setText("Output Directory: ");
            lbDir->setText(datDir);
        break;
    }
}


void ExportDialog::onSelectDir()
{
    // for xml export, we select a file instead of adirectory!
    if(exportSelector->getCurrentIndex() == 2)
    {

        QString fname = QFileDialog::getSaveFileName(this,"Set output .xml filename",lbDir->text());
        if(fname.isEmpty())return;
        if(!fname.endsWith(".xml"))
            fname.append(".xml");
        lbDir->setText(fname);
        Core::instance()->guiSettings->setValue("ediag/xml","lastfname",fname);
    }
    else
    {
        QString dirname = QFileDialog::getExistingDirectory(this,tr("Select Directory for Export"),lbDir->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
        if(dirname.isEmpty())return;
        if(!dirname.endsWith("/"))
            dirname.append("/");
        lbDir->setText(dirname);
        Core::instance()->guiSettings->setValue("idiag/csv","exportDir",dirname);
    }
}


/**
 * @brief ExportDialog::onOk This method is responsible for creating a SignalIORequest
 * which will be handled by the signal manager.
 */
void ExportDialog::onOk()
{    
    int etype = exportSelector->getCurrentIndex();

    SignalIORequest erq;

    erq.datadir = lbDir->text();

    QList<QVariant> checkedRunIds;
    for(int i = 0; i < runTree->topLevelItemCount();i++)
    {
        QTreeWidgetItem* gi = runTree->topLevelItem(i);
        for(int j = 0; j < gi->childCount(); j++)
        {
            if(gi->child(j)->checkState(0) == Qt::Checked)
                checkedRunIds << gi->child(j)->data(0,Qt::UserRole+1);
        }
    }

    erq.userData.insert(8, checkedRunIds);

    switch (etype)
    {
        case 0:
            erq.itype = CSV;
            erq.userData.insert(0, "multiple files");
            // set export flags
            erq.userData.insert(13, dscb_raw->isChecked()); // save raw data
            erq.userData.insert(14, dscb_abs->isChecked()); // save absolute data
            erq.userData.insert(15, dscb_tmp->isChecked()); // save temperature data
            erq.userData.insert(16, dsrb_raw_post->isChecked()); // raw data pre- or postprocessed
            erq.userData.insert(17, dsrb_abs_post->isChecked()); // absolute data pre- or postprocessed

            erq.userData.insert(28, checkboxRawStdev->isChecked()); //raw standard deviation
            erq.userData.insert(29, checkboxAbsStdev->isChecked()); //abs standard deviation

            break;
        case 1:
            erq.itype = MAT;
            // set export flags
            erq.userData.insert(13, checkboxRawUnprocessed->isChecked() && dscb_raw->isChecked());
            erq.userData.insert(14, checkboxAbsUnprocessed->isChecked() && dscb_abs->isChecked());
            erq.userData.insert(15, checkboxTempUnprocessed->isChecked() && dscb_tmp->isChecked());
            erq.userData.insert(16, checkboxRawProcessed->isChecked() && dscb_raw->isChecked());
            erq.userData.insert(17, checkboxAbsProcessed->isChecked() && dscb_abs->isChecked());

            erq.userData.insert(25, checkboxTempProcessed->isChecked() && dscb_tmp->isChecked());
            erq.userData.insert(26, checkboxTempStdev->isChecked() && dscb_tmp->isChecked());

            erq.userData.insert(28, checkboxRawStdev->isChecked() && dscb_raw->isChecked());
            erq.userData.insert(29, checkboxAbsStdev->isChecked() && dscb_abs->isChecked());

            erq.userData.insert(35, checkboxMatCompression->isChecked());

            break;

        default: break;
    }

    // do only save data (no settings)
    erq.userData.insert(9 ,false); // modeling settings
    erq.userData.insert(10,false); // gui settings
    erq.userData.insert(11,false); // general settings
    erq.userData.insert(12,true);  // data

    // send export request to SignalManager::handleActionExport()
    // SIGNAL: signalExportRequest(SignalIORequest erq)
    // SLOT: SignalManager::exportSignals(SignalIORequest rq)
    emit signalExportRequest(erq);
    close();
}


void ExportDialog::onCancel()
{
    emit signalCanceled();
    close();
}


/**
 * @brief ExportDialog::onDataSelCheckBoxToggled This slot is executed when
 * any of the data selcetion checkboxes has been toggled by the user
 * @param state new state parameter
 */
void ExportDialog::onDataSelCheckBoxToggled(bool state)
{
    QObject* s = QObject::sender();
    GuiSettings* gs = Core::instance()->guiSettings;

    if(s == dscb_raw)
    {
        dsrb_raw_pre->setEnabled(state);
        dsrb_raw_post->setEnabled(state);
        checkboxRawStdev->setEnabled(state);

        checkboxRawProcessed->setEnabled(state);
        checkboxRawUnprocessed->setEnabled(state);

        gs->setValue("ediag/datasel","raw_checked_state",state);
    }
    else if(s == dscb_abs)
    {
        dsrb_abs_pre->setEnabled(state);
        dsrb_abs_post->setEnabled(state);
        checkboxAbsStdev->setEnabled(state);

        checkboxAbsUnprocessed->setEnabled(state);
        checkboxAbsProcessed->setEnabled(state);

        gs->setValue("ediag/datasel","abs_checked_state",state);
    }
    else if(s == dscb_tmp)
    {
        gs->setValue("ediag/datasel","tmp_checked_state",state);

        checkboxTempUnprocessed->setEnabled(state);
        checkboxTempProcessed->setEnabled(state);
        checkboxTempStdev->setEnabled(state);
    }
    else if(s == checkboxRawUnprocessed)
        gs->setValue("ediag/datasel", "raw_unprocessed_state", state);
    else if(s == checkboxRawProcessed)
        gs->setValue("ediag/datasel", "raw_processed_state", state);
    else if(s == checkboxRawStdev)
        gs->setValue("ediag/datasel", "raw_stdev_state", state);
    else if(s == checkboxAbsUnprocessed)
        gs->setValue("ediag/datasel", "abs_unprocessed_state", state);
    else if(s == checkboxAbsProcessed)
        gs->setValue("ediag/datasel", "abs_processed_state", state);
    else if(s == checkboxAbsStdev)
        gs->setValue("ediag/datasel", "abs_stdev_state", state);
    else if(s == checkboxTempUnprocessed)
        gs->setValue("ediag/datasel", "temp_unprocessed_state", state);
    else if(s == checkboxTempProcessed)
        gs->setValue("ediag/datasel", "temp_processed_state", state);
    else if(s == checkboxTempStdev)
        gs->setValue("ediag/datasel", "temp_stdev_state", state);
    else if(s == checkboxMatCompression)
        gs->setValue("ediag/mat", "mat_compression", state);
}


/**
 * @brief ExportDialog::onDataSelRadioButtonToggled this slot is executed when
 * a data selection radio button has been toggled
 * @param state new state parameter
 */
void ExportDialog::onDataSelRadioButtonToggled(bool state)
{
    QObject* s = QObject::sender();
    GuiSettings* gs = Core::instance()->guiSettings;

    if(s == dsrb_raw_pre && state )
    {
        gs->setValue("ediag/datasel","raw_post_proc",false);
    }
    else if(s == dsrb_raw_post && state )
    {
        gs->setValue("ediag/datasel","raw_post_proc",true);
    }
    else if(s == dsrb_abs_pre && state )
    {
        gs->setValue("ediag/datasel","abs_post_proc",false);
    }
    else if(s == dsrb_abs_post && state )
    {
        gs->setValue("ediag/datasel","abs_post_proc",true);
    }
}


