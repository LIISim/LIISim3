#include "databasewindow.h"

#include "../../core.h"

#include "../signalEditor/memusagewidget.h"

#include <QToolButton>

DatabaseWindow::DatabaseWindow(QWidget *parent) : QWidget(parent)
{
    MSG_DETAIL_1("init DatabaseWindow");
    setObjectName("DatabaseWindow");

    dbEdit = new DatabaseEditor;

    // init toolbar
    m_ribbonToolbar = new QToolBar;

    // "view" toolbox
    RibbonToolBox* viewToolbox = new RibbonToolBox("DATABASE");
    QAction* viewAction = new QAction(QIcon(Core::rootDir + "resources/icons/table.png"), "Database Editor", this);
    viewAction->setCheckable(true);
    viewAction->setChecked(true);

    viewToolbox->addViewAction(viewAction, 1, 0, 1, 1); // colspan 1 = tools.size()

    m_ribbonToolbar->addWidget(viewToolbox);
    m_ribbonToolbar->addSeparator();

    // "database directory" toolbox
//    RibbonToolBox* dbbox = new RibbonToolBox("DATABASE");
//
//    QLabel* descr1 = new QLabel("  Database directory: ");
//    QLabel* lb_dirname = new QLabel();
//    dbbox->addWidget(descr1,0,0);
//    dbbox->addWidget(lb_dirname, 0, 1, 1, 2); // colspan = 2

//    m_ribbonToolbar->addWidget(dbbox);
//    m_ribbonToolbar->addSeparator();

    // add Space to toolbar
    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_ribbonToolbar->addWidget(spacer2);
    m_ribbonToolbar->addSeparator();

    // add notification center
    QWidget *nc = Core::instance()->getNotificationManager()->getNotificationCenter(this);
    m_ribbonToolbar->addWidget(nc);
    m_ribbonToolbar->addSeparator();

    // add Memory usage information to toolbar
    MemUsageWidget* memw = new MemUsageWidget;
    m_ribbonToolbar->addWidget(memw);

    // main layout
    grid = new QGridLayout;
    setLayout(grid);

    grid->addWidget(dbEdit);

    nc->setObjectName("DBE_NOTIFICATION_CENTER");
    memw->setObjectName("DBE_MEMORY_BOX");
}

