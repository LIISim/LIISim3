#ifndef DATAITEMTREEWIDGET_H
#define DATAITEMTREEWIDGET_H

#include <QTreeWidget>

/**
 * @brief The DataItemTreeWidget class provides a tree view
 * that uses a predefined tree model. It extends the
 * standard QTreewidget and customizes some event
 * handlers of its parent class.
 */
class DataItemTreeWidget : public QTreeWidget
{
    Q_OBJECT

    QRect dropSite;
    bool isDragging;
public:
    explicit DataItemTreeWidget(QWidget *parent = 0)
        : QTreeWidget(parent),
          isDragging(false){}
public slots:
    virtual void dropEvent(QDropEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent * event);
    void paintEvent(QPaintEvent* e);
};

#endif // DATAITEMTREEWIDGET_H
