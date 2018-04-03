#ifndef MRUNGROUPTREEVIEW_H
#define MRUNGROUPTREEVIEW_H


#include "../../models/dataitemobserverwidget.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QKeyEvent>
#include <QAction> 

#include "dataitemtreewidget.h"

class MRunGroupTWI;
class MRunTreeWidgetItem;


/**
 * @brief The DataItemTreeView class represents an Item based Treeview.
 * It is specialized for the use with DataItems and implements the
 * DataItemObserverWidget interface  for the choosen root item.
 * @ingroup GUI
 */
class DataItemTreeView : public DataItemObserverWidget
{
    Q_OBJECT
public:


    enum VisType { ATOOL, SE_PSTEPS, SE_GROUPS, EXP_DIAG, DA_GROUPS, DA_VC_GROUPS};

    explicit DataItemTreeView(VisType visType,DataItem* root =0, QWidget *parent = 0);

    VisType visType;

    void setDataRoot(DataItem* root, bool cleanup = true);

    void addTopLevelItem(QTreeWidgetItem* item);
    void setPermanentDummyItem(QTreeWidgetItem* dummy);
    int indexOfTopLevelItem(QTreeWidgetItem* item);
    int topLevelItemCount();
    QList<bool> topLevelItemExpandedStates();
    void setTopLevelItemExpandedStates(QList<bool> states);
    void setColumnCount(int c);
    void setIndentation(int i);


    inline QTreeWidget* treeWidget(){return m_treeWidget;}

    QTreeWidgetItem* topLevelItem(int index);

    void clearAll();
    void clearDummy();
    void expandAll();

    void setHeaderLabel(const QString & label);
    void setHeaderLabels(const QStringList &labels);

    inline void setColumnWidth(int col, int width){m_treeWidget->setColumnWidth(col,width);}

    void setMultiSelectionMode(bool enabled);

private:

    // ACTIONS FOR BASIC FUNCTIONS
    QAction* acDelete;
    QAction* acAdd;

    // GUI ELEMENTS
    QVBoxLayout* layVbox;
    DataItemTreeWidget* m_treeWidget;
    QTreeWidgetItem* m_dummy;

    bool m_mute_checkedItemsChanged;

    //Elements for MRun-property-dependent selection
    class SelectionContainer
    {
    public:
        QList<QVariant> liisettings;
        QList<QVariant> filter;
        QList<double> gainChannel1;
        QList<double> gainChannel2;
        QList<double> gainChannel3;
        QList<double> gainChannel4;
    };

    QMenu *menuDepSelection;
    QMenu *menuDepLIISettingsSelection;
    QMenu *menuDepFilterSelection;
    QMenu *menuDepGainVoltageSelection;
    QMenu *menuDepGainChannel1Selection;
    QMenu *menuDepGainChannel2Selection;
    QMenu *menuDepGainChannel3Selection;
    QMenu *menuDepGainChannel4Selection;

    QPoint selectionPoint;

    bool multiSelectionMode;
    bool mapCleanup;

    QMap<int, SelectionContainer> dependantSelectionMap;

    void mapToSelection(QTreeWidgetItem *item);


signals:

    /**
     * @brief selectionChanged This signal is emitted if processign step selection has changed
     * @param selection
     */
    void selectionChanged(QList<QTreeWidgetItem*> selection);


    /**
     * @brief checkedItemsChanged This signal is emitted if the list of checked items has been changed
     * @param checkedItems
     */
    void checkedItemsChanged(QList<QTreeWidgetItem*> checkedItems);

    void visualEnabledItemsChanged(QList<QTreeWidgetItem*> visualEnabledItems);


    void treeItemModified(QTreeWidgetItem* item, int col);

    /**
     * @brief keyPressed This signal is emitted if any KeyEvent has been
     * detected within this widget. This allows parent Widgets to handle
     * a KeyEvent, which has been placed on this widget.
     * @param event
     */
    void keyPressed(QKeyEvent* event);

    void keyReleased(QKeyEvent* event);

public slots:

    // HANDLE KEYPRESS
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    void handleKeyPress(QKeyEvent *event);

    void resizeEvent ( QResizeEvent * event );

    void setCheckedRunIDs(QList<int>& itemIDs);

protected slots:

    // IMPLEMENT THE DataItemObserverWidget

    void onDataChildInserted( DataItem* child_data, int position);
    void onDataChildRemoved( DataItem* child_data);
    void onDataDestroyed();

private slots:

    // HANDLE USER INTERACTION

    void onSelectionChanged();
    void onContextMenuRequested(const QPoint & point);
    void handleTreeItemChanged(QTreeWidgetItem* item,int col);
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int col);
    void onTreeItemClicked(QTreeWidgetItem* item, int col);

    void onCheckedMRunsChanged();
    void onVisualStateChanged();

    // ACTION HANDLERS
    void onActionDelete();
    void onAcAdd();
    void onContextMenuSetLIISettings(QAction* action);

    void onContextMenuDependantSelectorLIISettings(QAction* action);
    void onContextMenuDependantSelectorFilter(QAction *action);
    void onContextMenuDependantSelectorGainVoltage(QAction *action);

    void onContextMenuSetGroupLIISettings(QAction *action);

    void onItemExpanded(QModelIndex modelIndex){resizeEvent(0);}
};

#endif // MRUNGROUPTREEVIEW_H
