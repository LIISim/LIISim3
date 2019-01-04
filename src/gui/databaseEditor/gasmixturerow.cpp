#include "gasmixturerow.h"

#include "core.h"


DBEGasMixtureRow::DBEGasMixtureRow(QTableWidget &tableWidget, GasMixture* mixture) : QObject(), _tableWidget(tableWidget), _mixture(mixture)
{
    for(int i = 0; i < _mixture->getNoGases(); i++)
        _mixture_gases.push_back(QPair<QString, double>(_mixture->getGas(i)->name, _mixture->getX(i)));

    updateTable();

    connect(&_tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(onTableItemChanged(QTableWidgetItem*)));
}


void DBEGasMixtureRow::saveParameters() //TODO: error messages
{
    _mixture->clearAllGases();

    QList<DatabaseContent*> *gases = Core::instance()->getDatabaseManager()->getGases();

    for(int i = 0; i < _mixture_gases.size(); i++)
    {
        GasProperties *gas = nullptr;
        for(int j = 0; j < gases->size(); j++)
            if(gases->at(j)->name == _mixture_gases.at(i).first)
                gas = dynamic_cast<GasProperties*>(gases->at(j));

        if(gas)
        {
            qDebug() << "saveParameters" << gas->name << _mixture_gases.at(i).second;
            _mixture->addGas(gas, _mixture_gases.at(i).second);
        }
        else
            qDebug() << "saveParameters gas not found";
    }
}


void DBEGasMixtureRow::updateTable()
{
    while(!_gasComboboxes.isEmpty())
    {
        QComboBox *cb = _gasComboboxes.takeFirst();
        disconnect(cb, SIGNAL(currentTextChanged(QString)), this, SLOT(onComboboxGasIndexChanged(QString)));
    }

    while(!_activeRemoveButtons.isEmpty())
    {
        QToolButton *button = _activeRemoveButtons.takeFirst();
        disconnect(button, SIGNAL(clicked(bool)), this, SLOT(onButtonRemoveGasClicked()));
    }

    _activeTableItems.clear();

    _tableWidget.clearContents();

    _tableWidget.setRowCount(_mixture_gases.size()+1);

    QList<DatabaseContent*> *gases = Core::instance()->getDatabaseManager()->getGases();

    QStringList gasNames;

    for(int i = 0; i < gases->size(); i++)
        gasNames.push_back(gases->at(i)->name);

    for(int i = 0; i < _mixture_gases.size(); i++)
    {
        QComboBox *cbGas = new QComboBox;
        cbGas->addItems(gasNames);
        int idx = cbGas->findText(_mixture_gases.at(i).first);
        if(idx != -1)
            cbGas->setCurrentIndex(idx);

        QTableWidgetItem* item2 = new QTableWidgetItem(QString("%0").arg(_mixture_gases.at(i).second));
        QTableWidgetItem* item3 = new QTableWidgetItem("-");
        item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        item3->setFlags(Qt::ItemIsEnabled);

        QToolButton *button = new QToolButton();
        button->setText("X");

        _tableWidget.setCellWidget(i, 0, cbGas);
        _tableWidget.setItem(i, 1, item2);
        _tableWidget.setItem(i, 2, item3);
        _tableWidget.setRowHeight(i, 20);
        _tableWidget.setCellWidget(i, 3, button);

        _activeTableItems.push_back(item2);
        _gasComboboxes.push_back(cbGas);
        _activeRemoveButtons.push_back(button);

        connect(button, SIGNAL(clicked(bool)), SLOT(onButtonRemoveGasClicked()));
        connect(cbGas, SIGNAL(currentTextChanged(QString)), SLOT(onComboboxGasIndexChanged(QString)));
    }

    QToolButton *button = new QToolButton();
    button->setText("+");
    connect(button, SIGNAL(clicked(bool)), SLOT(onButtonAddGasClicked()));

    _tableWidget.setCellWidget(_mixture_gases.size(), 0, button);
    _tableWidget.setRowHeight(_mixture_gases.size(), 20);
}


void DBEGasMixtureRow::onComboboxGasIndexChanged(QString text)
{
    int idx = _gasComboboxes.indexOf(static_cast<QComboBox*>(QObject::sender()));
    if(idx != -1)
    {
        _mixture_gases.replace(idx, QPair<QString, double>(_gasComboboxes.at(idx)->currentText(), _mixture_gases.at(idx).second));
    }
    emit dataChanged();
}


void DBEGasMixtureRow::onButtonAddGasClicked()
{
    _mixture_gases.push_back(QPair<QString, double>(Core::instance()->getDatabaseManager()->getGases()->first()->name, 0.0f));
    updateTable();
    emit dataChanged();
}


void DBEGasMixtureRow::onButtonRemoveGasClicked()
{
    int idx = _activeRemoveButtons.indexOf(static_cast<QToolButton*>(QObject::sender()));
    if(idx != -1)
    {
        _mixture_gases.removeAt(idx);
    }
    updateTable();
    emit dataChanged();
}


void DBEGasMixtureRow::onTableItemChanged(QTableWidgetItem *item)
{
    int idx = _activeTableItems.indexOf(item);
    if(idx != -1)
    {
        QRegExp regExp("[^0-9.]+");
        if(item->text().contains(regExp))
        {
            QBrush brush(QColor(Qt::red), Qt::Dense5Pattern);
            item->setBackground(brush);
            item->setToolTip("Error: Only numerical values are allowed");
            return;
        }

        double sum = 0;
        for(auto *item : _activeTableItems)
            sum += item->text().toDouble();

        if(sum > 1.0f)
        {
            QBrush brush(QColor(Qt::red), Qt::Dense5Pattern);
            for(auto *item : _activeTableItems)
            {
                item->setBackground(brush);
                item->setToolTip("Error: Sum of fraction values is > 1.0");
            }
            return;
        }

        QBrush brush;
        for(auto *item : _activeTableItems)
        {
            item->setBackground(brush);
            item->setToolTip(item->text());
        }

        _mixture_gases.replace(idx, QPair<QString, double>(_mixture_gases.at(idx).first, item->text().toDouble()));

        emit dataChanged();
    }
}
