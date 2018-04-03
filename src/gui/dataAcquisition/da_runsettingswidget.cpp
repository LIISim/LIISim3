#include "da_runsettingswidget.h"

#include <QHeaderView>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include "core.h"
#include "signal/mrungroup.h"

DA_RunSettingsWidget::DA_RunSettingsWidget(QWidget *parent) : QTableWidget(parent)
{
    lastGroup = -2;
    lastGroupFound = false;
    udpsLoaded = false;

    setRowCount(7);
    setColumnCount(2);

    setShowGrid(false);

    for(int i = 0; i < rowCount(); i++)
            setRowHeight(i,18);

    setColumnWidth(0,150);
    setMinimumHeight(300);

    verticalHeader()->setVisible(false);
    QStringList mt_horizontalHeaders;
    mt_horizontalHeaders << "Run Settings" << " " << " ";
    setHorizontalHeaderLabels(mt_horizontalHeaders);

    leDescription = new QTextEdit(this);
    cbFilter = new LIIFilterComboBox(this);
    cbMRunGroup = new QComboBox(this);

    lecw = new LaserEnergyControllerWidget(this);
    lemw = new LaserEnergyMeasurementWidget(this);

    setItem(0, 0, new QTableWidgetItem("Description"));
    item(0, 0)->setFlags(Qt::ItemIsEnabled);
    setCellWidget(0, 1, leDescription);
    setRowHeight(0, 75);
    setColumnWidth(1, 300);
    //item(0, 0)->setTextAlignment(Qt::AlignTop || Qt::AlignLeft);

    setItem(1, 0, new QTableWidgetItem("Filter"));
    item(1, 0)->setFlags(Qt::ItemIsEnabled);
    setCellWidget(1, 1, cbFilter);

    setItem(2, 0, new QTableWidgetItem("MRun Group"));
    item(2, 0)->setFlags(Qt::ItemIsEnabled);
    setCellWidget(2, 1, cbMRunGroup);

    setItem(3, 0, new QTableWidgetItem(""));
    item(3, 0)->setFlags(Qt::ItemIsEnabled);
    setItem(3, 1, new QTableWidgetItem(""));
    item(3, 1)->setFlags(Qt::ItemIsEnabled);

    setItem(4, 0, new QTableWidgetItem("Laser Fluence"));
    item(4, 0)->setFlags(Qt::ItemIsEnabled);
    setCellWidget(4, 1, lecw);

    setItem(5, 0, new QTableWidgetItem("Measured Laser Fluence"));
    item(5, 0)->setFlags(Qt::ItemIsEnabled);
    setCellWidget(5, 1, lemw);

    setItem(6, 0, new QTableWidgetItem("User Defined Parameters"));
    udpmw2 = new UserDefinedParameterMasterWidget(this);
    setCellWidget(6, 1, udpmw2);

    onGroupsChanged();

    connect(Core::instance()->dataModel(), SIGNAL(groupsChanged()), SLOT(onGroupsChanged()));
    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));
    connect(udpmw2->buttonAdd, SIGNAL(clicked(bool)), SLOT(onAddUDPClicked()));
    connect(udpmw2->buttonClear, SIGNAL(clicked(bool)), SLOT(onClearUDPClicked()));
    connect(leDescription, SIGNAL(textChanged()), SLOT(onDescriptionTextChanged()));
    connect(cbMRunGroup, SIGNAL(currentIndexChanged(int)), SLOT(onGroupIndexChanged(int)));
}


DA_RunSettingsWidget::~DA_RunSettingsWidget()
{

}


double DA_RunSettingsWidget::getLaserFluence()
{
    return lecw->getLaserFluence();
}


QMap<QString, QVariant> DA_RunSettingsWidget::getParameterList()
{
    QMap<QString, QVariant> map;
    for(int i = 0; i < parameterWidgets.size(); i++)
    {
        if(parameterWidgets.at(i)->isValid())
            map.insert(parameterWidgets.at(i)->getIdentifier(), parameterWidgets.at(i)->getValue());
    }
    return map;
}


void DA_RunSettingsWidget::onGroupsChanged()
{
    cbMRunGroup->blockSignals(true);
    QList<QPair<QString,int>> groups = Core::instance()->dataModel()->sortedGroupNameList();
    int default_id = Core::instance()->dataModel()->defaultGroup()->id();

    int current_id;
    if(lastGroup != -2 && !lastGroupFound)
        current_id = lastGroup;
    else
        current_id = cbMRunGroup->currentData().toInt();
    int current_index = 0;

    cbMRunGroup->clear();

    cbMRunGroup->addItem("Create new group", -1);
    cbMRunGroup->addItem("Ungrouped", default_id);

    for(int i = 0; i < groups.size(); i++)
    {
        if(groups.at(i).second == current_id)
        {
            current_index = cbMRunGroup->count();
            if(!lastGroupFound)
            {
                lastGroup = -2;
                lastGroupFound = true;
            }
        }
        if(groups.at(i).second != default_id)
            cbMRunGroup->addItem(groups.at(i).first, groups.at(i).second);
    }
    cbMRunGroup->blockSignals(false);
    cbMRunGroup->setCurrentIndex(current_index);
}


void DA_RunSettingsWidget::onAddUDPClicked()
{
    parameterWidgets.push_back(new UserDefinedParameterWidget(this));
    setRowCount(rowCount() + 1);
    QWidget *widget = new QWidget();
    widget->setLayout(new QHBoxLayout);
    widget->layout()->setMargin(0);
    widget->layout()->addWidget(parameterWidgets.back());
    setCellWidget(rowCount() - 1, 1, widget);
    connect(parameterWidgets.back(), SIGNAL(removeClicked(UserDefinedParameterWidget*)), SLOT(onRemoveUDPClicked(UserDefinedParameterWidget*)));
    connect(parameterWidgets.back(), SIGNAL(valuesChanged()), SLOT(onUDPDataChanged()));
}


void DA_RunSettingsWidget::onClearUDPClicked()
{
    while(!parameterWidgets.isEmpty())
    {
        delete parameterWidgets.back();
        parameterWidgets.removeLast();
    }
    setRowCount(7);

    saveUDPs();
}


void DA_RunSettingsWidget::onRemoveUDPClicked(UserDefinedParameterWidget *widget)
{
    int index = parameterWidgets.indexOf(widget);
    if(index != -1)
    {
        parameterWidgets.removeAt(index);
        delete widget;

        setRowCount(rowCount() - 1);

        for(int i = 0; i < parameterWidgets.size(); i++)
        {
            QWidget *widget = new QWidget();
            widget->setLayout(new QHBoxLayout);
            widget->layout()->setMargin(0);
            widget->layout()->addWidget(parameterWidgets.at(i));
            setCellWidget(7 + i, 1, widget);
        }
    }

    saveUDPs();
}


void DA_RunSettingsWidget::onDescriptionTextChanged()
{
    Core::instance()->guiSettings->setValue("da_runsettings", "description", leDescription->toPlainText());
}


void DA_RunSettingsWidget::onGroupIndexChanged(int index)
{
    Core::instance()->guiSettings->setValue("da_runsettings", "lastGroup", cbMRunGroup->currentData().toInt());
}


void DA_RunSettingsWidget::onGUISettingsChanged()
{
    leDescription->blockSignals(true);
    leDescription->setText(Core::instance()->guiSettings->value("da_runsettings", "description").toString());
    leDescription->blockSignals(false);

    lastGroup = Core::instance()->guiSettings->value("da_runsettings", "lastGroup", -2).toInt();

    loadUDPs();
}

void DA_RunSettingsWidget::saveUDPs()
{
    GuiSettings *settings = Core::instance()->guiSettings;
    settings->setValue("runsettings", "udp_count", parameterWidgets.size());

    for(int i = 0; i < parameterWidgets.size(); i++)
    {
        QString identifier_id = QString("udp_%0_identifier").arg(i);
        QString value_id = QString("udp_%0_value").arg(i);
        settings->setValue("runsettings", identifier_id, parameterWidgets.at(i)->getIdentifier());
        settings->setValue("runsettings", value_id, parameterWidgets.at(i)->getValue());
    }
}


void DA_RunSettingsWidget::loadUDPs()
{
    if(!udpsLoaded)
    {
        GuiSettings * settings = Core::instance()->guiSettings;
        int udpCount = settings->value("runsettings", "udp_count", "0").toInt();

        for(int i = 0; i < udpCount; i++)
        {
            QString identifier_id = QString("udp_%0_identifier").arg(i);
            QString value_id = QString("udp_%0_value").arg(i);
            QString identifier = settings->value("runsettings", identifier_id).toString();
            QString value = settings->value("runsettings", value_id).toString();

            parameterWidgets.push_back(new UserDefinedParameterWidget(this));
            parameterWidgets.back()->setIdentifier(identifier);
            parameterWidgets.back()->setValue(value);
            setRowCount(rowCount() + 1);
            QWidget *widget = new QWidget();
            widget->setLayout(new QHBoxLayout);
            widget->layout()->setMargin(0);
            widget->layout()->addWidget(parameterWidgets.back());
            setCellWidget(rowCount() - 1, 1, widget);
            connect(parameterWidgets.back(), SIGNAL(removeClicked(UserDefinedParameterWidget*)), SLOT(onRemoveUDPClicked(UserDefinedParameterWidget*)));
            connect(parameterWidgets.back(), SIGNAL(valuesChanged()), SLOT(onUDPDataChanged()));
        }

        udpsLoaded = true;
    }
}


void DA_RunSettingsWidget::onUDPDataChanged()
{
    saveUDPs();
}

