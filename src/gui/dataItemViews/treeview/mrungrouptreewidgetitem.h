#ifndef MRUNGROUPTREEWIDGETITEM_H
#define MRUNGROUPTREEWIDGETITEM_H

#include <QWidget>

#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "dataitemtreeview.h"
#include "mruntreewidgetitem.h"


#include "../../models/dataitemobserverobject.h"
#include "../../signal/mrungroup.h"


/**
 * @brief The MRunGroupTWI class. TODO: DOCUMENTATION
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class MRunGroupTWI : public DataItemObserverObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    explicit MRunGroupTWI(
            DataItemTreeView::VisType vistype,
            MRunGroup* group,
            QTreeWidget* view = 0);

    ~MRunGroupTWI();

    void onDataChanged(int pos, QVariant value);
    void onDataChildInserted( DataItem* child_data, int position);
    void onDataChildRemoved( DataItem* child_data);
    void onDataDestroyed();

    QList<QTreeWidgetItem*> checkedItems(){return m_checkedItems;}

    QList<QTreeWidgetItem*> visualEnabledItems(){return m_visualEnabledItems;}

    void setCheckedItems(QList<int> & itemDIs);

private:

    MRunGroup* m_group;
    DataItemTreeView::VisType vistype;
    bool m_mute;

    bool m_toggleShowAllRuns;

    QList<QTreeWidgetItem*> m_checkedItems;

    QList<QTreeWidgetItem*> m_visualEnabledItems;

signals:

    /**
     * @brief checkedItemsChanged This signal is emitted if the checked state of
     * one of the groups children has been changed
     */
    void checkedItemsChanged();

    void visualEnabledItemsChanged();

protected slots:

public slots:

    void handleDrop(QList<QTreeWidgetItem*> items, bool dropAbove = false);
    void handleTreeItemModified(int col);
    void handleDoubleClick(int col);
    void handleClick(int col);
    void deleteData();

private slots:

    void onMrunTWICheckedStateChanged(MRunTreeWidgetItem* item,bool state);

    void onVisualStateItemChanged(MRunTreeWidgetItem* item, bool state);

};

#endif // MRUNGROUPTREEWIDGETITEM_H
