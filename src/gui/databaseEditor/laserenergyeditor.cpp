#include "laserenergyeditor.h"
#include "../../general/LIISimException.h"
#include <QHeaderView>

LaserEnergyEditor::LaserEnergyEditor(QWidget *parent) : DbEditorWidget(parent)
{
    db_data = NULL;
    defaultFileName = "laserEnergy/newLaserEnergySettings";
    defaultName = "new Laser Energy Settings";

    table = new QTableWidget;
    lay_props->addWidget(table, 2, 0, 2, 5);
}


void LaserEnergyEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getLaserEnergy();
}


void LaserEnergyEditor::initData()
{
    DbEditorWidget::initData();
}


void LaserEnergyEditor::onSelectionChanged(const QItemSelection &selection)
{
    DbEditorWidget::onSelectionChanged(selection);

    LaserEnergy* le = dbm->getLaserEnergy(currentIndex);

    table->clear();

    table->setColumnCount(3);
    //table->setRowCount(le->lookupTable.size());
    table->setRowCount(le->table.size());
    QStringList columnNames;
    columnNames << "Set" << "Position" << "Energy";
    table->setHorizontalHeaderLabels(columnNames);
    table->verticalHeader()->setVisible(false);

    /*for(int i = 0; i < le->lookupTable.size(); i++)
    {
        QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(le->lookupTable.at(i).set));
        QTableWidgetItem* item2 = new QTableWidgetItem(QString::number(le->lookupTable.at(i).pos));
        QTableWidgetItem* item3 = new QTableWidgetItem(QString::number(le->lookupTable.at(i).energy));
        item1->setFlags(Qt::ItemIsEnabled);
        item2->setFlags(Qt::ItemIsEnabled);
        item3->setFlags(Qt::ItemIsEnabled);
        table->setItem(i, 0, item1);
        table->setItem(i, 1, item2);
        table->setItem(i, 2, item3);
    }*/

    int i = 0;
    for(QMap<double, QPair<double, double> >::iterator it = le->table.begin(); it != le->table.end(); ++it)
    {
        QTableWidgetItem* item1 = new QTableWidgetItem(this->locale().toString(it.value().first));
        QTableWidgetItem* item2 = new QTableWidgetItem(this->locale().toString(it.value().second));
        QTableWidgetItem* item3 = new QTableWidgetItem(this->locale().toString(it.key()));
        item1->setFlags(Qt::ItemIsEnabled);
        item2->setFlags(Qt::ItemIsEnabled);
        item3->setFlags(Qt::ItemIsEnabled);
        table->setItem(i, 0, item1);
        table->setItem(i, 1, item2);
        table->setItem(i, 2, item3);
        table->setRowHeight(i, 20);
        i++;
    }
}


void LaserEnergyEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("no item selected. add item to list first");
        return;
    }
    DbEditorWidget::onApplyChanges();

    LaserEnergy *laserEnergy = dbm->getLaserEnergy(currentIndex);

    QString oldFilename = laserEnergy->filename;
    QString newFilename = leFname->text();

    try
    {
        for(int i = 0; i < db_data->size(); i++)
        {
            if(newFilename != oldFilename && db_data->at(i)->filename == newFilename)
                throw LIISimException("Invalid filename, file already exists: "+newFilename);
        }
        laserEnergy->filename = newFilename.toLatin1().data();
        laserEnergy->name = leName->text().toLatin1().data();
        laserEnergy->description = leDescr->text().toLatin1().data();

        dbm->modifiedContent(laserEnergy);
        QString msg = "applied changes to: "+newFilename;
        MSG_STATUS(msg);
        if(newFilename != oldFilename)
        {
            dbm->deleteFile(oldFilename.toLatin1().data());
            msg.append(" deleted: "+oldFilename);
            MSG_STATUS(msg);
        }
        QModelIndex idx = listModel->index(currentIndex);
        listModel->setData(idx, leName->text());
    }
    catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        qDebug() << e.what();
    }
}


void LaserEnergyEditor::onAddItemToList()
{
    DbEditorWidget::onAddItemToList();
    QString filename = getValidDefaultFileName();

    try
    {
        LaserEnergy *le = new LaserEnergy;
        le->name = defaultName.toLatin1().data();
        le->filename = filename.toLatin1().data();

        dbm->addContentToDB(le);
        MSG_STATUS("added: "+filename);
    }
    catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        butAdd->setEnabled(true);
    }
    butAdd->setEnabled(true);
}


void LaserEnergyEditor::onRemoveCurrentSelectionFromList()
{
    DbEditorWidget::onRemoveCurrentSelectionFromList();
}
