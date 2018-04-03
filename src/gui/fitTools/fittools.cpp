#include "fittools.h"

#include "../../core.h"

#include "fitcreator.h"
#include "../utils/materialcombobox.h"
#include "../utils/calculationtoolbox.h"
#include "../signalEditor/memusagewidget.h"

FitTools::FitTools(QWidget *parent) : QWidget(parent)
{
    MSG_DETAIL_1("init FitTools");

    // ----------------------
    // WIDGETS
    // ----------------------

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);;
    setLayout(mainLayout);

    widgetStack = new QStackedWidget;
    mainLayout->addWidget(widgetStack);

    // ----------------------
    // ACTIONGROUP
    // ----------------------

    viewActionGroup = new QActionGroup(this);
    viewActionGroup->setExclusive(true);
    connect(viewActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(onViewActionTriggered(QAction*)));

    // ----------------------
    // TOOLBAR
    // ----------------------

    m_toolbar = new QToolBar;

    // view box
    viewToolbox = new RibbonToolBox("FIT TOOLS");


    // FIT CREATOR

    FitCreator* fitcreator = new FitCreator;
    widgetStack->addWidget(fitcreator);
    setObjectName(fitcreator->objectName());

    viewActionGroup->addAction(fitcreator->viewAction());

    // TODO: ADD GUI FOR FITRUN ANALYSIS HERE


    QAction* toolbarAction = new QAction(fitcreator->viewAction()->icon(), "Fit Creator", this);
    toolbarAction->setCheckable(true);
    toolbarAction->setChecked(true);
    viewToolbox->addViewAction(toolbarAction, 0, 0, 1, 1);

    QAction* toolbarAction2 = new QAction(QIcon(Core::rootDir + "resources/icons/trade.png"), "Fit Analysis", this);
    toolbarAction2->setEnabled(false);
    toolbarAction2->setToolTip("Inactive:\n Tools are in development");
    viewToolbox->addViewAction(toolbarAction2, 0, 1, 1, 1);

    viewToolbox->setMinimumWidth(280);

    m_toolbar->addWidget(viewToolbox);
    m_toolbar->addSeparator();

    // select fit creator at startup
    int idx = viewActionGroup->actions().indexOf(fitcreator->viewAction());
    widgetStack->setCurrentIndex(idx);


    // ----------------------
    // "Current View" toolbox
    // ----------------------
    rtbTool = new RibbonToolBox("CURRENT VIEW");

    labelToolIcon = new QLabel();
    labelToolIcon->setPixmap(fitcreator->viewAction()->icon().pixmap(32,32));
    labelToolName = new QLabel("Fit Creator");
    QFont lbfont;
    lbfont.setBold(true);
    labelToolName->setFont(lbfont);

    labelToolIcon->setAlignment(Qt::AlignLeft);
    labelToolName->setAlignment(Qt::AlignLeft);

    rtbTool->addWidget(labelToolIcon, 1, 1);
    rtbTool->addWidget(labelToolName, 1, 2);

    rtbTool->layoutGrid->setColumnMinimumWidth(2,150);
    rtbTool->setMinimumWidth(200);

    m_toolbar->addWidget(rtbTool);
    m_toolbar->addSeparator();

#ifdef LIISIM_FULL
    // calculation box
    CalculationToolbox *calcToolbox = new CalculationToolbox(this);
    m_toolbar->addWidget(calcToolbox);
    m_toolbar->addSeparator();

    connect(calcToolbox, SIGNAL(recalc(QList<Signal::SType>)), fitcreator, SLOT(onCalcToolboxRecalc(QList<Signal::SType>)));
#endif

    // add some space to toolbar
    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_toolbar->addWidget(spacer2);
    m_toolbar->addSeparator();

    // add notification area
    m_toolbar->addWidget(Core::instance()->getNotificationManager()->getNotificationCenter(this));
    m_toolbar->addSeparator();

    // add Memory usage information to toolbar
    MemUsageWidget* memw = new MemUsageWidget;
    m_toolbar->addWidget(memw);


    /*connect(Core::instance()->getSignalManager(),
            SIGNAL(processingStateChanged(bool)),
            SLOT(onProcessingStateChanged(bool)));
    */
}

FitTools::~FitTools()
{

}


/**
 * @brief FitTools::onViewActionTriggered This slot is executed
 * when an action from the view action group has been triggered.
 * It changes the current widget shown.
 * @param action triggered action
 */
void FitTools::onViewActionTriggered(QAction *action)
{
    int index = viewActionGroup->actions().indexOf(action);
    widgetStack->setCurrentIndex(index);
    setObjectName(widgetStack->currentWidget()->objectName());
}


void FitTools::onCalcActionTriggered()
{
    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot recalculate (active background tasks!)";
        MSG_WARN(msg);
        MSG_STATUS(msg);
        return;
    }
    setCursor(Qt::WaitCursor);
    actionCalc->setEnabled(false);
    Core::instance()->getSignalManager()->processAllMRuns(Signal::TEMPERATURE);
}

void FitTools::onProcessingStateChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
    actionCalc->setEnabled(state);
}

