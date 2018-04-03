#include "sessionsavedialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../../core.h"



SessionSaveDialog::SessionSaveDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    resize(600,400);
    setWindowTitle("Save .xml Session");
    setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // INIT GUI WIDGETS

    QVBoxLayout* mainLayout= new QVBoxLayout(this);

    QHBoxLayout* topLayout = new QHBoxLayout;
    mainLayout->addLayout(topLayout);

    labelInfo = new QLabel("Output .xml Session file");
    topLayout->addWidget(labelInfo);

    lineEditFilename = new QLineEdit;
    topLayout->addWidget(lineEditFilename);

    buttonSelectFile = new QPushButton("Select");
    connect(buttonSelectFile,SIGNAL(released()),SLOT(onButtonSelectReleased()));
    topLayout->addWidget(buttonSelectFile);


    QHBoxLayout* midLayout = new QHBoxLayout;
    mainLayout->addLayout(midLayout);

    dataTree = new DataItemTreeView(DataItemTreeView::EXP_DIAG, Core::instance()->dataModel()->rootItem());
    dataTree->setMaximumWidth(300);
    dataTree->setHeaderLabel("Save selected Runs");

    QList<int> allRunIds;
    QList<MRun*> allRuns = Core::instance()->dataModel()->mrunList();
    for(int i=0; i<allRuns.size(); i++)
        allRunIds << allRuns[i]->id();
    dataTree->setCheckedRunIDs(allRunIds);

    midLayout->addWidget(dataTree);


    QVBoxLayout* optionsLayout = new QVBoxLayout;
    midLayout->addLayout(optionsLayout);

    QWidget *spacer0 = new QWidget(this);
    spacer0->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    optionsLayout->addWidget(spacer0);

    checkboxRelative  = new QCheckBox();
    checkboxRelative->setText("save file paths relative to xml-file location");
    optionsLayout->addWidget(checkboxRelative);

    QWidget *spacer1 = new QWidget(this);
    spacer1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    midLayout->addWidget(spacer1);

    QHBoxLayout* bottomLayout = new QHBoxLayout;
    mainLayout->addLayout(bottomLayout);

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    bottomLayout->addWidget(spacer);

    buttonCancel = new QPushButton("Cancel");
    connect(buttonCancel,SIGNAL(released()),SLOT(onButtonCancelReleased()));
    bottomLayout->addWidget(buttonCancel);

    buttonSave = new QPushButton("Save");
    connect(buttonSave,SIGNAL(released()),SLOT(onButtonSaveReleased()));
    bottomLayout->addWidget(buttonSave);


    // INIT GUISETTINGS
    GuiSettings* gs = Core::instance()->guiSettings;

    lineEditFilename->setText(gs->value("sessionDiag","lastfname","../").toString());
    checkboxRelative->setChecked(gs->value("sessionDiag","relPath").toBool());

    fileSelected = false;
}


/**
 * @brief SessionSaveDialog::done overwrites QDialog::done(int)
 * This slot is executed when the dialog has been accepted/rejected/..
 * by the user
 * @param r result parameter, see QDialog::DialogCode
 */
void SessionSaveDialog::done(int r)
{

    QString fname = lineEditFilename->text();
    if(r == QDialog::Accepted)
    {
        if(fname.isEmpty())
        {
            QString msg = "Cannot save .xml session: empty file name!";
            MSG_ERR(msg);
            QMessageBox::warning(0,"Error",msg);
            return;
        }

        // check if file exists already,
        // skip check if user already approved overwriting of file!
        if(QFile(fname).exists() && !fileSelected)
        {
            QString title = "File already exists";
            QString text = QString("Do you want to overwrite this "
                                   "file:<br><br><i>'%0'</i>")
                           .arg(fname);

            QMessageBox msgBox(QMessageBox::Warning,
                               title,text,QMessageBox::Yes|QMessageBox::No );
            msgBox.setModal(true);
            msgBox.setDefaultButton(QMessageBox::No);
            if(msgBox.exec() == QMessageBox::No)
            {
                QDialog::done(r);
                return;
            }
        }

        SignalIORequest rq;
        rq.itype = XML;
        rq.userData.insert(0,fname);
        rq.userData.insert(9,false);
        rq.userData.insert(10,false);
        rq.userData.insert(11,false);
        rq.userData.insert(12,true);

        // infos about checked run ids
        rq.userData.insert(8, getCheckedRunIDs());

        // store data paths relative to xml file location
        rq.userData.insert(2,checkboxRelative->isChecked());

        // send request to signal manager
        Core::instance()->getSignalManager()->exportSignalsManager( rq);

    }

    GuiSettings* gs = Core::instance()->guiSettings;
    gs->setValue("sessionDiag","lastfname",fname);
    gs->setValue("sessionDiag","relPath",checkboxRelative->isChecked());

    QDialog::done(r);
}

// -------------
// UI CALLBACKS
// -------------


/**
 * @brief SessionSaveDialog::onButtonSelectReleased This slot is
 * executed when the 'Select' Button has been released. Opens
 * a native file dialog which allows to specify an output file for
 * the session export.
 */
void SessionSaveDialog::onButtonSelectReleased()
{
    // get the path of the last file used from gui settings
    QString currentFname = lineEditFilename->text();
    if(currentFname.isEmpty())
        currentFname = Core::rootDir;

    QString fname = QFileDialog::getSaveFileName(QApplication::focusWidget(),
                                         "Save .xml Session",
                                         currentFname,".xml (*.xml)");

    if(!fname.isEmpty())
    {
        lineEditFilename->setText(fname);
        fileSelected = true;
    }
}


/**
 * @brief SessionSaveDialog::onButtonCancelReleased This slot is executed
 * when the 'Cancel' Button has been released
 */
void SessionSaveDialog::onButtonCancelReleased()
{
    done(QDialog::Rejected); // reject dialog
}

/**
 * @brief SessionSaveDialog::onButtonSaveReleased This slot is executed
 * when the 'Save' Button has been released
 */
void SessionSaveDialog::onButtonSaveReleased()
{
    done(QDialog::Accepted); // accept dialog
}

// ---------------
// PRIVATE HELPERS
// ---------------

QList<QVariant> SessionSaveDialog::getCheckedRunIDs()
{
    QList<QVariant> checkedRunIds;
    for(int i = 0; i < dataTree->topLevelItemCount();i++)
    {
        QTreeWidgetItem* gi = dataTree->topLevelItem(i);
        for(int j = 0; j < gi->childCount(); j++)
            if(gi->child(j)->checkState(0) == Qt::Checked)
                checkedRunIds << gi->child(j)->data(0,Qt::UserRole+1);

    }
    return checkedRunIds;
}








