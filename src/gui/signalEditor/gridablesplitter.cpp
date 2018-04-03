#include "gridablesplitter.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QFrame>

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------

GridableSplitter::GridableSplitter(Qt::Orientation orientation, QWidget *parent)
    : QSplitter(orientation,parent)
{
    m_mute = false;
    connect(this,SIGNAL(splitterMoved(int,int)),SLOT(handleThisSplitterMoved(int,int)));
}


void GridableSplitter::addWidget(QWidget *widget)
{
    QSplitter::addWidget(widget);
}


void GridableSplitter::copyMovesFromAndTo(GridableSplitter *split)
{
    connect(split,SIGNAL(splitterMoved(int,int)),SLOT(handleOtherSplitterMoved(int,int)));
    m_splitters.append(split);
}


void GridableSplitter::handleOtherSplitterMoved(int pos, int index)
{
    if(m_mute)
        return;

    for(int i = 0; i < m_splitters.size();i++)
        m_splitters[i]->m_mute = true;

    this->moveSplitter(pos,index);

    for(int i = 0; i < m_splitters.size();i++)
        m_splitters[i]->m_mute = false;
}


void GridableSplitter::handleThisSplitterMoved(int pos, int index)
{
    //qDebug() << "GridableSplitter::doMove: " << this->sizes();
    if(m_mute)
        return;

    if(m_splitters.isEmpty())
        return;

    m_mute = true;

    for(int i = 0; i < m_splitters.size();i++)
    {
        m_splitters[i]->doMove(pos,index);
    }
    m_mute = false;
}


void GridableSplitter::doMove(int pos, int index)
{
    this->moveSplitter(pos,index);
}
