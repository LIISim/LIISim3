#include "transmissioneditor.h"
#include "../../general/LIISimException.h"
#include <QHeaderView>
#include <qwt_symbol.h>


TransmissionEditor::TransmissionEditor(QWidget *parent) : DbEditorWidget(parent)
{
    db_data = NULL;
    defaultFileName = "transmissions/newTransmission";
    defaultName = "new Transmission";

    table = new QTableWidget;
    basePlot = new BasePlotWidgetQwt(this);

    QWidget *widget = new QWidget;
    widget->setLayout(new QHBoxLayout);
    widget->layout()->setMargin(0);
    widget->layout()->addWidget(table);
    widget->layout()->addWidget(basePlot);
    basePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    basePlot->setPlotAxisTitles("Wavelength / nm", "Transmission / %");

    lay_props->addWidget(widget, 2, 0, 2, 5);
}


void TransmissionEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getTransmissions();
}


void TransmissionEditor::initData()
{
    DbEditorWidget::initData();
}


void TransmissionEditor::onSelectionChanged(const QItemSelection &selection)
{
    DbEditorWidget::onSelectionChanged(selection);

    TransmissionDBE *trans = dbm->getTransmission(currentIndex);

    table->clear();
    basePlot->detachAllCurves();

    table->setColumnCount(2);
    table->setRowCount(trans->spectrum.xData.size() + 1);
    QStringList columnNames;
    columnNames << "" << "";
    table->setHorizontalHeaderLabels(columnNames);
    table->verticalHeader()->setVisible(false);

    table->setItem(0, 0, new QTableWidgetItem("Group"));
    table->item(0, 0)->setFlags(Qt::ItemIsEnabled);
    QString group;
    switch(trans->group)
    {
    case TransmissionDBE::BANDPASS: group = "Bandpass"; break;
    case TransmissionDBE::DICHROIC: group = "Dichroic"; break;
    case TransmissionDBE::ND: group = "ND"; break;
    case TransmissionDBE::OTHER: group = "Other"; break;
    }
    table->setItem(0, 1, new QTableWidgetItem(group));
    table->item(0, 1)->setFlags(Qt::ItemIsEnabled);
    table->setRowHeight(0,20);

    int tableRow = 1;

    QVector<double> xData, yData;

    for(int i = 0; i < trans->spectrum.xData.size(); i++)
    {
        xData.append(trans->spectrum.xData.at(i));
        yData.append(trans->spectrum.yData.at(i));

        QTableWidgetItem *item1 = new QTableWidgetItem(this->locale().toString(trans->spectrum.xData.at(i)));
        QTableWidgetItem *item2 = new QTableWidgetItem(this->locale().toString(trans->spectrum.yData.at(i)));;
        item1->setFlags(Qt::ItemIsEnabled);
        item2->setFlags(Qt::ItemIsEnabled);
        table->setItem(tableRow, 0, item1);
        table->setItem(tableRow, 1, item2);
        table->setRowHeight(tableRow,20);
        tableRow++;
    }

    // plot for visualization
    curve = new BasePlotCurve(trans->name);

    QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
    s->setColor(QColor(Qt::red));
    s->setSize(12);
    curve->setSymbol(s);
    curve->setPen(QColor(Qt::red));
    curve->setSamples(xData, yData);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    basePlot->registerCurve(curve);
}


void TransmissionEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("no item selected. add item to list first");
        return;
    }

    DbEditorWidget::onApplyChanges();

    TransmissionDBE *trans = dbm->getTransmission(currentIndex);

    QString oldFilename = trans->filename;
    QString newFilename = leFname->text();

    try
    {
        for(int i = 0; i < db_data->size(); i++)
        {
            if(newFilename != oldFilename && db_data->at(i)->filename == newFilename)
                throw LIISimException("Invalid filename, file already exists: "+newFilename);
        }
        trans->filename = newFilename.toLatin1().data();
        trans->name = leName->text().toLatin1().data();
        trans->description = leDescr->text().toLatin1().data();

        dbm->modifiedContent(trans);
        QString msg = "applied changed to:" + newFilename;
        MSG_STATUS(msg);
        if(newFilename != oldFilename)
        {
            dbm->deleteFile(oldFilename.toLatin1().data());
            msg.append(" deleted:" + oldFilename);
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


void TransmissionEditor::onAddItemToList()
{
    DbEditorWidget::onAddItemToList();
    QString filename = getValidDefaultFileName();

    try
    {
        TransmissionDBE *trans = new TransmissionDBE;
        trans->name = defaultFileName.toLatin1().data();
        trans->filename = filename.toLatin1().data();

        dbm->addContentToDB(trans);
        MSG_STATUS("added:"+filename);
    }
    catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        butAdd->setEnabled(true);
    }
    butAdd->setEnabled(true);
}


void TransmissionEditor::onRemoveCurrentSelectionFromList()
{
    DbEditorWidget::onRemoveCurrentSelectionFromList();
}
