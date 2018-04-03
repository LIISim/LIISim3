#include "ribbontoolbox.h"
#include <QFile>

#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QDebug>
#include <QToolTip>

RibbonToolBox::RibbonToolBox(QString title, QWidget *parent) : QWidget(parent)
{
    layoutMainV = new QVBoxLayout;
    setLayout(layoutMainV);

    layoutGrid = new QGridLayout;

    layoutMainV->addLayout(layoutGrid);

    sectionTitle = new QLabel(title);
    sectionTitle->setObjectName("sectionTitle");
    sectionTitle->setMaximumHeight(14);
    sectionTitle->setAlignment(Qt::AlignCenter);
    layoutMainV->addWidget(sectionTitle);

    layoutMainV->setMargin(0);
    layoutMainV->setSpacing(0);
    layoutGrid->setSpacing(1);
    layoutGrid->setMargin(4);
}


RibbonToolBox::~RibbonToolBox()
{
}


void RibbonToolBox::addWidget(QWidget* w, int row, int col, int rowspan, int colspan)
{
    layoutGrid->addWidget(w,row,col,rowspan,colspan,Qt::AlignVCenter);
}


/**
 * @brief RibbonToolBox::addViewAction used for VIEW toolboxes
 * @param a
 * @param lb
 * @param row
 * @param col
 * @param rowspan
 * @param colspan
 * @param iconsize
 */
void RibbonToolBox::addViewAction(QAction* a,  int row, int col, int rowspan, int colspan)
{
    // break line
    QString lbl = QString(a->text());
    lbl.replace(" ","\n");
    a->setText(lbl);

    QToolButton* actionButton = new QToolButton();
    actionButton->setDefaultAction(a);
    actionButton->setIcon(a->icon());
    actionButton->setIconSize(QSize(20,20));
    actionButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    actionButton->setFixedWidth(85);
    actionButton->setFixedHeight(56);


    layoutGrid->addWidget(actionButton, row, col, rowspan, colspan, Qt::AlignLeft);
    layoutGrid->setAlignment(Qt::AlignLeft);
    setMinimumWidth(280);

    // add left/right some space
    layoutGrid->setContentsMargins(5,5,5,5);
    layoutGrid->setHorizontalSpacing(6);
}


void RibbonToolBox::addAction(QAction* a, int row, int col, int rowspan, int colspan, int iconsize)
{
    QToolButton* b = new QToolButton(this);
    b->setText(a->text());
    b->setAutoRaise(true);
    b->setIcon(a->icon());
    b->setToolTip(a->toolTip());
    b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    if(iconsize > 0)
    {
        b->setIconSize(QSize(iconsize,iconsize));
        b->setMaximumHeight(iconsize+4);
        b->setMaximumWidth(iconsize+2);
    }

    if(a->isCheckable())
    {
        b->setCheckable(true);
        connect(b,SIGNAL(toggled(bool)),a,SLOT(setChecked(bool)));
        connect(a,SIGNAL(toggled(bool)),b,SLOT(setChecked(bool)));
        connect(b,SIGNAL(released()),a,SLOT(toggle()));
        connect(b,SIGNAL(released()),a,SLOT(trigger()));
    }
    else
    {
        connect(b,SIGNAL(released()),a,SLOT(toggle()));
        connect(b,SIGNAL(released()),a,SLOT(trigger()));
    }

    b->setChecked(a->isChecked());

    layoutGrid->addWidget(b,row,col,rowspan,colspan,Qt::AlignCenter);
}

