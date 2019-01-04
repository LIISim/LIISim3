#include "materialeditor.h"
#include "../../general/LIISimException.h"
#include "../../database/databasemanager.h"
#include <QHeaderView>
#include <QMessageBox>
#include "dbelementnamedialog.h"

MaterialEditor::MaterialEditor(QWidget *parent) :  DbEditorWidget(parent)
{
    db_data         = NULL;
    defaultFileName = "materials/newMaterial";
    defaultName     = "new Material";
    table           = new QTableWidget;    
    table_calc      = new QTableWidget;

    table_calc->setMaximumHeight(160);

    QLabel* lbl_calc = new QLabel("Calculated properties:");
    lbl_calc->setFixedHeight(15);

    lay_props->addWidget(table,      2, 0, 1, 5);
    lay_props->addWidget(lbl_calc,   3, 0, 1, 5);
    lay_props->addWidget(table_calc, 4, 0, 1, 5);

    _rowDataChanged = false;
    _blockInit = false;
    _emRowStart = 0;

    // setup table properties
    table->setColumnCount(15);

    table->setColumnWidth(0, 80); // name
    table->setColumnWidth(1, 80); // type

    int a;
    for(a = 2; a < 11; a++)
        table->setColumnWidth(a, 60);

    table->setColumnWidth(a++, 60);  // unit
    table->setColumnWidth(a++, 400); // equation
    table->setColumnWidth(a++, 300);
    table->setColumnWidth(a++, 20);

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
                << "Unit" << "Equation" << "Source" << "";
    table->setHorizontalHeaderLabels(columnNames);

    table->verticalHeader()->setVisible(false);
}


void MaterialEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getMaterials();
}


void MaterialEditor::initData()
{
    if(!_blockInit && !(_rowDataChanged = false))
        DbEditorWidget::initData();
}


void MaterialEditor::onSelectionChanged(const QItemSelection &selection)
{
    if(_rowDataChanged)
    {
        int ret = QMessageBox::warning(this, "LIISim3 Database Editor",
                  "The currently selected material entry has been modified.\n"
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
    updateCurrentView();
}

void MaterialEditor::updateCurrentView()
{
    if(db_data->size()==0)
        return;

    if(_rowDataChanged)
    {
        int ret = QMessageBox::warning(this, "LIISim3 Database Editor",
                  "The currently selected material entry has been modified.\n"
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

    Material* dbi = dbm->getMaterial(currentIndex);

    // set number of rows
    int no_props = 14 + dbi->Em.values.size() + 1;

    // reset tables
    table->clearContents();
    table->setRowCount(no_props);

    // table properties
    int k = 0;

    while(!_tableRows.isEmpty())
    {
        DBETableRowElement *row = _tableRows.takeFirst();
        disconnect(row, SIGNAL(dataChanged()), this, SLOT(onRowDataChanged()));
        delete row;
    }

    while(!_emTableRows.isEmpty())
    {
        DBETableRowElement *row = _emTableRows.takeFirst();
        disconnect(row, SIGNAL(dataChanged()), this, SLOT(onEmRowDataChanged()));
        disconnect(row, SIGNAL(deleteRequested()), this, SLOT(onEmRowDeleteRequest()));
        delete row;
    }

    QStringList types = Property::eqTypeList;
    types.removeLast();

    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->alpha_T_eff, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->C_p_mol, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->H_v, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->molar_mass, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->molar_mass_v, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->rho_p, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->theta_e, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->p_v, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->p_v_ref, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->T_v_ref, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->eps, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->Em_func, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->omega_p, types, k++));
    _tableRows.push_back(DBETableRowElement::buildTableRow(*table, dbi->tau, types, k++));

    //_emValues = QMap<int, Property>(dbi->Em.values);
    _emRowStart = k;

    _emValues.clear();
    for(auto &prop : dbi->Em.values)
        _emValues.push_back(prop.second);

    for(auto &prop : _emValues)
        _emTableRows.push_back(DBETableRowElement::buildTableRow(*table, prop, types, k++, true));

    for(auto row : _tableRows)
        connect(row, SIGNAL(dataChanged()), SLOT(onRowDataChanged()));

    for(auto row : _emTableRows)
        connect(row, SIGNAL(dataChanged()), SLOT(onEmRowDataChanged()));

    for(auto row : _emTableRows)
        connect(row, SIGNAL(deleteRequested()), SLOT(onEmRowDeleteRequest()));

    QToolButton *button = new QToolButton();
    button->setText("+ Em");

    table->setCellWidget(k++, 0, button);

    connect(button, SIGNAL(clicked(bool)), SLOT(onEmRowAddRequest()));

    updateCalculatedProperties();
}


void MaterialEditor::updateCalculatedProperties()
{
    Material* dbi = dbm->getMaterial(currentIndex);

    int no_calc = 6;

    table_calc->clear();

    // setup table calc
    table_calc->setColumnCount(4);
    table_calc->setColumnWidth(0, 180); // property
    table_calc->setColumnWidth(1, 100); // value
    table_calc->setColumnWidth(2, 60);  // unit
    table_calc->setColumnWidth(3, 400); // descriptions
    table_calc->setRowCount(no_calc);

    QStringList columnNames;
    columnNames << "Property" << "Value"
                << "Unit" << "Description";

    table_calc->setHorizontalHeaderLabels(columnNames);

    table_calc->verticalHeader()->setVisible(false);

    // font
    QFont ifont;
    ifont.setItalic(true);
    ifont.setBold(true);

    // table calculated properties (from Material::functions)
    int k = 0;
    QTableWidgetItem* item0;

    // if individual vapor pressure formula is given:
    if(dbi->p_v.available)
    {
        item0 = new QTableWidgetItem("vapor pressure is calculated by "
                                     "individual equation: p_v(T)");
    }
    // if Clausius-Clapeyeron reference values are given:
    else if(dbi->p_v_ref.available && dbi->T_v_ref.available)
    {
        QString desc_cc = QString("vapor pressure is calculated by "
                                  "Clausius-Clapeyron equation(T* = %0 K and p* = %1 Pa)"
                                  " using H_v(T)"
                                  ).arg(dbi->T_v_ref())
                                   .arg(dbi->p_v_ref());

        item0 = new QTableWidgetItem(desc_cc);
    }
    // no information
    else
    {
        item0 = new QTableWidgetItem("vapor pressure is not given (= 0.0 Pa)");
    }
    item0->setFlags(Qt::ItemIsEnabled);
    item0->setFont(ifont);
    table_calc->setItem(k, 0, item0);
    table_calc->setSpan(k, 0, 1, 4); // colspan = 4
    table_calc->setRowHeight(k, 20);
    k++;


    QTableWidgetItem* item1 = new QTableWidgetItem("vapor_pressure(293 K)");
    QTableWidgetItem* item2 = new QTableWidgetItem(QString("%0").arg(dbi->vapor_pressure(293.0)));
    QTableWidgetItem* item3 = new QTableWidgetItem("[Pa]");
    QTableWidgetItem* item4 = new QTableWidgetItem("Vapor pressure at 293 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("vapor_pressure(1500 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->vapor_pressure(1500.0)));
    item3 = new QTableWidgetItem("[Pa]");
    item4 = new QTableWidgetItem("Vapor pressure at 1500 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("vapor_pressure(3000 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->vapor_pressure(3000.0)));
    item3 = new QTableWidgetItem("[Pa]");
    item4 = new QTableWidgetItem("Vapor pressure at 3000 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("vapor_pressure(4000 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->vapor_pressure(4000.0)));
    item3 = new QTableWidgetItem("[Pa]");
    item4 = new QTableWidgetItem("Vapor pressure at 4000 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("vapor_pressure(5000 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->vapor_pressure(5000.0)));
    item3 = new QTableWidgetItem("[Pa]");
    item4 = new QTableWidgetItem("Vapor pressure at 5000 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;
}


void MaterialEditor::showPropertyInRow(const Property &prop, int row)
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

    //QLabel* eqw = dbm->eqList->defaultEq(prop.type);
    //table->setCellWidget(row, a++, eqw); // equation

    //table->setItem(row, a++, item4); // source

    //table->setRowHeight(row, eqw->size().height());
}


/**
 * @brief MaterialEditor::showOpticalPropertyInRow
 * @param prop
 * @param row (this parameter is changed during function call)
 */
void MaterialEditor::showOpticalPropertyInRow(const OpticalProperty &prop, int &row)
{
    for(varListConstIterator it = prop.values.begin(); it != prop.values.end(); it++)
    {
        showPropertyInRow(it->second, row);
        row++;
    }
}


void MaterialEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("no item selected. Add item to list first");
        return;
    }
    DbEditorWidget::onApplyChanges();

    Material* modMaterial = dbm->getMaterial(currentIndex);

    for(auto row : _tableRows)
        row->savePropertyParameters();

    for(auto row : _emTableRows)
        row->savePropertyParameters();

    modMaterial->Em.values.clear();

    for(auto em : _emValues)
        modMaterial->Em.values.insert(std::pair<int, Property>((int)em.parameter[0], em));

    QString oldfname = modMaterial->filename;
    QString newfname = leFname->text();

    try
    {
        for(int i=0;i<db_data->size();i++)
        {
            if(newfname != oldfname && db_data->at(i)->filename == newfname  )
                throw LIISimException("Invalid filename, file already exists: "+newfname);
        }

        modMaterial->filename = newfname.toLatin1().data();
        modMaterial->name = leName->text().toLatin1().data();
        modMaterial->description = leDescr->text().toLatin1().data();

        dbm->modifiedContent(modMaterial);
        QString msg = QString("Applied changes to: %0 (%1)").arg(modMaterial->name).arg(newfname);
        MSG_STATUS(msg);
        if(newfname != oldfname)
        {
            dbm->deleteFile(oldfname.toLatin1().data());
            msg.append(", deleted: "+oldfname);
            MSG_STATUS(msg);
        }

        //   update View;
        QModelIndex idx = listModel->index(currentIndex);
        listModel->setData(idx,leName->text());
    }
    catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        qDebug() << e.what();
    }
}


void MaterialEditor::onAddItemToList()
{
    DBElementNameDialog nameDialog(typeid(Material), this);
    nameDialog.setWindowTitle("Enter name for new material database entry");
    nameDialog.setDefault("DefaultMaterial");
    if(nameDialog.exec() == 1)
    {
        try
        {
            Material *m = new Material;
            m->name = nameDialog.getName();
            m->filename = nameDialog.getUniqueFilename(Core::instance()->generalSettings->databaseDirectory().toLatin1().data(), ".txt");

            dbm->addContentToDB(m);

            MSG_STATUS(QString("Database entry added: %0 (%1)").arg(m->name).arg(m->filename));
        }
        catch(LIISimException &e)
        {
            MSG_STATUS(e.what());
            MSG_ERR(e.what());
        }
    }
}


void MaterialEditor::onRemoveCurrentSelectionFromList()
{
    DbEditorWidget::onRemoveCurrentSelectionFromList();
}


void MaterialEditor::onRowDataChanged()
{
    DbEditorWidget::onValueEdited("");
    _rowDataChanged = true;
}


void MaterialEditor::onEmRowDataChanged()
{
    onRowDataChanged();
}


void MaterialEditor::onEmRowDeleteRequest()
{
    int idx = _emTableRows.indexOf(static_cast<DBETableRowElement*>(QObject::sender()));
    if(idx != -1)
    {
        for(auto row : _emTableRows)
            row->savePropertyParameters();

        DBETableRowElement *row = _emTableRows.takeAt(idx);
        table->removeRow(_emRowStart + idx);
        _emValues.removeAt(idx);

        delete row;
    }
    onRowDataChanged();
}


void MaterialEditor::onEmRowAddRequest()
{
    QStringList types = Property::eqTypeList;
    types.removeLast();

    table->insertRow(table->rowCount()-1);

    Property prop;
    prop.type = "optics_case";
    prop.name = "Em";
    prop.unit = "[-]";
    prop.source = "";
    prop.inFile = true;
    prop.available = true;

    _emValues.push_back(prop);
    _emTableRows.push_back(DBETableRowElement::buildTableRow(*table, _emValues.last(), types, table->rowCount()-2, true));

    connect(_emTableRows.last(), SIGNAL(dataChanged()), SLOT(onEmRowDataChanged()));
    connect(_emTableRows.last(), SIGNAL(deleteRequested()), SLOT(onEmRowDeleteRequest()));

    onRowDataChanged();
}
