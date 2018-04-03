#ifndef MRUNTREEWIDGETITEM_H
#define MRUNTREEWIDGETITEM_H

#include <QWidget>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSpinBox>
#include <QLabel>
#include <QToolButton>

#include "dataitemtreeview.h"
#include "../../models/dataitemobserverobject.h"
#include "../../signal/mrun.h"

/**
 * @brief The MRunTreeWidgetItem is an Item shown in a QTreeWidget for the
 * representation of a MRun.
 *
 * @ingroup GUI
 * @details Therefore it implements QTreeWidgetItem
 * DataItemObserverObject. MRunTreeWidgetItem listens to changes of the
 * observed MRun (eg. the MRunTreeWidgetItem removes itself when the observed
 * MRun is deleted). Furthermore it implements Handlers for
 * certain user interactions (also see DataItemTreeView).
 *
 * Roles for QTreeWidgetItem::data(int column,int role):
 *
 * - column: alwas zero
 * - Qt::UserRole:  holds 2 (used by DataItemTreeView to identify MRunTreeWidgetItem)
 * - Qt::UserRole + 1: holds DataItem::id() of MRun object
 *
 */
class MRunTreeWidgetItem : public DataItemObserverObject, public QTreeWidgetItem
{
    Q_OBJECT

    DataItemTreeView::VisType vistype;
public:
    explicit MRunTreeWidgetItem(DataItemTreeView::VisType vistype,MRun* mrun,
            QTreeWidgetItem* parentItem = 0);

    ~MRunTreeWidgetItem();
    void setupItemWidgets();

private:

    /**
     * @brief m_mrun Pointer to observed MRun (useful to avoid continous casting)
     */
    MRun* m_mrun;

    /**
     * @brief m_mute This flag should be set if the QTreeWidgetItem is modified
     * programmatically
     */
    bool m_mute;

    /**
     * @brief m_spinSigIdx holds the "selected signal index". This Widget
     * is vsible only if the column count of the QTreeWidget is > 1 (eg Analysistools).
     */
    QSpinBox* m_spinSigIdx;

    /**
     * @brief m_colorLabel is used for showing and editing the MRun's color property.
     * This Widget is vsible only if the column count of the QTreeWidget is > 1 (eg Analysistools).
     */
    QLabel* m_colorLabel;

    QToolButton *m_showButton;
    bool m_visualState;

    void updateTooltip();

signals:

    /**
     * @brief checkStateChanged this signal is emitted if the items checked state has changed
     * @param item pointer to item
     * @param state new state
     */
    void checkStateChanged(MRunTreeWidgetItem* item, bool state);

    void visualStateChanged(MRunTreeWidgetItem* item, bool state);

protected slots:

    // --------------------------------
    // Overwrite DataItemObserverWidget
    // --------------------------------

    void onDataChanged(int pos, QVariant value);
    void onDataChildInserted( DataItem* child_data, int position);
    void onDataChildRemoved( DataItem* child_data);
    void onDataDestroyed();

public slots:

    // -----------------------------
    // Handlers for user interaction
    // -----------------------------

    void handleDrop(QList<QTreeWidgetItem*> items, bool dropAbove = false);
    void handleTreeItemModified(int col);
    void handleDoubleClick(int col);
    void handleClick(int col);
    void deleteData();
    void setShowMRun(bool state);


private slots:

    void onImportFinished();
    void onSigIdxChanged(int value);    
    void onRunStatusChanged();
    void onShowButtonReleased();

};

#endif // MRUNTREEWIDGETITEM_H
