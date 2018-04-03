#include "sessionloaddialog.h"

#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>

#include "../../core.h"

SessionLoadDialog::SessionLoadDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    resize(600,370);
    setWindowTitle("Load .xml Session");
    setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // INIT GUI WIDGETS

    QVBoxLayout* mainLayout= new QVBoxLayout(this);

    QHBoxLayout* topLayout = new QHBoxLayout;
    mainLayout->addLayout(topLayout);

    labelInfo = new QLabel("Load .xml Session file:");
    topLayout->addWidget(labelInfo);

    labelFname = new QLabel();
    topLayout->addWidget(labelFname);

    QWidget *spacer0 = new QWidget(this);
    spacer0->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    topLayout->addWidget(spacer0);

    buttonSelectFile = new QPushButton("Select");
    connect(buttonSelectFile,SIGNAL(released()),SLOT(onButtonSelectReleased()));
    topLayout->addWidget(buttonSelectFile);


    QHBoxLayout* midLayout = new QHBoxLayout;
    mainLayout->addLayout(midLayout);



    QVBoxLayout* optionsLayout = new QVBoxLayout;
    midLayout->addLayout(optionsLayout);
    optionsLayout->addWidget(new QLabel("What should happen to existing data?"));

    radioButtons = new QButtonGroup;
    radioButtons->setExclusive(true);
    int maxOptWidth = 300;

    radioButtonClear = new QRadioButton("Clear current session");
    radioButtonClear->setMaximumWidth(maxOptWidth);
    radioButtons->addButton(radioButtonClear);
    optionsLayout->addWidget(radioButtonClear);

    QLabel* labelClearInfo = new QLabel("Deletes all runs and processing steps "
                                        "currently loaded, loads runs and "
                                        "processing steps from session file.");
    labelClearInfo->setContentsMargins(17,5,5,5);
    labelClearInfo->setWordWrap(true);
    labelClearInfo->setMaximumWidth(maxOptWidth);
    optionsLayout->addWidget(labelClearInfo);

    radioButtonAddIgnore = new QRadioButton("Add runs, use existing processing steps" );
    radioButtonAddIgnore->setMaximumWidth(maxOptWidth);
    radioButtons->addButton(radioButtonAddIgnore);
    optionsLayout->addWidget(radioButtonAddIgnore);

    QLabel* labelAddIgnoreInfo = new QLabel("Keep runs and processing steps "
                                            "currently loaded and insert "
                                            "runs from session file into existing run structure."
                                            " All processing steps defined within the "
                                            "session file will be ignored.");
    labelAddIgnoreInfo->setContentsMargins(17,5,5,5);
    labelAddIgnoreInfo->setWordWrap(true);
    labelAddIgnoreInfo->setMaximumWidth(maxOptWidth);
    optionsLayout->addWidget(labelAddIgnoreInfo);


    radioButtonAddOverwrite = new QRadioButton("Add runs, use processing steps from session:"  );
    radioButtons->addButton(radioButtonAddOverwrite);
    optionsLayout->addWidget(radioButtonAddOverwrite);
    radioButtonAddOverwrite->setMaximumWidth(maxOptWidth);

    QLabel* labelAddOverwriteInfo = new QLabel("Keep runs "
                                               "currently loaded and insert "
                                               "runs and processing steps from session file "
                                               "into existing run structure."
                                               " All processing steps currently loaded "
                                               "will be overwritten.");
    labelAddOverwriteInfo->setContentsMargins(17,5,5,5);
    labelAddOverwriteInfo->setWordWrap(true);
    labelAddOverwriteInfo->setMaximumWidth(maxOptWidth);
    optionsLayout->addWidget(labelAddOverwriteInfo);


    QWidget *spacer1 = new QWidget(this);
    spacer1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    midLayout->addWidget(spacer1);

    QHBoxLayout* bottomLayout = new QHBoxLayout;
    mainLayout->addLayout(bottomLayout);

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    bottomLayout->addWidget(spacer);

    buttonCancel = new QPushButton("Cancel");
    connect(buttonCancel,SIGNAL(released()),SLOT(onButtonCancelReleased()));
    bottomLayout->addWidget(buttonCancel);

    buttonLoad = new QPushButton("Load");
    connect(buttonLoad,SIGNAL(released()),SLOT(onButtonLoadReleased()));
    bottomLayout->addWidget(buttonLoad);

    // INIT GUISETTINGS
    GuiSettings* gs = Core::instance()->guiSettings;

    labelFname->setText(gs->value("sessionDiag","lastfname",Core::rootDir).toString());

    setDataMode(gs->value("sessionDiag","mode",0).toInt());
}


/**
 * @brief SessionLoadDialog::done overwrites QDialog::done(int)
 * This slot is executed when the dialog has been accepted/rejected/..
 * by the user
 * @param r result parameter, see QDialog::DialogCode
 */
void SessionLoadDialog::done(int r)
{
    QString fname = labelFname->text();

    if(r == QDialog::Accepted)
    {
        if(!QFile(fname).exists())
        {
            QString msg = QString("Cannot load .xml Session file: "
                                  "File '%0' does not exist!")
                                  .arg(fname);
            MSG_ERR(msg);
            QMessageBox::warning(0,"Error",msg);
            return;
        }

        // setup a IO requset
        SignalIORequest rq;
        rq.userData.insert(0,fname);
        rq.itype = XML;

        // setup data mode
        rq.userData.insert(19,dataMode());

        // send request to SignalManager
        Core::instance()->getSignalManager()->importSignalsManager(rq);
    }


    GuiSettings* gs = Core::instance()->guiSettings;
    gs->setValue("sessionDiag","lastfname",fname);
    gs->setValue("sessionDiag","mode",dataMode());
    QDialog::done(r);
}


// -------------
// UI CALLBACKS
// -------------

/**
 * @brief SessionLoadDialog::onButtonSelectReleased This slot is
 * executed when the 'Select' Button has been released. Opens
 * a native file dialog which allows to specify an input file for
 * the session import.
 */
void SessionLoadDialog::onButtonSelectReleased()
{
    // get the path of the last file used from gui settings
    QString currentFname = labelFname->text();
    if(currentFname.isEmpty())
        currentFname = Core::rootDir;

    QString fname = QFileDialog::getOpenFileName(QApplication::focusWidget(),
                                                 "Load .xml Session",
                                                 currentFname,".xml (*.xml)");

    if(!fname.isEmpty())
        labelFname->setText(fname);
}

/**
 * @brief SessionLoadDialog::onButtonCancelReleased This slot is executed
 * when the 'Cancel' Button has been released
 */
void SessionLoadDialog::onButtonCancelReleased()
{
    done(QDialog::Rejected);
}

/**
 * @brief SessionLoadDialog::onButtonLoadReleased This slot is executed
 * when the 'Load' Button has been released
 */
void SessionLoadDialog::onButtonLoadReleased()
{
    done(QDialog::Accepted);
}


// ---------------
// PRIVATE HELPERS
// ---------------

void SessionLoadDialog::setDataMode(int mode)
{
    switch(mode)
    {
    case 0:
        radioButtonClear->setChecked(true);
        break;
    case 1:
        radioButtonAddIgnore->setChecked(true);
        break;
    case 2:
        radioButtonAddOverwrite->setChecked(true);
        break;
    default:
        radioButtonClear->setChecked(true);
    }
}

int SessionLoadDialog::dataMode()
{
    int dataMode = 0;

    if(radioButtonAddIgnore->isChecked())
        dataMode = 1;
    else if(radioButtonAddOverwrite->isChecked())
        dataMode = 2;
    return dataMode;
}

