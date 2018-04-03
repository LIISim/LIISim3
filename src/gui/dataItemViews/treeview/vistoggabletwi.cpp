#include "vistoggabletwi.h"

#include <QDebug>
#include "../../core.h"

VisToggableTWI::VisToggableTWI(QTreeWidget *view)
    : QObject(0),
      QTreeWidgetItem(view)
{
    init();
}


VisToggableTWI::VisToggableTWI(QTreeWidget *view, QString text)
    : QObject(0),
      QTreeWidgetItem(view)
{
    init();
    setText(1,text);
    //m_nameLabel->setText(text);
}


VisToggableTWI::~VisToggableTWI()
{
}


void VisToggableTWI::init()
{
    m_mainItemWidget = new QWidget;
    m_itwLayout = new QHBoxLayout;
    m_mainItemWidget->setLayout(m_itwLayout);
    m_itwLayout->setMargin(0);

    m_visButton = new QPushButton();
    m_visButton->setFlat(true);
    m_visButton->setMaximumHeight(17);
    m_visButton->setMaximumWidth(17);
    m_itwLayout->addWidget(m_visButton);

  //  m_nameLabel = new QLabel("Name");
  //  m_itwLayout->addWidget(m_nameLabel);

    connect(m_visButton,SIGNAL(released()),SLOT(onVisButtonReleased()));
    setupItemWidget();
    updatePlotVisibilityIcon();
}


void VisToggableTWI::setupItemWidget()
{
    treeWidget()->setItemWidget(this,0,m_mainItemWidget);
}


void VisToggableTWI::setVisState(bool state)
{
    setData(0,Qt::UserRole+5,state);
    updatePlotVisibilityIcon();
}


void VisToggableTWI::onVisButtonReleased()
{
    bool state = this->data(0,Qt::UserRole+5).toBool();
    setData(0,Qt::UserRole+5,!state);
    updatePlotVisibilityIcon();
    emit dataChanged(5,!state);
}


void VisToggableTWI::updatePlotVisibilityIcon()
{
    bool state = this->data(0,Qt::UserRole+5).toBool();

    if(state)
    {
        QIcon ic(Core::rootDir + "resources/icons/visible1_32.png");
        m_visButton->setIcon(ic);
    }
    else
    {
        QIcon ic(Core::rootDir + "resources/icons/visible1gray_32.png");
        m_visButton->setIcon(ic);
    }
}


