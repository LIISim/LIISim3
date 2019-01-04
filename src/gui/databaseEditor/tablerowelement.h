#ifndef TABLEROWELEMENT_H
#define TABLEROWELEMENT_H

#include <QObject>
#include <QTableWidget>

#include "database/structure/property.h"

class DBETableRowElement : public QObject
{
    Q_OBJECT
public:
    static DBETableRowElement* buildTableRow(QTableWidget &tableWidget, Property &prop, QStringList allowedTypes, int row, bool deletable = false);

    void savePropertyParameters();
    const Property& getProperty();

private:
    DBETableRowElement(QTableWidget &tableWidget, Property &prop, QStringList allowedTypes, int row, bool deletable);

    QList<QTableWidgetItem*> _activeTableItems;
    QTableWidgetItem *_itemSource;
    QTableWidgetItem *_itemName;

    QTableWidget &_tableWidget;
    Property &_prop;
    int _row;
    QString _propType;

    static int defaultHeight;

    bool _errorSourceSet;

    static QString _textErrorNotSet;
    static QString _textErrorOptional;
    static QString _textErrorUndefined;

private slots:
    void onComboboxTypeIndexChanged(QString text);
    void onButtonDeleteClicked();

public slots:
    void onTableItemChanged(QTableWidgetItem *item);

signals:
    void dataChanged();
    void deleteRequested();

};

#endif // TABLEROWELEMENT_H
