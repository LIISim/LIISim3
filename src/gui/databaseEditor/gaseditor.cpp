#include "gaseditor.h"
#include "../../general/LIISimException.h"
#include "../../database/databasemanager.h"
#include <QMessageBox>
#include <QHeaderView>

#include "dbelementnamedialog.h"

GasEditor::GasEditor(QWidget *parent) :  DbEditorWidget(parent), _rowDataChanged(false), _blockInit(false)
{
    db_data = NULL;
    defaultFileName = "gases/newGas";
    defaultName = "new Gas";

    table = new QTableWidget;
    lay_props->addWidget(table,2,0,2,5);

    table->setColumnCount(14);
    table->setRowCount(4);

    table->setColumnWidth(0, 80); // name
    table->setColumnWidth(1, 90); // type

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
}

void GasEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getGases();
}

void GasEditor::initData()
{
    if(!_blockInit && !(_rowDataChanged = false))
        DbEditorWidget::initData();
}

void GasEditor::onSelectionChanged(const QItemSelection &selection)
{
    if(_rowDataChanged)
    {
        int ret = QMessageBox::warning(this, "LIISim3 Database Editor",
                  "The currently selected gas entry has been modified.\n"
                  "Do you want to apply your changes?",
                  QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard);

        if(ret == QMessageBox::Save)
        {
            _blockInit = true;
            onApplyChanges();
            _blockInit = false;
        }
        _rowDataChanged = false;
    }

    DbEditorWidget::onSelectionChanged(selection);

    updateView();
}


void GasEditor::updateView()
{
    GasProperties* dbi = dbm->getGas(currentIndex);

    table->clearContents();

    int k = 0;

    while(!_tableRows.isEmpty())
    {
        DBETableRowElement *row = _tableRows.takeFirst();
        disconnect(row, SIGNAL(dataChanged()), this, SLOT(onRowDataChanged()));
        delete row;
    }

    QStringList types = Property::eqTypeList;
    types.removeLast();

    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->alpha_T, types, k++));
    connect(_tableRows.last(), SIGNAL(dataChanged()), SLOT(onRowDataChanged()));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->C_p_mol, types, k++));
    connect(_tableRows.last(), SIGNAL(dataChanged()), SLOT(onRowDataChanged()));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->molar_mass, types, k++));
    connect(_tableRows.last(), SIGNAL(dataChanged()), SLOT(onRowDataChanged()));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->zeta, types, k++));
    connect(_tableRows.last(), SIGNAL(dataChanged()), SLOT(onRowDataChanged()));
}


void GasEditor::onRowDataChanged()
{
    DbEditorWidget::onValueEdited("");
    _rowDataChanged = true;
}


void GasEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("no item selected. Add item to list first");
        return;
    }
    DbEditorWidget::onApplyChanges();

    for(int i = 0; i < _tableRows.size(); i++)
        _tableRows.at(i)->savePropertyParameters();

    GasProperties* modgas = dbm->getGas(currentIndex);

    QString oldfname = modgas->filename;
    QString newfname = leFname->text();

    try
    {
        for(int i=0;i<db_data->size();i++)
        {
            if(newfname != oldfname && db_data->at(i)->filename == newfname)
                throw LIISimException("Invalid filename, file already exists: "+newfname);
        }

        modgas->filename = newfname.toLatin1().data();
        modgas->name = leName->text().toLatin1().data();
        modgas->description = leDescr->text().toLatin1().data();
        qDebug() << modgas->filename << " " << modgas->name;
        dbm->modifiedContent(modgas);
        QString msg = QString("Applied changes to: %0 (%1)").arg(modgas->name).arg(modgas->filename);
        MSG_STATUS(msg);

        if(newfname != oldfname){
            dbm->deleteFile(oldfname.toLatin1().data());
            msg.append(" deleted: "+oldfname);
            MSG_STATUS(msg);
        }

        //update view
        QModelIndex idx = listModel->index(currentIndex);
        listModel->setData(idx,leName->text());
        emit signal_gasesUpdated();
    }
    catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        qDebug() << e.what();
    }
}


void GasEditor::onAddItemToList()
{
    DBElementNameDialog nameDialog(typeid(GasProperties), this);
    nameDialog.setWindowTitle("Enter name for new gas database entry");
    nameDialog.setDefault("DefaultGas");
    if(nameDialog.exec() == 1)
    {
        try
        {
            GasProperties *gp = new GasProperties;
            gp->name = nameDialog.getName();
            gp->filename = nameDialog.getUniqueFilename(Core::instance()->generalSettings->databaseDirectory().toLatin1().data(), ".txt");

            dbm->addContentToDB(gp);

            MSG_STATUS(QString("Database entry added: %0").arg(gp->name));
        }
        catch(LIISimException &e)
        {
            MSG_STATUS(e.what());
            MSG_ERR(e.what());
        }

        emit signal_gasesUpdated();
    }
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
