#include "gasmixeditor.h"
#include "../../general/LIISimException.h"
#include "../../database/databasemanager.h"
#include <QHeaderView>

//#include "externalLibraries/mathml/qwt_mathml_text_engine.h"
//#include "externalLibraries/mathml/qwt_mml_document.h"
//#include <QPainter>
//#include <qevent.h>

#include <gui/utils/mathml/equationlist.h>

GasMixEditor::GasMixEditor(QWidget *parent) :   DbEditorWidget(parent)
{
    db_data = NULL;
    defaultFileName = "gasmixtures/newGasMixture";
    defaultName = "new Gas Mixture";
    table_gases = new QTableWidget;
    table_props = new QTableWidget;
    table_calc = new QTableWidget;

    table_props->setWordWrap(true);


    table_gases->setMaximumHeight(100);
    table_props->setMaximumHeight(150);

    QLabel* lbl_props = new QLabel("Properties (manually set):");
    lbl_props->setFixedHeight(15);

    QLabel* lbl_calc = new QLabel("Calculated properties:");
    lbl_calc->setFixedHeight(15);

    lay_props->addWidget(table_gases, 2, 0, 1, 5);
    lay_props->addWidget(lbl_props,   3, 0, 1, 5);
    lay_props->addWidget(table_props, 4, 0, 1, 5);
    lay_props->addWidget(lbl_calc,   5, 0, 1, 5);
    lay_props->addWidget(table_calc,  6, 0, 1, 5);
}


void GasMixEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getGasMixtures();
}


void GasMixEditor::initData()
{
    DbEditorWidget::initData();
}


void GasMixEditor::onSelectionChanged(const QItemSelection &selection)
{
    DbEditorWidget::onSelectionChanged(selection);
    updateCurrentView();
}


void GasMixEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("No item selected. Add item to list first");
        return;
    }
    DbEditorWidget::onApplyChanges();

    GasMixture *modmix = dbm->getGasMixture(currentIndex);

    QString oldfname = modmix->filename;
    QString newfname = leFname->text();

    try{
        for(int i=0;i<db_data->size();i++)
        {
            if(newfname != oldfname && db_data->at(i)->filename == newfname  )
                throw LIISimException("Invalid filename, file already exists: "+newfname);
        }

        modmix->filename = newfname.toLatin1().data();
        modmix->name = leName->text().toLatin1().data();
        modmix->description = leDescr->text().toLatin1().data();

        qDebug() << modmix->filename << " " << modmix->name;
        dbm->modifiedContent(modmix);
        QString msg = " applied changes to: "+newfname;
        MSG_STATUS(msg);

        if(newfname != oldfname){
            dbm->deleteFile(oldfname.toLatin1().data());
            msg.append(" deleted: "+oldfname);
            MSG_STATUS(msg);
        }

        //   update View;
        QModelIndex idx = listModel->index(currentIndex);
        listModel->setData(idx,leName->text());

    }catch(LIISimException e)
    {

        MSG_STATUS(e.what());
        qDebug() << e.what();
    }
}


void GasMixEditor::onAddItemToList()
{
    DbEditorWidget::onAddItemToList();

    QString fname = getValidDefaultFileName();


    try{
         GasMixture* g = new GasMixture;
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

    butAdd->setEnabled(true);
}


void GasMixEditor::onRemoveCurrentSelectionFromList()
{
    DbEditorWidget::onRemoveCurrentSelectionFromList();
}



void GasMixEditor::showPropertyInRow(const Property& prop, int row)
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

    table_props->setItem(row, 0, item1); // name
    table_props->setItem(row, 1, item2); // type

    int a;
    for(a = 2; a < 11; a++)
    {
        QString val = QString("%0").arg(prop.parameter[a-2]);

        QTableWidgetItem* item_a = new QTableWidgetItem(val);
        item_a->setToolTip(prop.description);
        item_a->setFlags(Qt::ItemIsEnabled);
        item_a->setToolTip(val);

        table_props->setItem(row, a, item_a); // a_i
    }

    table_props->setItem(row, a++, item3); // unit

    QLabel* eqw = dbm->eqList->defaultEq(prop.type);
    table_props->setCellWidget(row, a++, eqw); // equation

    table_props->setItem(row, a++, item4); // source

    table_props->setRowHeight(row, eqw->size().height());
}



/**
 * @brief update view (for current gasmixture)
 * @details called if the current GasMixture selection or the DatabaseContent has changed.
 */
void GasMixEditor::updateCurrentView()
{
    if(db_data->size()==0)
        return;

    GasMixture* dbi = dbm->getGasMixture(currentIndex);


    // number of table rows
    int no_gases = dbi->getNoGases();
    int no_props = 3;
    int no_calc = 9;

    // reset tables
    table_gases->clear();
    table_props->clear();
    table_calc->clear();

    // setup table gases
    table_gases->setColumnCount(3);
    table_gases->setRowCount(no_gases);

    table_gases->setColumnWidth(0, 180);
    table_gases->setColumnWidth(1, 100);
    table_gases->setColumnWidth(2, 60);

    QStringList columnNames;
    columnNames << "Gas" << "Fraction" << "Unit";
    table_gases->setHorizontalHeaderLabels(columnNames);

    table_gases->verticalHeader()->setVisible(false);


    // setup table props
    table_props->setColumnCount(14);

    table_props->setColumnWidth(0, 80);
    table_props->setColumnWidth(1, 50);

    int a;
    for(a = 2; a < 11; a++)
        table_props->setColumnWidth(a, 60);

    table_props->setColumnWidth(a++, 60);  // unit
    table_props->setColumnWidth(a++, 400); // equation
    table_props->setColumnWidth(a++, 400);
    table_props->setRowCount(no_props);

    columnNames.clear();
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
    table_props->setHorizontalHeaderLabels(columnNames);

    table_props->verticalHeader()->setVisible(false);


    // setup table calc
    table_calc->setColumnCount(4);
    table_calc->setColumnWidth(0, 180); // property
    table_calc->setColumnWidth(1, 100); // value
    table_calc->setColumnWidth(2, 60);  // unit
    table_calc->setColumnWidth(3, 400); // descriptions
    table_calc->setRowCount(no_calc);

    columnNames.clear();
    columnNames << "Property" << "Value"
                << "Unit" << "Description";
    table_calc->setHorizontalHeaderLabels(columnNames);

    table_calc->verticalHeader()->setVisible(false);


    // table gases
    for(int i = 0; i< no_gases; i++)
    {
        QTableWidgetItem* item1 = new QTableWidgetItem(dbi->getGas(i)->name);
        QTableWidgetItem* item2 = new QTableWidgetItem(QString("%0").arg(dbi->getX(i)));
        QTableWidgetItem* item3 = new QTableWidgetItem("-");

        item1->setFlags(Qt::ItemIsEnabled);
        item2->setFlags(Qt::ItemIsEnabled);
        item3->setFlags(Qt::ItemIsEnabled);        
        table_gases->setItem(i,0,item1);
        table_gases->setItem(i,1,item2);
        table_gases->setItem(i,2,item3);
        table_gases->setRowHeight(i,20);
    }


    // table properaties
    int k = 0;

    showPropertyInRow(dbi->L, k++);
    showPropertyInRow(dbi->gamma_eqn, k++);
    showPropertyInRow(dbi->therm_cond, k++);


    // table calculated properties (from GasMixture::functions)
    k = 0;

    QTableWidgetItem* item1 = new QTableWidgetItem("molar_mass");
    QTableWidgetItem* item2 = new QTableWidgetItem(QString("%0").arg(dbi->molar_mass));
    QTableWidgetItem* item3 = new QTableWidgetItem("[kg/mol]");
    QTableWidgetItem* item4 = new QTableWidgetItem("Molar mass of gas mixture");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("c_tg(293 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->c_tg(293.0)));
    item3 = new QTableWidgetItem("[m/s]");
    item4 = new QTableWidgetItem("Thermal velocity of gas molecules at 293 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("c_tg(1500 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->c_tg(1500.0)));
    item3 = new QTableWidgetItem("[m/s]");
    item4 = new QTableWidgetItem("Thermal velocity of gas molecules at 1500 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("C_p_mol(293 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->C_p_mol(293.0)));
    item3 = new QTableWidgetItem("[J/mol/K]");
    item4 = new QTableWidgetItem("Molar heat capacity at 293 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("C_p_mol(1500 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->C_p_mol(1500.0)));
    item3 = new QTableWidgetItem("[J/mol/K]");
    item4 = new QTableWidgetItem("Molar heat capacity at 1500 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("c_p_kg(293 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->c_p_kg(293.0)));
    item3 = new QTableWidgetItem("[J/kg/K]");
    item4 = new QTableWidgetItem("Specific heat capacity at 293 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("c_p_kg(1500 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->c_p_kg(1500.0)));
    item3 = new QTableWidgetItem("[J/kg/K]");
    item4 = new QTableWidgetItem("Specific heat capacity at 1500 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;


    QString desc_gamma;

    if(dbi->gamma_eqn.available)
        desc_gamma = "gamma(T) = gamma_eqn(T)";
    else
        desc_gamma = "gamma(T) = Cp(T) / (Cp(T)− R)";

    item1 = new QTableWidgetItem("gamma(293 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->gamma(293.0)));
    item3 = new QTableWidgetItem("[-]");
    item4 = new QTableWidgetItem("Heat-capacity ratio: " + desc_gamma + " at 293 K");
    item1->setFlags(Qt::ItemIsEnabled);
    item2->setFlags(Qt::ItemIsEnabled);
    item3->setFlags(Qt::ItemIsEnabled);
    table_calc->setItem(k, 0, item1);
    table_calc->setItem(k, 1, item2);
    table_calc->setItem(k, 2, item3);
    table_calc->setItem(k, 3, item4);
    table_calc->setRowHeight(k, 20);
    k++;

    item1 = new QTableWidgetItem("gamma(1500 K)");
    item2 = new QTableWidgetItem(QString("%0").arg(dbi->gamma(1500.0)));
    item3 = new QTableWidgetItem("[-]");
    item4 = new QTableWidgetItem("Heat-capacity ratio: " + desc_gamma + " at 1500 K");
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
