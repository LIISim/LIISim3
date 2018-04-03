#include "gaseditor.h"
#include "../../general/LIISimException.h"
#include "../../database/databasemanager.h"
#include <QMessageBox>
#include <QHeaderView>

GasEditor::GasEditor(QWidget *parent) :  DbEditorWidget(parent)
{
    db_data = NULL;
    defaultFileName = "gases/newGas";
    defaultName = "new Gas";

    table = new QTableWidget;
    lay_props->addWidget(table,2,0,2,5);
}

void GasEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getGases();

}

void GasEditor::initData()
{
    DbEditorWidget::initData();
}

void GasEditor::onSelectionChanged(const QItemSelection &selection)
{
    DbEditorWidget::onSelectionChanged(selection);

    GasProperties* dbi = dbm->getGas(currentIndex);

    // set number of rows
    int no_props = 4;

    table->clear();

    // setup table
    table->setColumnCount(14);
    table->setRowCount(no_props);

    table->setColumnWidth(0, 80); // name
    table->setColumnWidth(1, 80); // type

    int a;
    for(a = 2; a < 11; a++)
        table->setColumnWidth(a, 60);

    table->setColumnWidth(a++, 60);  // unit
    table->setColumnWidth(a++, 400); // equation
    table->setColumnWidth(a++, 300);


    QStringList columnNames;
    columnNames << "Property"
                << "Type"
                << QString("a%0").arg(QChar(0x2080))
                << QString("a%0").arg(QChar(0x2081))
                << QString("a%0").arg(QChar(0x2082))
                << QString("a%0").arg(QChar(0x2083))
                << QString("a%0").arg(QChar(0x2084))
                << QString("a%0").arg(QChar(0x2085))
                << QString("a%0").arg(QChar(0x2086))
                << QString("a%0").arg(QChar(0x2087))
                << QString("a%0").arg(QChar(0x2088))                
                << "Unit" << "Equation" << "Source";
    table->setHorizontalHeaderLabels(columnNames);

    table->verticalHeader()->setVisible(false);

    int k = 0;

    showPropertyInRow(dbi->alpha_T, k++);
    showPropertyInRow(dbi->C_p_mol, k++);
    showPropertyInRow(dbi->molar_mass, k++);
    showPropertyInRow(dbi->zeta, k++);

}


void GasEditor::showPropertyInRow(const Property &prop, int row)
{
    QTableWidgetItem* item1 = new QTableWidgetItem(prop.name);
    QTableWidgetItem* item2 = new QTableWidgetItem(prop.type);
    QTableWidgetItem* item3 = new QTableWidgetItem(prop.unit);
    QTableWidgetItem* item4 = new QTableWidgetItem(prop.source);

    item1->setToolTip(prop.description);
    item2->setToolTip(prop.description);
    item3->setToolTip(prop.description);
    item4->setToolTip(prop.source);

    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    item4->setFlags(Qt::ItemIsEnabled);

    if(prop.inFile == false)
    {
        QColor tcolor;

        if(prop.optional)
        {
            item4->setText("Warning: (optional) variable was not defined in file");
            tcolor = QColor(255,165,0);
        }
        else
        {
            item4->setText("Error: variable was not defined in file");
            tcolor = QColor(Qt::red);
        }

        item1->setTextColor(tcolor);
        item2->setTextColor(tcolor);
        item4->setTextColor(tcolor);
    }
    else if(prop.available == false)
    {
        item4->setText("Warning: type and values are not set (not used during calculation)");

        item1->setTextColor(QColor(255,165,0));
        item2->setTextColor(QColor(255,165,0));
        item4->setTextColor(QColor(255,165,0));
    }

    table->setItem(row, 0, item1); // name
    table->setItem(row, 1, item2); // type

    int a;
    for(a = 2; a < 11; a++)
    {
        QString val = QString("%0").arg(prop.parameter[a-2]);

        QTableWidgetItem* item_a = new QTableWidgetItem(val);
        item_a->setToolTip(prop.description);
        item_a->setFlags(Qt::ItemIsEnabled);
        item_a->setToolTip(val);

        table->setItem(row, a, item_a); // a_i
    }

    table->setItem(row, a++, item3); // unit

    QLabel* eqw = dbm->eqList->defaultEq(prop.type);
    table->setCellWidget(row, a++, eqw); // equation

    table->setItem(row, a++, item4); // source

    table->setRowHeight(row, eqw->size().height());
}


void GasEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("no item selected. Add item to list first");
        return;
    }
    DbEditorWidget::onApplyChanges();


    GasProperties* modgas = dbm->getGas(currentIndex);

    QString oldfname = modgas->filename;
    QString newfname = leFname->text();



    try
    {
        for(int i=0;i<db_data->size();i++)
        {
            if(newfname != oldfname && db_data->at(i)->filename == newfname  )
                throw LIISimException("Invalid filename, file already exists: "+newfname);
        }

        modgas->filename = newfname.toLatin1().data();
        modgas->name = leName->text().toLatin1().data();
        modgas->description = leDescr->text().toLatin1().data();
        qDebug() << modgas->filename << " " << modgas->name;
        dbm->modifiedContent(modgas);
        QString msg = "apllied changes to: "+newfname;
        MSG_STATUS(msg);

        if(newfname != oldfname){
            dbm->deleteFile(oldfname.toLatin1().data());
            msg.append(" deleted: "+oldfname);
            MSG_STATUS(msg);
        }

        //   update View;
        QModelIndex idx = listModel->index(currentIndex);
        listModel->setData(idx,leName->text());
        emit signal_gasesUpdated();
    }catch(LIISimException e)
    {

        MSG_STATUS(e.what());
        qDebug() << e.what();
    }
}


void GasEditor::onAddItemToList()
{

    DbEditorWidget::onAddItemToList();

    // get filename
    QString fname = getValidDefaultFileName();
  //  QString fname = defaultFileName+".txt";

    try{
         GasProperties* g = new GasProperties;
         g->name = defaultName.toLatin1().data();
         g->filename =  fname.toLatin1().data();

         dbm->addContentToDB(g);
         MSG_STATUS("added: "+fname);

    }catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        qDebug() << e.what();
        butAdd->setEnabled(true);
    }

    emit signal_gasesUpdated();
    butAdd->setEnabled(true);
}

void GasEditor::onRemoveCurrentSelectionFromList()
{
    if(db_data == NULL)return;
    if(db_data->size() <= 0)
    {
        butRemove->setEnabled(true);
        return;
    }

    // check if the gas is used in any mixtures !!!
    GasProperties* curGas = dbm->getGas(currentIndex);
    if(curGas->no_mixture_references > 0)
    {
        QMessageBox msgBox;
        QString val;
        val.sprintf("%d",curGas->no_mixture_references);
        msgBox.setText(curGas->name+" is used in "+val+" Gas Mixtures!");
        msgBox.setInformativeText("Do you really want to remove this gas?\n(gas will also be deleted from all mixtures)");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int choice = msgBox.exec();

        if(choice == QMessageBox::No)
            return;
    }
    DbEditorWidget::onRemoveCurrentSelectionFromList();

    emit signal_gasesUpdated();
}
