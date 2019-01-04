#ifndef GASMIXTUREROW_H
#define GASMIXTUREROW_H

#include <QTableWidget>
#include <QToolButton>
#include <QComboBox>

#include "database/structure/gasmixture.h"

class DBEGasMixtureRow : public QObject
{
    Q_OBJECT
public:
    DBEGasMixtureRow(QTableWidget &tableWidget, GasMixture* mixture);

    void saveParameters();

private:
    void updateTable();

    QTableWidget &_tableWidget;
    GasMixture *_mixture;

    QList<QTableWidgetItem*> _activeTableItems;
    QList<QToolButton*> _activeRemoveButtons;
    QList<QComboBox*> _gasComboboxes;

    QList<QPair<QString, double>> _mixture_gases;

private slots:
    void onComboboxGasIndexChanged(QString text);

    void onButtonAddGasClicked();

    void onButtonRemoveGasClicked();

public slots:
    void onTableItemChanged(QTableWidgetItem *item);

signals:
    void dataChanged();
};

#endif // GASMIXTUREROW_H
