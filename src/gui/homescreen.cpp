#include "homescreen.h"

#include "utils/ribbontoolbox.h"
#include "../core.h"
#include "settingsWidgets/generalsettingswidget.h"
#include "signalEditor/memusagewidget.h"

#include "io/ioxml.h"

HomeScreen::HomeScreen(QWidget *parent) : QWidget(parent)
{
    MSG_DETAIL_1("init HomeScreen");
    setObjectName("Homescreen");

    // init toolbar
    m_ribbonToolbar = new QToolBar(this);

    // "session" toolbox
    RibbonToolBox* sessionbox = new RibbonToolBox("SESSION");
    sessionbox->addAction(Core::instance()->actionLoadXMLSession,0,0);
    sessionbox->addAction(Core::instance()->actionSaveXMLSession,1,0);
    sessionbox->layoutGrid->setColumnMinimumWidth(0, 130);
    m_ribbonToolbar->addWidget(sessionbox);
    m_ribbonToolbar->addSeparator();

    // "programsettings" toolbox
    RibbonToolBox* settingsbox = new RibbonToolBox("PROGRAM SETTINGS");
    settingsbox->addAction(Core::instance()->actionLoadProgramSettings,0,0);
    settingsbox->addAction(Core::instance()->actionSaveProgramSettings,1,0);
    settingsbox->layoutGrid->setColumnMinimumWidth(0,165);
    settingsbox->layoutGrid->setColumnMinimumWidth(1,140);
    settingsbox->layoutGrid->setAlignment(Qt::AlignLeft);

    checkBoxLoadDataAtStartup = new QCheckBox("Automatic signal loading");
    settingsbox->addWidget(checkBoxLoadDataAtStartup,0,1,1,1);

    actionSaveAsDefault = new QAction("Save as default", this);
    actionSaveAsDefault->setIcon(QIcon(Core::rootDir + "resources/icons/cog.png"));
    settingsbox->addAction(actionSaveAsDefault, 1, 1, 1, 1);

    connect(checkBoxLoadDataAtStartup,
            SIGNAL(toggled(bool)),
            SLOT(onCheckBoxLoadDataAtStartupToggled(bool)));

    connect(actionSaveAsDefault,
            SIGNAL(triggered(bool)),
            SLOT(onActionSaveAsDefaultTriggered()));

    connect(Core::instance()->generalSettings,
            SIGNAL(settingsChanged()),
            SLOT(onGeneralSettingsChanged()));


    m_ribbonToolbar->addWidget(settingsbox);
    m_ribbonToolbar->addSeparator();

    // "files" toolbox
    RibbonToolBox* filesbox = new RibbonToolBox("SIGNAL DATA");
    filesbox->addAction(Core::instance()->getSignalManager()->actionDataImport(),0,0);
    filesbox->addAction(Core::instance()->getSignalManager()->actionDataExport(),1,0);
    filesbox->layoutGrid->setColumnMinimumWidth(0, 130);
    m_ribbonToolbar->addWidget(filesbox);
    m_ribbonToolbar->addSeparator();


    // add Space to toolbar
    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_ribbonToolbar->addWidget(spacer2);
    m_ribbonToolbar->addSeparator();

    // add notification center
    QWidget* nc = Core::instance()->getNotificationManager()->getNotificationCenter(this);
    m_ribbonToolbar->addWidget(nc);
    m_ribbonToolbar->addSeparator();

    // add Memory usage information to toolbar
    MemUsageWidget* memw = new MemUsageWidget;
    m_ribbonToolbar->addWidget(memw);

    // main layout
    grid = new QGridLayout;
    setLayout(grid);

    splith0 = new QSplitter(Qt::Horizontal);
    grid->addWidget(splith0);

    // LEFT LAYOUT: MODELING/GENERAL SETTINGS
    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    QWidget* leftWidget = new QWidget;
    leftWidget->setLayout(leftLayout);
    leftWidget->setMaximumWidth(250);

    splith0->addWidget(leftWidget);
    GeneralSettingsWidget* gs = new GeneralSettingsWidget;
    leftLayout->addWidget(gs);
    leftLayout->addStretch(-1);

    connect(gs->buttonTutorial, SIGNAL(clicked(bool)), SLOT(onGeneralSettingsShowTutorial()));

    // RIGHT LAYOUT: LOG
    logview = new LogMessageWidget;
    splith0->addWidget(logview);

//    splitv1r = new QSplitter(Qt::Vertical);
//    splith0->addWidget(splitv1r);

//    logview = new LogMessageWidget;
//    dbEdit = new DatabaseEditor;
//    dbEdit->setVisible(false);

//    splitv1r->addWidget(dbEdit);
//    splitv1r->addWidget(logview);

    sessionbox->setObjectName("HS_SESSION_BOX");
    settingsbox->setObjectName("HS_SETTINGS_BOX");
    filesbox->setObjectName("HS_FILES_BOX");
    nc->setObjectName("HS_NOTIFICATION_CENTER");
    memw->setObjectName("HS_MEMORY_BOX");
    leftWidget->setObjectName("HS_SETTINGS_WIDGET");
    logview->setObjectName("HS_LOG_WIDGET");
}

HomeScreen::~HomeScreen()
{
}


/**
 * @brief HomeScreen::onCheckBoxLoadDataAtStartupToggled Checkbox callback function.
 * This slot is executed when the 'Load data at program startup' checkbox has
 * been toggled. It updates the corresponding Core::generalSettings entry
 * @param state
 */
void HomeScreen::onCheckBoxLoadDataAtStartupToggled(bool state)
{
    checkBoxLoadDataAtStartup->blockSignals(true);
    Core::instance()->generalSettings->setLoadDataAtStartup(state);
    checkBoxLoadDataAtStartup->blockSignals(false);
}


void HomeScreen::onActionSaveAsDefaultTriggered()
{
    Core::instance()->saveProgramSettings(Core::rootDir + "defaultSettings.ini");

    IOxml::saveInitSession();
}


/**
 * @brief HomeScreen::onGeneralSettingsChanged Slot listening to changes
 * of Core::generalSettings Object. Updates the 'Load data at program startup' checkbox
 * state.
 */
void HomeScreen::onGeneralSettingsChanged()
{
    checkBoxLoadDataAtStartup->blockSignals(true);
    checkBoxLoadDataAtStartup->setChecked(Core::instance()->generalSettings->loadDataAtStartup());
    checkBoxLoadDataAtStartup->blockSignals(false);
}


void HomeScreen::onGeneralSettingsShowTutorial()
{
    emit showTutorials();
}
