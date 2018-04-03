#ifndef PROCESSINGPLUGINTREEWIDGETITEM_H
#define PROCESSINGPLUGINTREEWIDGETITEM_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

#include "../../models/dataitemobserverobject.h"
#include "../../signal/processing/processingplugin.h"
#include "../../utils/plugininputfield.h"


/**
 * @brief The PPTreeWidgetItem class. TODO: DOCUMENTATION
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class PPTreeWidgetItem : public DataItemObserverObject, public QTreeWidgetItem
{
    Q_OBJECT

public:

    explicit PPTreeWidgetItem(ProcessingPlugin* plugin, QTreeWidgetItem* parentItem = 0);
    explicit PPTreeWidgetItem( ProcessingPlugin* plugin, QTreeWidget* view);

private:

    bool m_mute;
    bool m_isTempCalc;
    bool m_isDummy;
    ProcessingPlugin* m_plugin;
    QList<PluginInputField*> m_inputFields;

    void init();
    void initParams();
    void reloadParams();

signals:

public slots:

    void handleDrop(QList<QTreeWidgetItem*> items, bool dropBefore = false);
    void handleTreeItemModified(int col);
    void handleClick(int col);

    void deleteData();
    void setupItemWidgets();

protected slots:

    void onDataChanged(int pos, QVariant value);
    void onDataChildInserted( DataItem* child_data, int position);
    void onDataChildRemoved( DataItem* child_data);
    void onDataDestroyed();

private slots:

    void handleDirtyStateChanged(bool state);

    void updateNameLabel();
    void updateLinkStateLabel();
    void updatePlotVisibilityIcon();

    void onFieldParameterChanged();

};

#endif // PROCESSINGPLUGINTREEWIDGETITEM_H
