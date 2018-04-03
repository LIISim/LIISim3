#include "masterwindow.h"

#include "../core.h"

#include <QGroupBox>
#include <QToolButton>
#include <QFile>
#include <QStyle>
#include <QMessageBox>
#include <QApplication>

#include "homescreen.h"
#include "databaseEditor/databasewindow.h"
#include "signalEditor/signalprocessingeditor.h"
#include "analysisTools/analysistools.h"
#include "fitTools/fittools.h"

#include "../io/iobase.h"

// include datacquisition tools if LIISIM_PICOSCOPE
// has been defined (also see: LIISim3.pro project file)
#ifdef LIISIM_PICOSCOPE
#include "dataAcquisition/dataacquisitionwindow.h"
#endif

int MasterWindow::masterWindowCount = 0;

QString MasterWindow::identifier_masterwindow   = "MasterWindow";
QString MasterWindow::identifier_tutorial       = "Tutorial";

/**
 * @brief MasterWindow::MasterWindow
 * @param parent
 */
MasterWindow::MasterWindow(QWidget *parent) : QMainWindow(parent)
{
    MSG_DETAIL_1("init MasterWindow");

    setAttribute(Qt::WA_DeleteOnClose);
    masterWindowCount++;

    aboutWindow = nullptr;

    QString title_pre;
    if(Core::LIISIM_VERSION_PRE)
        title_pre = QString(" - pre");
    else
        title_pre = QString("");

    #ifdef LIISIM_FULL
        m_windowTitle = QString("LIISim %0 (FULL) %1").arg(Core::LIISIM_VERSION).arg(title_pre);
    #else
        m_windowTitle = QString("LIISim %0 %1").arg(Core::LIISIM_VERSION).arg(title_pre);
    #endif

    setWindowTitle(m_windowTitle);

    Core::instance(); // initializes the Application logic (if not initialized yet.)

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // set general style sheet
    if(QFile::exists(Core::rootDir + "resources/style/style.qss"))
    {
        QFile file(Core::rootDir + "resources/style/style.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        setStyleSheet(styleSheet);
    }
    else
    {
        MSG_ERR("Cannot find stylesheet: " + Core::rootDir + "resources/style/style.qss");
    }

    layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetMaximumSize);
    centralWidget->setLayout(layout);

    topBar = new RibbonTabBar;
    topBar->setObjectName("MasterTopBar");
//    topBar->setMaximumHeight(52);
    layout->addWidget(topBar);
    layout->addSpacing(-1);

    widgetStack = new QStackedWidget;

    layout->addWidget(widgetStack);

    // STATUS BAR
    statusbar = new StatusMessageWidget;
    setStatusBar(statusbar);

    // STATUS BAR - MODELING SETTINGS INFORMATION

    status_modelingSettingsInfo = new QLabel("LIISettings");
    status_modelingSettingsInfo->setToolTip("current modeling settings");
    statusbar->addPermanentWidget(status_modelingSettingsInfo);

    // STATUS BAR - PROGRESS BAR

    status_progressBar = new QProgressBar;
    status_progressBar->setMaximumHeight(15);
    status_progressBar->setMaximumWidth(120);
    status_progressBar->setMinimum(0);
    status_progressBar->setMaximum(100);
    status_progressBar->setTextVisible(true);

    connect(Core::instance()->getSignalManager(),
            SIGNAL(progressUpdate(int)),
            status_progressBar, SLOT(setValue(int)));

    statusbar->addPermanentWidget(status_progressBar);
    status_progressBar->setVisible(Core::instance()->getSignalManager()->isImporting());

    status_abort = new QPushButton("Abort");
    status_abort->setToolTip("Abort all pending Signal Imports");
    statusbar->addPermanentWidget(status_abort);

    status_abort->setVisible(Core::instance()->getSignalManager()->isImporting());

    connect(status_abort,SIGNAL(released()),SLOT(onAbortReleased()));

    // HOME SCREEN

    HomeScreen* homeScreen = new HomeScreen;
    widgetStack->addWidget(homeScreen);
    topBar->addTab("Home", homeScreen->ribbonToolbar());

    // DATABASE EDITOR (same ribbonToolbar as homeScreen)

    DatabaseWindow* databaseWindow = new DatabaseWindow;
    widgetStack->addWidget(databaseWindow);
    topBar->addTab("Database", databaseWindow->ribbonToolbar());

    // DATA ACQUISITION

    // include datacquisition tools if LIISIM_PICOSCOPE
    // has been defined (also see: LIISim3.pro project file)
    #ifdef LIISIM_PICOSCOPE
        DataAcquisitionWindow* dataAcquisitionWindow = new DataAcquisitionWindow;
        widgetStack->addWidget(dataAcquisitionWindow);
        topBar->addTab("Data Acquisition", dataAcquisitionWindow->ribbonToolbar());        
    #endif

    // SIGNAL EDITOR

    SignalProcessingEditor* sigEdit =new SignalProcessingEditor;
    widgetStack->addWidget(sigEdit);
    topBar->addTab("Signal Processing",sigEdit->ribbonToolbar());

    // ANALYSIS TOOLS

    AnalysisTools* atools = new AnalysisTools;
    widgetStack->addWidget(atools);
    topBar->addTab("Analysis Tools",atools->ribbonToolbar());

    // FIT TOOLS

    FitTools* ftools = new FitTools;
    widgetStack->addWidget(ftools);
    topBar->addTab("Fit Tools",ftools->toolbar());

    // top-right actions

    actionHelp = new QAction("Help", this);
    topBar->addTopToolbarAction(actionHelp);

    actionAbout = new QAction("About LIISim", this);
    topBar->addTopToolbarAction(actionAbout);

    actionNewWindow = new QAction("New Window", this);
    topBar->addTopToolbarAction(actionNewWindow);

  //  ribbonMenu->setMaximumHeight(35);

    connect(topBar,
            SIGNAL(selectedTabChanged(int)),
            SLOT(onTabChanged(int)));

    connect(actionNewWindow,
            SIGNAL(triggered()),
            SLOT(onActionNewWindow()));

    connect(actionHelp,
            SIGNAL(triggered()),
            SLOT(onActionHelp()));

    connect(actionAbout,
            SIGNAL(triggered()),
            SLOT(onActionAbout()));

    connect(Core::instance()->getSignalManager(),
            SIGNAL(importStateChanged(bool)),
            SLOT(handleIOStateChanged(bool)));

    connect(Core::instance()->getSignalManager(),
            SIGNAL(exportStateChanged(bool)),
            SLOT(handleIOStateChanged(bool)));

    connect(Core::instance()->modelingSettings,
            SIGNAL(settingsChanged()),
            SLOT(onModelingSettingsChanged()));

    connect(Core::instance()->guiSettings,
            SIGNAL(settingsChanged()),
            SLOT(onGUISettingsChanged()));

    connect(Core::instance()->generalSettings,
            SIGNAL(currentLocaleChanged(QLocale)),
            SLOT(onCurrentLocaleChanged(QLocale)));

    connect(Core::instance(),
            SIGNAL(sig_updateProgressBar(int)),
            SLOT(onUpdateProgressBar(int)));

    connect(homeScreen,
            SIGNAL(showTutorials()),
            SLOT(onShowTutorials()));

    onModelingSettingsChanged();
    MSG_DETAIL_1("MasterWindow init done.");

    showMaximized();
    //resize(1000,750);

    // create object to visualize tutorial steps
    tutorialViewer = new TutorialViewer(this);
}


MasterWindow::~MasterWindow()
{
    if(aboutWindow != nullptr)
        delete aboutWindow;

    masterWindowCount--;
}


void MasterWindow::onTabChanged(int i)
{
    if(i < 1 || i > widgetStack->count())
        return;

    widgetStack->setCurrentIndex(i-1);

    Core::instance()->guiSettings->setValue("MasterWindow", "SelectedTab", i-1);

    if(!Core::instance()->guiSettings->value(identifier_tutorial, widgetStack->currentWidget()->objectName(), false).toBool())
    {
        qApp->processEvents();

        // show welcome screen on first start
        if(!Core::instance()->settingsFound()
                || Core::instance()->guiSettings->value(identifier_tutorial, "showWelcome", false).toBool())
            tutorialViewer->setFirstStart();

        tutorialViewer->showTutorial(widgetStack->currentWidget()->objectName());
    }
    Core::instance()->guiSettings->setValue(identifier_tutorial, widgetStack->currentWidget()->objectName(), true);
}


void MasterWindow::handleIOStateChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);

        status_progressBar->setToolTip("IO progress");
        status_progressBar->setValue(0);
        status_progressBar->setVisible(true);
    }
    else
    {
        setCursor(Qt::ArrowCursor);

        status_progressBar->setVisible(false);
    }
    status_abort->setVisible(state);
}


void MasterWindow::onModelingSettingsChanged()
{
    QString statusText;

    statusText.append("Spectroscopic Material: ");
    statusText.append(Core::instance()->modelingSettings->materialSpec().name + " - ");
    statusText.append("Heat transfer Material: ");
    statusText.append(Core::instance()->modelingSettings->material().name + " - ");
    statusText.append(Core::instance()->modelingSettings->gasMixture().name + " - ");

    statusText.append(Core::instance()->modelingSettings->heatTransferModel()->name + " ");

    status_modelingSettingsInfo->setText(statusText);
}


// ---------------
// Action Handlers
// ---------------

void MasterWindow::onActionNewWindow()
{
    MasterWindow* w = new MasterWindow;
    w->show();
}


void MasterWindow::onActionHelp()
{
    tutorialViewer->showTutorial(widgetStack->currentWidget()->objectName());
}


void MasterWindow::onActionAbout()
{    
    if(aboutWindow == nullptr)
        aboutWindow = new AboutWindow(m_windowTitle);
    aboutWindow->show();
}

void MasterWindow::onAbortReleased()
{
    IOBase::abort_flag = true;
}


void MasterWindow::onGUISettingsChanged()
{
    widgetStack->setCurrentIndex(Core::instance()->guiSettings->value("MasterWindow", "SelectedTab").toInt());
    topBar->setSelectedTab(Core::instance()->guiSettings->value("MasterWindow", "SelectedTab").toInt()+1);

    if(Core::instance()->guiSettings->value("aboutWindow", "showAtStart", true).toBool())
    {
        if(aboutWindow == nullptr)
            aboutWindow = new AboutWindow(m_windowTitle);
        aboutWindow->show();
    }
}


void MasterWindow::onShowTutorials()
{
    for(int i = 0; i < widgetStack->count(); i++)
        Core::instance()->guiSettings->setValue(identifier_tutorial, widgetStack->widget(i)->objectName(), false);

    Core::instance()->guiSettings->setValue(identifier_tutorial, widgetStack->currentWidget()->objectName(), true);
    tutorialViewer->showTutorial(widgetStack->currentWidget()->objectName());
}


void MasterWindow::onCurrentLocaleChanged(QLocale locale)
{
    this->setLocale(locale);
}


void MasterWindow::onUpdateProgressBar(int value)
{
    if(value >= 100)
    {
        status_progressBar->setValue(value);
        status_progressBar->setVisible(false);
    }
    else
    {
        status_progressBar->setVisible(true);
        status_progressBar->setValue(value);
    }
}

