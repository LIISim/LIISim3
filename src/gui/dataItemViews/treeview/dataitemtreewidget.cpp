#include "dataitemtreewidget.h"

#include <QPainter>

#include "mrungrouptreewidgetitem.h"
#include "mruntreewidgetitem.h"
#include "processingplugintreewidgetitem.h"

/**
 * @brief DataItemTreeWidget::paintEvent overwrites
 * QTreeWidget::paintEvent(). Draws a customized dotted line
 * when an item is dragged through the view.
 * @param e
 */
void DataItemTreeWidget::paintEvent(QPaintEvent *e)
{
    QTreeWidget::paintEvent(e);

    // redraw the drop indication line
    if(isDragging)
    {
        QPainter painter(viewport());

        int x, y, w, h;
        dropSite.getRect ( &x, &y, &w, &h );

        QPoint point(x,y);
        QModelIndex modidx = indexAt ( point );
        QRect arect = visualRect ( modidx );
        int b = arect.y();

        // add row height if indicator position is below dropitem
        if(dropIndicatorPosition() == BelowItem)
        {
            b += this->rowHeight(modidx);
        }
        QBrush brush(Qt::black, Qt::Dense4Pattern);
        QPen pen;
        pen.setWidth(2);
        pen.setBrush(brush);
        painter.setPen(pen);
        painter.drawLine ( 0, b, width(), b );
        e->accept();
    }
}

/**
 * @brief DataItemTreeWidget::dropEvent overwrites
 * QTreeWidget::dropEvent(). Calls custom drop-handlers
 * for tree items.
 * @param event
 */
void DataItemTreeWidget::dropEvent(QDropEvent *event)
{
    isDragging = false;
    event->pos();
    QTreeWidgetItem* item = this->itemAt(event->pos());


    if(item)
    {
        // qDebug() << "DITW.dropEvent" <<item->data(0,Qt::UserRole).toInt();
        QList<QTreeWidgetItem*> sourceItems = this->selectedItems();

        if(sourceItems.isEmpty())
            sourceItems.append(this->currentItem());

        int userData = item->data(0,Qt::UserRole).toInt();
        if(userData == 1)       // mrungroup
        {
            MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);

            // check if items are dropped above or below the "drop to" item
            if(dropIndicatorPosition() == AboveItem)
                gi->handleDrop(sourceItems,true);
            else
                gi->handleDrop(sourceItems);
        }
        else if(userData == 2)  // mrun
        {
            MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);

            // check if items are dropped above or below the "drop to" item
            if(dropIndicatorPosition() == AboveItem)
                mi->handleDrop(sourceItems,true);
            else
                mi->handleDrop(sourceItems);

        }
        else if(userData == 4)  // processingplugin
        {
            PPTreeWidgetItem* pi = dynamic_cast<PPTreeWidgetItem*>(item);

            // check if items are dropped above or below the "drop to" item
            if(dropIndicatorPosition() == AboveItem)
                pi->handleDrop(sourceItems,true);
            else
                pi->handleDrop(sourceItems);
        }
    }
}


/**
 * @brief DataItemTreeWidget::dragEnterEvent overwrites
 * QTreeWidget::dragEnterEvent(). Accepts drag event dependent
 * on tree item type.
 * @param e
 */
void DataItemTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
    //qDebug() << "DITW.dragEvent";
    QTreeWidgetItem* item = this->itemAt(e->pos());

    if(item )
    {
        int userData = item->data(0,Qt::UserRole).toInt();
        if(userData == 1)       // mrungroup
        {
            e->setAccepted(true);
        }
        else if(userData == 2)  // mrun
        {
            e->setAccepted(true);
        }
        else if(userData == 4)  // processingplugin
        {
         //   PPTreeWidgetItem* pi = dynamic_cast<PPTreeWidgetItem*>(item);
         //   qDebug() << "dragEnter  " << pi->isDraggable();
          e->setAccepted(true);
        }
    }
    else  {

        e->setAccepted(false);
    }

    QTreeWidget::dragEnterEvent(e);
    // e->dropAction()
}

/**
 * @brief DataItemTreeWidget::dragMoveEvent overwrites
 * QTreeWidget::dragMoveEvent(). Keeps track of position and
 * status of drag move event.
 * @param event
 */
void DataItemTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
    dropSite = event->answerRect(); //save drop site
    QTreeWidget::dragMoveEvent(event);
    isDragging = true;
}


