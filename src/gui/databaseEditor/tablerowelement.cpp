#include "tablerowelement.h"

#include <QComboBox>
#include <QLabel>

#include "core.h"

int DBETableRowElement::defaultHeight = 26;

QString DBETableRowElement::_textErrorNotSet = "Warning: type and values are not set (not used during calculation)";
QString DBETableRowElement::_textErrorOptional = "Warning: (optional) variable was not defined in file";
QString DBETableRowElement::_textErrorUndefined = "Error: variable was not defined in file";

DBETableRowElement* DBETableRowElement::buildTableRow(QTableWidget &tableWidget, Property &prop, QStringList allowedTypes, int row, bool deletable)
{
    return new DBETableRowElement(tableWidget, prop, allowedTypes, row, deletable);
}


DBETableRowElement::DBETableRowElement(QTableWidget &tableWidget, Property &prop, QStringList allowedTypes, int row, bool deletable) :
    QObject(), _tableWidget(tableWidget), _prop(prop), _row(row), _propType(prop.type)
{
    _itemName = new QTableWidgetItem(prop.name);
    _itemSource = new QTableWidgetItem(prop.source);
    QTableWidgetItem *itemUnit = new QTableWidgetItem(prop.unit);

    QComboBox *comboboxType = new QComboBox();
    comboboxType->addItems(allowedTypes);
    int idx = comboboxType->findText(_propType);
    if(idx != -1)
        comboboxType->setCurrentIndex(idx);
    else
    {
        comboboxType->addItem(_propType);
        comboboxType->setCurrentIndex(comboboxType->count() - 1);
    }

    _itemName->setToolTip(prop.description);
    comboboxType->setToolTip(prop.description);
    itemUnit->setToolTip(prop.description);
    _itemSource->setToolTip(prop.description);

    /*if(prop.inFile == false)
    {
        QColor tcolor;

        if(prop.optional)
        {
            _itemSource->setText(_textErrorOptional);
            tcolor = QColor(255,165,0);
        }
        else
        {
            _itemSource->setText(_textErrorUndefined);
            tcolor = QColor(Qt::red);
        }

        _itemName->setTextColor(tcolor);
        _itemSource->setTextColor(tcolor);
        _errorSourceSet = true;
    }
    else if(prop.available == false)
    {
        _itemSource->setText(_textErrorNotSet);

        _itemName->setTextColor(QColor(255,165,0));
        _itemSource->setTextColor(QColor(255,165,0));
        _errorSourceSet = true;
    }
    else
    {
        _itemName->setTextColor(QColor(Qt::black));
        _itemSource->setTextColor(QColor(Qt::black));
        _errorSourceSet = false;
    }*/

    _tableWidget.setItem(_row, 0, _itemName);
    _tableWidget.setCellWidget(_row, 1, comboboxType);

    _tableWidget.setItem(_row, 11, itemUnit);
    _tableWidget.setItem(_row, 13, _itemSource);

    if(deletable)
    {
        QToolButton *button = new QToolButton();
        button->setText("X");
        connect(button, SIGNAL(clicked(bool)), SLOT(onButtonDeleteClicked()));
        _tableWidget.setCellWidget(_row, 14, button);
    }

    onComboboxTypeIndexChanged(_propType);

    connect(comboboxType, SIGNAL(currentTextChanged(QString)), SLOT(onComboboxTypeIndexChanged(QString)));
    connect(&_tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(onTableItemChanged(QTableWidgetItem*)));
}


void DBETableRowElement::savePropertyParameters()
{
    for(int i = 0; i < _activeTableItems.size(); i++)
        _prop.parameter[_activeTableItems.at(i)->column()-2] = _activeTableItems.at(i)->text().toDouble();

    _prop.type = _propType;
    if(_propType == "notSet")
        _prop.available = false;
    else
    {
        _prop.available = true;
        _prop.source = _itemSource->text();
    }
}


const Property& DBETableRowElement::getProperty()
{
    return _prop;
}


void DBETableRowElement::onComboboxTypeIndexChanged(QString text)
{
    QLabel *eqw = Core::instance()->getDatabaseManager()->eqList->defaultEq(text);
    _tableWidget.setCellWidget(_row, 12, eqw);
    if(defaultHeight < eqw->pixmap()->height())
        _tableWidget.setRowHeight(_row, eqw->pixmap()->height());
    else
        _tableWidget.setRowHeight(_row, defaultHeight);

    //_prop.type = text;
    _propType = text;

    _activeTableItems.clear();

    int inputs_enabled = 0;

    if(text == "const")
        inputs_enabled = 1;
    else if(text == "case")
        inputs_enabled = 3;
    else if(text == "poly")
        inputs_enabled = 9;
    else if(text == "poly2")
        inputs_enabled = 6;
    else if(text == "polycase")
        inputs_enabled = 9;
    else if(text == "exp")
        inputs_enabled = 5;
    else if(text == "exppoly")
        inputs_enabled = 8;
    else if(text == "powx")
        inputs_enabled = 6;
    else if(text == "optics_case")
        inputs_enabled = 4;
    else if(text == "optics_exp")
        inputs_enabled = 2;
    else if(text == "optics_lambda")
        inputs_enabled = 9;
    else if(text == "optics_temp")
        inputs_enabled = 5;

    int index = 0;

    for(; index < inputs_enabled && index < 9; index++)
    {
        QString val = QString("%0").arg(_prop.parameter[index]);

        QTableWidgetItem *item = new QTableWidgetItem(val);
        item->setToolTip(_prop.description);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        item->setToolTip(val);

        _tableWidget.setItem(_row, index + 2, item);
        _activeTableItems.push_back(item);
    }

    for(; index < 9; index++)
    {
        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setFlags(0);
        _tableWidget.setItem(_row, index + 2, item);
    }

    if(text == "notSet")
    {
        _itemSource->setText(_textErrorNotSet);
        _itemSource->setFlags(Qt::ItemIsEnabled);

        _itemName->setTextColor(QColor(255,165,0));
        _itemSource->setTextColor(QColor(255,165,0));
        _errorSourceSet = true;
    }
    else
    {
        _itemSource->setText(_prop.source);
        _itemSource->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

        _itemName->setTextColor(QColor(Qt::black));
        _itemSource->setTextColor(QColor(Qt::black));
        _errorSourceSet = false;
    }

    emit dataChanged();
}


void DBETableRowElement::onButtonDeleteClicked()
{
    emit deleteRequested();
}


void DBETableRowElement::onTableItemChanged(QTableWidgetItem *item)
{
    if(_activeTableItems.contains(item))
    {
        QRegExp regExp("[^0-9.]+");
        if(item->text().contains(regExp))
        {
            QBrush brush(QColor(Qt::red), Qt::Dense5Pattern);
            item->setBackground(brush);
            item->setToolTip("Error: Only numerical values are allowed");
        }
        else
        {
            QBrush brush;
            item->setBackground(brush);
            item->setToolTip(item->text());

            emit dataChanged();
        }
    }
    else if(_itemSource == item)
    {
        emit dataChanged();
    }
}
