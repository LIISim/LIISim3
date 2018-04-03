#include "databaseeditor.h"

#include "../../database/databasemanager.h"
#include "gaseditor.h"
#include "materialeditor.h"
#include "gasmixeditor.h"
#include "liisettingseditor.h"
#include "laserenergyeditor.h"
#include "spectrumeditor.h"
#include "transmissioneditor.h"

#include <QDesktopServices>
#include <QFileDialog>
#include "../../core.h"

DatabaseEditor::DatabaseEditor(QWidget *parent) :  QWidget(parent)
{
    this->setWindowTitle("Database Editor");
    this->resize(600,360);

    // setup gui
    lay_main_v_outer = new QVBoxLayout;
    lay_main_v_outer->setMargin(0);
    setLayout(lay_main_v_outer);
    lay_main_v = new QVBoxLayout;
    lay_main_v->setMargin(0);

    lay_main_v_outer->addLayout(lay_main_v);
   // lay_main_v->setMargin(2);

    // gui for database path
    lay_db_path = new QHBoxLayout;
    lay_main_v->addLayout(lay_db_path);

    lbDbPathDescr = new QLabel("Database directory: ");
    lbDbPath = new QLabel(Core::rootDir + "database/");

    // workaround for TutorialViewer (Labels -> Layout -> Container)
    QHBoxLayout* lay_db_path_sub = new QHBoxLayout;
    lay_db_path_sub->addWidget(lbDbPathDescr);
    lay_db_path_sub->addWidget(lbDbPath);
    lay_db_path_sub->setMargin(0);
    QWidget* container_db_path = new QWidget;
    container_db_path->setLayout(lay_db_path_sub);


    bOpenFolder = new QPushButton(QIcon(Core::rootDir + "resources/icons/folder_explore.png"), "");
    bOpenFolder->setMaximumWidth(100);
    connect(bOpenFolder, SIGNAL(released()), this, SLOT(onOpenExplorer()));

    bScanDir = new QPushButton("Rescan Database");
    bScanDir->setMaximumWidth(100);
    connect(bScanDir,SIGNAL(released()),this,SLOT(slot_startDbScan()));

    bChangeDir = new QPushButton(tr("Change Folder"));
    bChangeDir->setMaximumWidth(100);
    connect(bChangeDir,SIGNAL(released()),this,SLOT(slot_onChangeFolder()));

    lay_db_path->addWidget(new QLabel(), Qt::AlignRight);
    lay_db_path->addWidget(bScanDir);
    lay_db_path->addWidget(container_db_path);
    lay_db_path->addWidget(bOpenFolder);
    lay_db_path->addWidget(bChangeDir);

    // gui tabwidget+content
    tabs = new QTabWidget;

    lay_main_v->addWidget(tabs);

    gasEdit = new GasEditor;
    tabs->addTab(gasEdit,tr("Gases"));

    matEdit =new MaterialEditor;
    tabs->addTab(matEdit,tr("Materials"));

    gasmixEdit = new GasMixEditor;
    tabs->addTab(gasmixEdit,tr("Gas Mixtures"));
    connect(gasEdit,SIGNAL(signal_gasesUpdated()),gasmixEdit,SLOT(updateCurrentView()));

    liisettEdit = new LiiSettingsEditor;
    tabs->addTab(liisettEdit,tr("LII-Settings"));

    // set LIISettings as default tab
    tabs->setCurrentIndex(3);

#ifdef LIISIM_FULL
    laserEnergyEdit = new LaserEnergyEditor;
    tabs->addTab(laserEnergyEdit, tr("Laser Energy Settings"));
#endif

#ifdef LIISIM_FULL
    spectrumEditor = new SpectrumEditor;
    tabs->addTab(spectrumEditor, tr("Spectrum Editor"));

    transmissionEditor = new TransmissionEditor;
    tabs->addTab(transmissionEditor, tr("Transmission Editor"));
#endif

    connect(Core::instance()->getDatabaseManager(),SIGNAL(signal_scanDatabaseFinished()),
            this, SLOT(slot_scanFinished()));

    connect(this,SIGNAL(signal_requestDbScan()),
            Core::instance()->getDatabaseManager(),SLOT(slot_scanDatabase()));

    connect(Core::instance()->generalSettings,SIGNAL(settingsChanged()),SLOT(onGeneralSettingsChanged()));

    db_manager = NULL;

    onGeneralSettingsChanged();
    setDatabaseManager(Core::instance()->getDatabaseManager());

    bScanDir->setObjectName("DBE_BTN_SCAN_DIR");
    container_db_path->setObjectName("DBE_CURRENT_PATH");
    bOpenFolder->setObjectName("DBE_BTN_OPEN_FOLDER");
    bChangeDir->setObjectName("DBE_BTN_CHANGE_FOLDER");
    tabs->setObjectName("DBE_EDITOR_TABS");
}


void DatabaseEditor::setDatabaseManager(DatabaseManager *dbm)
{
    this->db_manager = dbm;
    gasEdit->setDatabaseManager(dbm);
    matEdit->setDatabaseManager(dbm);
    gasmixEdit->setDatabaseManager(dbm);
    liisettEdit->setDatabaseManager(dbm);
#ifdef LIISIM_FULL
    laserEnergyEdit->setDatabaseManager(dbm);
#endif

#ifdef LIISIM_FULL
    spectrumEditor->setDatabaseManager(dbm);
    transmissionEditor->setDatabaseManager(dbm);
#endif

    connect(dbm,SIGNAL(signal_contentChanged(int)),SLOT(onDatabaseContentChanged(int)));
    slot_scanFinished();
}


/**
 * @brief handle database scan
 */
void DatabaseEditor::slot_startDbScan()
{
    bScanDir->setEnabled(false);
    bChangeDir->setEnabled(false);
    tabs->setEnabled(false);

    MSG_STATUS_CONST("scanning database ... ");

    Core::instance()->generalSettings->setDatabaseDirectory(lbDbPath->text());

    emit signal_requestDbScan();

}


/**
 * @brief visualize scan results
 */
void DatabaseEditor::slot_scanFinished()
{
   /* qDebug() << "database editor: found "<< db_manager->getGases()->size() << " gases, "<< db_manager->getMaterials()->size()<<" materials";
    for(int i=0; i< db_manager->getGases()->size(); i++)
    {
        qDebug() << "\t" << db_manager->getGases()->at(i).name.c_str() << "\t\t" << db_manager->getGases()->at(i).filename.c_str();
    }

    for(int i=0; i< db_manager->getMaterials()->size(); i++)
    {
        qDebug() << "\t" << db_manager->getMaterials()->at(i).name.c_str() << "\t\t" << db_manager->getMaterials()->at(i).filename.c_str();
    }*/

    bScanDir->setEnabled(true);
    bChangeDir->setEnabled(true);
    tabs->setEnabled(true);
    gasEdit->initData();
    matEdit->initData();
    gasmixEdit->initData();
    liisettEdit->initData();
#ifdef LIISIM_FULL
    laserEnergyEdit->initData();
#endif

#ifdef LIISIM_FULL
    spectrumEditor->initData();
    transmissionEditor->initData();
#endif
}


void DatabaseEditor::slot_onChangeFolder()
{
    QString newdir = QFileDialog::getExistingDirectory(this,tr("Select Database Directory"),"../",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

    if(!newdir.isEmpty())
    {
        if(!newdir.endsWith("/"))
            newdir.append("/");

        lbDbPath->setText(newdir);
        slot_startDbScan();
    }
    else
    {
        bScanDir->setEnabled(true);
        bChangeDir->setEnabled(true);
        tabs->setEnabled(true);
    }
}


/**
 * @brief DatabaseEditor::onGeneralSettingsChanged handle modifications
 * of GeneralSettings
 */
void DatabaseEditor::onGeneralSettingsChanged()
{
    lbDbPath->setText(Core::instance()->generalSettings->databaseDirectory());
}


void DatabaseEditor::onDatabaseContentChanged(int dbc_indent)
{
    slot_scanFinished();
}


/**
 * @brief DatabaseEditor::onOpenExplorer  opens dir with external application
 */
void DatabaseEditor::onOpenExplorer()
{
    QUrl url = QUrl::fromLocalFile(Core::instance()->generalSettings->databaseDirectory());

    // opens file/directory with external application
    QDesktopServices::openUrl(url);
}

