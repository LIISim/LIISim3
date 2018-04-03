#include "ribbontabbar.h"

#include <QFile>
#include <QDebug>
#include <QToolButton>

#include "../../core.h"

RibbonTabBar::RibbonTabBar(QWidget *parent) : QWidget(parent)
{
    topBarHeight  = 21; // height of tabbuttons and upper (dark) toolbar
    toolBarHeight = 80;
    minTabWidth = 90;

    // set custom style sheet  (if stylesheet resource exists)
    if(QFile::exists(Core::rootDir + "resources/style/ribbontabbarstyle.qss"))
    {
        QFile file(Core::rootDir + "resources/style/ribbontabbarstyle.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        setStyleSheet(styleSheet);
    }
    else
    {
        MSG_ERR("Cannot find stylesheet: " + Core::rootDir + "resources/style/ribbontabbarstyle.qss");
    }

    layoutOuter = new QVBoxLayout(this);
    layoutTabBar = new QHBoxLayout;
    layoutOuter->addLayout(layoutTabBar);

    tabGroup =new QButtonGroup(this);


    topToolBarRight = new QToolBar;
    topToolBarRight->setObjectName("topToolBarRight");

    topToolBarLeft = new QToolBar;
    topToolBarLeft->setObjectName("topToolBarLeft");

    layoutTabBar->addWidget(topToolBarLeft);
    layoutTabBar->addWidget(topToolBarRight);

    // right top toolbar

    actionShowHideTB =new QAction(this);
    actionShowHideTB->setToolTip("Minimize Toolstrip");
    QIcon ic(Core::rootDir + "resources/icons/doubleArrowUp.png");
    actionShowHideTB->setIcon(ic);

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topToolBarRight->addWidget(spacer);
    topToolBarRight->addAction(actionShowHideTB);
    topToolBarRight->setIconSize(QSize(12,12));

    // bottom toolbar stack

    toolBarStack = new QStackedWidget;
    layoutOuter->addWidget(toolBarStack);
    toolBarStack->setMinimumHeight(toolBarHeight);
    toolBarStack->setMaximumHeight(toolBarHeight);


    // set sizes/spacings/margins/connections

    layoutOuter->setMargin(0);
    layoutOuter->setSpacing(0);
    layoutTabBar->setMargin(0);
    layoutTabBar->setSpacing(0);
    layoutOuter->addStretch(0);

    topToolBarRight->setMinimumHeight(topBarHeight);
    topToolBarLeft->setMinimumHeight(topBarHeight);

    connect(tabGroup,
            SIGNAL(buttonToggled(int,bool)),
            SLOT(onButtonToggled(int,bool)));
    connect(actionShowHideTB,
            SIGNAL(triggered()),
            SLOT(onActionShowHideTb()));
    setMaximumHeight(topBarHeight+toolBarHeight);

}

RibbonTabBar::~RibbonTabBar()
{

}

void RibbonTabBar::setSelectedTab(int i)
{
    if(i < 1 || i > tabGroup->buttons().size())
        return;

    tabGroup->button(i)->setChecked(true);
}


void RibbonTabBar::onActionShowHideTb()
{
    if(toolBarStack->isVisible())
    {
        setMaximumHeight(topBarHeight);
        toolBarStack->setVisible(false);
        QIcon ic(Core::rootDir + "resources/icons/doubleArrowDown.png");
        actionShowHideTB->setIcon(ic);
        actionShowHideTB->setToolTip("Maximize Toolstrip");

    }
    else
    {
        setMaximumHeight(topBarHeight+toolBarHeight);
        toolBarStack->setVisible(true);
        QIcon ic(Core::rootDir + "resources/icons/doubleArrowUp.png");
        actionShowHideTB->setIcon(ic);
        actionShowHideTB->setToolTip("Minimize Toolstrip");

    }
}


void RibbonTabBar::onButtonToggled(int id, bool state)
{
    if(!state)
        return;

    toolBarStack->setCurrentIndex(id-1);
    emit selectedTabChanged(id);
}

void RibbonTabBar::addTab(const QString &name, QToolBar *toolbar)
{
    QPushButton* tabbutton = new QPushButton();
    tabbutton->setText(name);
    tabbutton->setCheckable(true);
    tabbutton->setMinimumHeight(topBarHeight);
    tabbutton->setMinimumWidth(minTabWidth);

  //  layoutTabButtons->addWidget(tabbutton);
    topToolBarLeft->addWidget(tabbutton);

    if(!toolbar)
        toolBarStack->addWidget(new QToolBar);
    else
        toolBarStack->addWidget(toolbar);

    tabGroup->addButton(tabbutton,toolBarStack->count());
}


void RibbonTabBar::addTopToolbarAction(QAction *action)
{
    topToolBarRight->insertAction(actionShowHideTB,action);
}

