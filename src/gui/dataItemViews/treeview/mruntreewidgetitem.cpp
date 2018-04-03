#include "mruntreewidgetitem.h"

#include <QColor>
#include <QColorDialog>

#include "../../../core.h"
#include "../../signal/mrungroup.h"


/**
 * @brief MRunTreeWidgetItem::MRunTreeWidgetItem creates a QTreeWidgetItem for
 * the given MRun and adds it as a new child to the parent QTreeWidgetItem.
 * @param mrun
 * @param parentItem parent item (default = 0, item is not inserted to view!)
 */
MRunTreeWidgetItem::MRunTreeWidgetItem(DataItemTreeView::VisType vistype, MRun* mrun, QTreeWidgetItem *parentItem)
    : DataItemObserverObject(mrun),
      QTreeWidgetItem(parentItem)
{
    m_mute = true;
    m_mrun = mrun;
    this->vistype = vistype;

    setText(0,mrun->getName());
    updateTooltip();


    setData(0,Qt::UserRole,2);
    setData(0,Qt::UserRole+1,mrun->id());

    if(parentItem)
        setupItemWidgets();

    // setup QTreeWidgetItem flags
   /* setFlags(Qt::ItemIsEnabled |
             Qt::ItemIsSelectable |
             Qt::ItemIsEditable   |
             Qt::ItemIsDragEnabled |
             Qt::ItemIsDropEnabled  );*/

    if(vistype == DataItemTreeView::EXP_DIAG)
    {
        setFlags(Qt::ItemIsEnabled |
                 Qt::ItemIsSelectable |
                 Qt::ItemIsEditable   |
                 Qt::ItemIsDragEnabled
             /*    Qt::ItemIsUserCheckable*/
                 );
        setCheckState(0,Qt::Unchecked);
    }
    else
    {
        setFlags(Qt::ItemIsEnabled |
                 Qt::ItemIsSelectable |
                 Qt::ItemIsEditable   |
                 Qt::ItemIsDragEnabled
                 );

    }

    if(vistype == DataItemTreeView::SE_GROUPS || vistype == DataItemTreeView::ATOOL || vistype == DataItemTreeView::EXP_DIAG)
    {
        connect(mrun->calculationStatus(),SIGNAL(changed()),SLOT(onRunStatusChanged()));
        onRunStatusChanged();
    }

    m_mute = false;
    m_visualState = false;

}


/**
 * @brief MRunTreeWidgetItem::~MRunTreeWidgetItem Destructor.
 */
MRunTreeWidgetItem::~MRunTreeWidgetItem()
{
}


/**
 * @brief MRunTreeWidgetItem::setupItemWidgets sets the Item-Widgets
 * for custom columns within the QTreeWidget-parent.
 * @details This method requires, that the item is visible in a
 * QTreeWidget. Therefore it must have been added as a child to antother
 * QTreeWidgetItem or directly to a QTreeWidget
 */
void MRunTreeWidgetItem::setupItemWidgets()
{
    if(!treeWidget())
        return;

    // only necessary for multiple columns (eg Analysistools)
    if(vistype == DataItemTreeView::ATOOL)
    {
        m_spinSigIdx = new QSpinBox;
        m_spinSigIdx->setToolTip("Select signal index for visualization");
        m_spinSigIdx->setMinimumWidth(45);
        m_spinSigIdx->setMaximumWidth(45);
        m_spinSigIdx->setMaximumHeight(17);
        m_spinSigIdx->setValue(1);
        m_spinSigIdx->setMinimum(1);
        m_spinSigIdx->setMaximum(m_mrun->sizeAllMpoints());
        connect(Core::instance()->getSignalManager(),SIGNAL(importFinished()),
                SLOT(onImportFinished()));
        connect(m_spinSigIdx,SIGNAL(valueChanged(int)),SLOT(onSigIdxChanged(int)));

        m_colorLabel = new QLabel;
        m_colorLabel->setMinimumWidth(17);
        m_colorLabel->setMaximumWidth(17);
        m_colorLabel->setToolTip("Color of measurement run in plot\n(Double click to change color)");

        // add a white dummy label next to the color label to hide the selection frame!
        QLabel* spaceDummy = new QLabel;
        QWidget* colorWidget = new QWidget;
        QHBoxLayout* colorLayout = new QHBoxLayout;
        colorLayout->setSpacing(0);
        colorLayout->setMargin(0);
        colorLayout->addWidget(m_colorLabel);
        colorLayout->addWidget(spaceDummy);
        colorWidget->setLayout(colorLayout);

        QColor curMrunCol = m_mrun->data(1).value<QColor>();
        QString style;
        style.sprintf("background-color: rgb(%d, %d, %d);",curMrunCol.red(),curMrunCol.green(),curMrunCol.blue());
        m_colorLabel->setStyleSheet(style);
        style.sprintf("background-color: rgb(%d, %d, %d);",255,255,255);
        spaceDummy->setStyleSheet(style);

        treeWidget()->setItemWidget(this,1,m_spinSigIdx);
        treeWidget()->setItemWidget(this,2,colorWidget);


        treeWidget()->setColumnWidth(0,treeWidget()->width() -17
                                     -m_spinSigIdx->width()
                                     -m_colorLabel->width());
        treeWidget()->setColumnWidth(1,m_spinSigIdx->width());
        treeWidget()->setColumnWidth(2,m_colorLabel->width());
        setCheckState(0,Qt::Unchecked);
    }
    else if(vistype == DataItemTreeView::DA_VC_GROUPS)
    {
        m_showButton = new QToolButton();
        m_showButton->setText("SHOW");
        m_showButton->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
        m_showButton->setStyleSheet("QToolButton { border-style: none}");
        m_showButton->setMaximumWidth(18);

        treeWidget()->setItemWidget(this, 1, m_showButton);

        treeWidget()->setColumnWidth(0, treeWidget()->width() - 30);
        treeWidget()->setColumnWidth(1, m_showButton->width());

        connect(m_showButton, SIGNAL(released()), SLOT(onShowButtonReleased()));

        m_spinSigIdx = 0;
        m_colorLabel = 0;
    }
    else
    {
        m_spinSigIdx = 0;
        m_colorLabel = 0;
    }
}

/**
 * @brief MRunTreeWidgetItem::onRunStatusChanged This slot is executed when
 * the calculation status of the observed mrun object changed
 */
void MRunTreeWidgetItem::onRunStatusChanged()
{
    if(!(vistype == DataItemTreeView::SE_GROUPS || vistype == DataItemTreeView::ATOOL || DataItemTreeView::EXP_DIAG))
        return;

    MRunCalculationStatus* state =m_mrun->calculationStatus();

    if(state->isError())
        setIcon(0, QIcon(Core::rootDir + "resources/icons/bullet_red.png"));
        //setTextColor(1,Qt::red);
    else if(state->isWarning() || state->isCancelled() || state->isPluginChanged())
        setIcon(0, QIcon(Core::rootDir + "resources/icons/bullet_orange.png"));
        //setTextColor(1,QColor("#ffb000"));
    else if(state->isSuccess())
        setIcon(0, QIcon(Core::rootDir + "resources/icons/bullet_green.png"));
        //setTextColor(1,Qt::green);
    else
        setIcon(0, QIcon(Core::rootDir + "resources/icons/bullet_white.png"));

    updateTooltip();
}


void MRunTreeWidgetItem::onShowButtonReleased()
{
    m_visualState = !m_visualState;

    if(m_visualState)
        m_showButton->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
    else
        m_showButton->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));

    emit visualStateChanged(this, m_visualState);
}



// ----------------------------------------
// Implementation of DataItemObserverObject
// ----------------------------------------

/**
 * @brief MRunTreeWidgetItem::onDataChanged This slot is executed when
 * the MRun::dataChanged( .. ) signal is emitted. This occures when
 * the MRun has been modified anywhere in the program.
 * @param pos position in datavector
 * @param value new value
 */
void MRunTreeWidgetItem::onDataChanged(int pos, QVariant value)
{
    m_mute = true;

    if(pos == 0)    // runname changed
        this->setText(0,value.toString());

    else if(pos == 1) // runcolor changed
    {
        if(m_colorLabel && m_mrun)
        {
            QColor curMrunCol = m_mrun->data(1).value<QColor>();
            QString style;
            style.sprintf("background-color: rgb(%d, %d, %d);",curMrunCol.red(),curMrunCol.green(),curMrunCol.blue());
            m_colorLabel->setStyleSheet(style);
        }
    }
    else if (pos == 2)// selected signal index changed
    {
        if(m_spinSigIdx && m_mrun)
        {
            int newSigIdx = m_mrun->data(2).toInt();
            if(newSigIdx >= 0 && newSigIdx < m_mrun->sizeAllMpoints())
            {
                m_spinSigIdx->setValue(newSigIdx+1);
            }
        }
    }
    else if(pos == 4) // liisettings have changed
    {
        updateTooltip();
    }
    m_mute = false;
}


/**
 * @brief MRunTreeWidgetItem::onDataChildInserted This slot is executed when
 * the MRun::childInserted(..) signal is emitted.
 * @param child_data new child
 */
void MRunTreeWidgetItem::onDataChildInserted(DataItem *child_data, int position)
{
    // nothing to do here
}


/**
 * @brief MRunTreeWidgetItem::onDataChildRemoved This slot is executed when
 * the MRun::childRemoved(..) signal is emitted.
 * @param child_data child which has been removed
 */
void MRunTreeWidgetItem::onDataChildRemoved(DataItem *child_data)
{
}


/**
 * @brief MRunTreeWidgetItem::onDataDestroyed This slot is executed when
 * the observed MRun has been deleted (implies self destruction of this
 * tree widget item).
 */
void MRunTreeWidgetItem::onDataDestroyed()
{
    m_mrun = 0;
    // qDebug() << "DataItemObserverWidget::onDataDestroyed " << data_id();

    // uncheck MRun if checked, notify listeners)
    if(checkState(0) == Qt::Checked)
    {
        setCheckState(0,Qt::Unchecked);
        emit checkStateChanged(this,false);
    }

    DataItemObserverObject::onDataDestroyed();

    // delete this row from the QTreeWidget
    this->deleteLater();
}


// -----------------------
// handle user interaction
// -----------------------


/**
 * @brief MRunTreeWidgetItem::handleTreeItemModified
 * This method is executed if the item has been edited
 * within the parent QTreeWidget.
 * @details If the m_mute flag is set to true it is assumed that
 * the tree item has been edited programatically.
 * Also see: DataItemTreeView::handleTreeItemChanged().
 * @param col
 */
void MRunTreeWidgetItem::handleTreeItemModified(int col)
{
    // ignore programatical calls: no user interaction!
    if(m_mute || !m_mrun)
        return;

    // update name in observed mrun
    if(col == 0)
        m_mrun->setName(this->text(0));
}


/**
 * @brief MRunTreeWidgetItem::deleteData This method is executed when the
 * "Delete" action is triggered from the right-click context menu
 * @details The MRun will be deleted. Also see: DataItemTreeView::onActionDelete().
 */
void MRunTreeWidgetItem::deleteData()
{
    // check if deletion is allowed
    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "cannot delete measurement run (active backgroud tasks!)";
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
        return;
    }

    // delete mrun
    delete DataItemObserverObject::data;
    m_mrun = 0; // pointer reset!
}


/**
 * @brief MRunTreeWidgetItem::handleDoubleClick is executed if a double
 * click has been performed on this tree item.
 * @details Also see: DataItemTreeView::onTreeItemDoubleClicked(..)).
 * @param col clicked column
 */
void MRunTreeWidgetItem::handleDoubleClick(int col)
{
    if(!m_mrun)
        return;

    // 2nd column: holds color label => open a nice color dialog
    if(col == 2)
    {
        QColor curColor = m_mrun->data(1).value<QColor>();

        QColor col = QColorDialog::getColor(curColor,0,"Select Measurement Run Color");

        if(col.isValid()){

            // save selected color to mrun
            m_mrun->setData(1,col);
        }
    }
}


/**
 * @brief MRunTreeWidgetItem::handleClick is executed when a single click
 * has been performed on the tree item.
 * @details Also see: DataItemTreeView::onTreeItemClicked(..)).
 * @param col clicked column
 */
void MRunTreeWidgetItem::handleClick(int col)
{
    if((col == 0 && m_spinSigIdx != 0) || vistype == DataItemTreeView::EXP_DIAG )
    {
        Qt::CheckState state = checkState(0);
        if(state == Qt::Checked)
        {
            setCheckState(0,Qt::Unchecked);
            emit checkStateChanged(this,false);
        }
        else
        {
           setCheckState(0,Qt::Checked);
            emit checkStateChanged(this,true);
        }
    }
}


/**
 * @brief MRunTreeWidgetItem::handleDrop This slot is executed when a
 * QDropEvent occures on the parent widget.
 * @details If a dropped item is a MRunTreeWidgetItem the item's MRun will
 * be inserted right after this MRun.
 * More drag'n'drop: See DataItemTreeWidget::dropEvent(..).
 * @param items list of other tree items which have been dropped on this item.
 * @param dropAbove drop items above this item (default false)
 */
void MRunTreeWidgetItem::handleDrop(QList<QTreeWidgetItem *> items, bool dropAbove)
{
    if(!m_mrun)
        return;

    // return if mrun is not stored in a group
    MRunGroup* g = Core::instance()->dataModel()->group(m_mrun->parentItem()->id());
    if(!g)
        return;

    // position of this mrun within the group
    int this_pos = m_mrun->position()+1;

    // iterate through all dropped items
    for(int i = 0; i < items.size(); i++)
    {

        QTreeWidgetItem* item = items[i];

        // read user role data
        int udat = item->data(0,Qt::UserRole).toInt();

        // handle MRunTreeWidgetItem drops
        if(udat == 2) // a MRun has been dropped
        {

            MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
            if(!mi)
                return;

            // get dropped mrun
            MRun* mrun = Core::instance()->dataModel()->mrun(mi->data_id());
            if(!mrun)
                return;

            if(dropAbove)
            {
                // if dropped run has same group as mrun ..
                if(mrun->parentItem()->id() == g->id())
                {
                    // remove dropped run, then reinsert it after mrun
                    g->removeChild(mrun);
                    g->insertChild(mrun,m_mrun->position());
                }
                else
                    g->insertChild(mrun,this_pos-1);
            }
            else // drop below this item (default)
            {
                // if dropped run has same group as mrun ..
                if(mrun->parentItem()->id() == g->id())
                {
                    // remove dropped run, then reinsert it after mrun
                    g->removeChild(mrun);
                    g->insertChild(mrun,m_mrun->position()+1);
                }
                else
                    g->insertChild(mrun,this_pos);
            }
            this_pos++;
        }
    }
}


/**
 * @brief MRunTreeWidgetItem::setShowMRun is used by
 * the parent MRunGroupTreeWidgetItem to un/check multiple MRunTreeWidgets
 * at once.
 * @param state new checked state
 */
void MRunTreeWidgetItem::setShowMRun(bool state)
{
    Qt::CheckState cstate = Qt::Unchecked;

    if(state)
        cstate = Qt::Checked;

    setCheckState(0,cstate);
    emit checkStateChanged(this,state);
}


// ---------------
// private helpers
// ---------------

/**
 * @brief MRunTreeWidgetItem::onImportFinished this slot updates the
 * maximum value of the signal selection spinbox.
 */
void MRunTreeWidgetItem::onImportFinished()
{
    if(m_spinSigIdx && m_mrun)
    {
        m_spinSigIdx->setMinimum(1);
        m_spinSigIdx->setMaximum(m_mrun->sizeAllMpoints());
    }
}


/**
 * @brief MRunTreeWidgetItem::onSigIdxChanged This slot is executed when the user has
 * changed the vaule selection of the signal index spinbox.
 * @param value new selection
 */
void MRunTreeWidgetItem::onSigIdxChanged(int value)
{
    if(!m_mute && !m_mrun)
        return;

    m_mrun->setData(2,value-1);
}


/**
 * @brief MRunTreeWidgetItem::updateTooltip updates tooltip information
 */
void MRunTreeWidgetItem::updateTooltip()
{
    if(!m_mrun)
        return;

    QString ttip = QString("%0\nLIISettings: %1\n\n%2")
            .arg(m_mrun->getName())
            .arg(m_mrun->liiSettings().name)
            .arg(m_mrun->calculationStatus()->toString());

    setToolTip(0,ttip);
}
