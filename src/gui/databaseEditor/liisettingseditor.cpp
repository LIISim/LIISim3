#include "liisettingseditor.h"
#include "../../general/LIISimException.h"

#include "../../database/databasemanager.h"
#include "../../calculations/temperature.h"

#include <QHeaderView>


LiiSettingsEditor::LiiSettingsEditor(QWidget *parent) :  DbEditorWidget(parent)
{
    db_data = NULL;
    defaultFileName = "liisettings/newSettings";
    defaultName = "new LII settings";

    table_channel = new QTableWidget;
    table_filters = new QTableWidget;

    lay_props->addWidget(table_channel,2,0,2,5);
    lay_props->addWidget(table_filters,3,0,2,5);

    // update table if modelingSettings are changed -> E(m) values need to be recalculated
    connect(Core::instance()->modelingSettings, SIGNAL(materialSpecChanged()),this, SLOT(initData()));

}


void LiiSettingsEditor::setDatabaseManager(DatabaseManager *dbm)
{
    DbEditorWidget::setDatabaseManager(dbm);
    db_data = dbm->getLIISettings();
}


void LiiSettingsEditor::initData()
{
   DbEditorWidget::initData();   
}


void LiiSettingsEditor::onSelectionChanged(const QItemSelection &selection)
{
    DbEditorWidget::onSelectionChanged(selection);

    LIISettings* dbi = dbm->getLIISetting(currentIndex);

    // set number of rows
    int no_channels = dbi->channels.size();
    int no_filters = dbi->filters.size();

    table_channel->clear();
    table_filters->clear();

    // setup table channel
    table_channel->setColumnCount(9);
    table_channel->setRowCount(no_channels);

    int a = 0;

    table_channel->setColumnWidth(a++, 80); // channel
    table_channel->setColumnWidth(a++, 80); // wavelength
    table_channel->setColumnWidth(a++, 120);  // bandpass    
    table_channel->setColumnWidth(a++, 120); // calibration sensitivity
    table_channel->setColumnWidth(a++, 140); // gain reference voltage
    table_channel->setColumnWidth(a++, 140); // gain calibration
    table_channel->setColumnWidth(a++, 120); // offset
    table_channel->setColumnWidth(a++, 250); // Current E(m)
    table_channel->setColumnWidth(a++, 300); // Current E(m) (Drude)


    QStringList columnNames;
    columnNames << "Channel"
                << "Wavelength"
                << "Bandpass filter \n width"
                << "Calibration \n (Sensititvity)"
                << "Gain calibration \n (x_ref)"
                << "Gain calibration \n (A)"
                << "Offset \n (Baseline)"
                << "Current E(m) (Function)\n (from Spectroscopic Material)"
                << "Current E(m) (Drude)\n (from Spectroscopic Material)";
    table_channel->setHorizontalHeaderLabels(columnNames);

    a = 0;

    table_channel->horizontalHeaderItem(a++)->setToolTip("Channel identifier");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Center wavelength \n"
                                                         "This wavelength is used for pyrometry within the TemperatureCalculator-Plugin.");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Bandpass filter width is used for advanced analysis tools");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Calibration sensitivity coefficients are used within the Calibration-Plugin \n"
                                                         "to correct all channels for its spectral sensitivity. \n"
                                                         "Please see the UserGuide for detailed information about the calculation procedure");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Gain calibration reference voltage x_ref is used by the Calibration-Plugin \n"
                                                         "Please see the UserGuide for detailed information about the calculation procedure");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Gain calibration coefficient A is used by the Calibration-Plugin \n"
                                                         "Please see the UserGuide for detailed information about the calculation procedure");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Offset is used for ungated PMTs within the Baseline-Plugin when 'LIISettings' is \n "
                                                         "selected as calculation method\n"
                                                         "Please see the UserGuide for detailed information about the calculation procedure");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Shows the current E(m) value calculated for this channel \n"
                                                         "by the Em_func from the SpectroscopicMaterial selected \n "
                                                         "within the SignalProcessing module");
    table_channel->horizontalHeaderItem(a++)->setToolTip("Shows the current E(m) value calculated for this channel \n"
                                                         "by the omega_p and tau from the SpectroscopicMaterial \n "
                                                         "selected within the SignalProcessing module");

    table_channel->verticalHeader()->setVisible(false);

    showChannels(dbi);


    // setup table channel
    table_filters->setColumnCount(1+no_channels);
    table_filters->setRowCount(no_filters);

    columnNames.clear();
    columnNames << "Filter identifier";

    a = 0;

    table_filters->setColumnWidth(a++, 140); // Filter identifier
    for(int k = 0; k < dbi->channels.size(); k++)
    {
        table_filters->setColumnWidth(a++, 100); // transmission
        columnNames << QString("Transmission \n Channel %0").arg(k+1);
    }

    table_filters->setHorizontalHeaderLabels(columnNames);

    table_filters->verticalHeader()->setVisible(false);

    showFilters(dbi);
}


void LiiSettingsEditor::showChannels(LIISettings* dbi)
{

    // name of spectroscopic model
    QString name = Core::instance()->modelingSettings->materialSpec().name;

    bool Em_func_available = Core::instance()->modelingSettings->materialSpec().Em_func.available;
    bool omega_p_available = Core::instance()->modelingSettings->materialSpec().omega_p.available;
    bool tau_available = Core::instance()->modelingSettings->materialSpec().tau.available;

    double Em, EmDrude;

    // show channel data
    for(int i=0; i < dbi->channels.size(); i++)
    {
        QTableWidgetItem* item1 = new QTableWidgetItem(QString("Channel %0").arg(i+1));
        QTableWidgetItem* item2 = new QTableWidgetItem(QString("%0 nm").arg(dbi->channels.at(i).wavelength));
        QTableWidgetItem* item3 = new QTableWidgetItem(QString("%0 nm").arg(dbi->channels.at(i).bandwidth));        
        QTableWidgetItem* item4 = new QTableWidgetItem(QString("%0").arg(dbi->channels.at(i).calibration));
        QTableWidgetItem* item5 = new QTableWidgetItem(QString("%0 V").arg(dbi->channels.at(i).pmt_gain));
        QTableWidgetItem* item6 = new QTableWidgetItem(QString("%0").arg(dbi->channels.at(i).pmt_gain_formula_A));
        QTableWidgetItem* item7 = new QTableWidgetItem(QString("%0 V").arg(dbi->channels.at(i).offset));


        // get current E(m) values from selected Material
        double lambda = dbi->channels.at(i).wavelength;
        double lambda_m = dbi->channels.at(i).wavelength * 1E-9;

        QTableWidgetItem* item8;
        QTableWidgetItem* item9;

        //qDebug() << "LIISettingsEditor: Test" << i;
        //EmDrude = Temperature::calcDrudeEm(lambda, Core::instance()->modelingSettings->materialSpec());

        if(Em_func_available)
        {
            Em = Core::instance()->modelingSettings->materialSpec().Em_func(0.0, lambda_m);
            item8 = new QTableWidgetItem(QString("%0 (%1)").arg(Em).arg(name));
        }
        else
        {
            item8 = new QTableWidgetItem(QString("Em_func not defined (%0)").arg(name));
        }

        if(omega_p_available && tau_available)
        {
            EmDrude = Temperature::calcDrudeEm(lambda, Core::instance()->modelingSettings->materialSpec());
            item9 = new QTableWidgetItem(QString("%0 (%1)").arg(EmDrude).arg(name));
        }
        else if(omega_p_available)
        {
            item9 = new QTableWidgetItem(QString("tau not defined (%0)").arg(name));
        }
        else if(tau_available)
        {
            item9 = new QTableWidgetItem(QString("omega_p not defined (%0)").arg(name));
        }
        else
        {
            item9 = new QTableWidgetItem(QString("omega_p and tau not defined (%0)").arg(name));
        }




        QString tooltip     = QString("Channel %0").arg(i+1);
        QString tooltip_Em  = QString("Current Material (Spectroscopic): %0")
                                    .arg(Core::instance()->modelingSettings->materialSpec().name);

        item1->setToolTip(tooltip);
        item2->setToolTip(tooltip);
        item3->setToolTip(tooltip);
        item4->setToolTip(tooltip);
        item5->setToolTip(tooltip);
        item6->setToolTip(tooltip);
        item7->setToolTip(tooltip);
        item8->setToolTip(tooltip_Em);
        item9->setToolTip(tooltip_Em);

        item1->setFlags(Qt::ItemIsEnabled);
        item2->setFlags(Qt::ItemIsEnabled);
        item3->setFlags(Qt::ItemIsEnabled);
        item4->setFlags(Qt::ItemIsEnabled);
        item5->setFlags(Qt::ItemIsEnabled);
        item6->setFlags(Qt::ItemIsEnabled);
        item7->setFlags(Qt::ItemIsEnabled);
        item8->setFlags(Qt::ItemIsEnabled);
        item9->setFlags(Qt::ItemIsEnabled);

        table_channel->setItem(i, 0, item1);
        table_channel->setItem(i, 1, item2);
        table_channel->setItem(i, 2, item3);
        table_channel->setItem(i, 3, item4);
        table_channel->setItem(i, 4, item5);
        table_channel->setItem(i, 5, item6);
        table_channel->setItem(i, 6, item7);
        table_channel->setItem(i, 7, item8);
        table_channel->setItem(i, 8, item9);

        table_channel->setRowHeight(i, 20);
    }
}


void LiiSettingsEditor::showFilters(LIISettings* dbi)
{
    // show filter data
    for(int i = 0; i < dbi->filters.size(); i++)
    {
        QTableWidgetItem* item1 = new QTableWidgetItem(QString("%0").arg(dbi->filters.at(i).identifier));

        table_filters->setItem(i, 0, item1);

        for(int c = 0; c < dbi->channels.size(); c++)
        {
            QTableWidgetItem* itemF;
            QString tooltip;


            if(dbi->filters[i].identifier == "no Filter")
            {
                itemF = new QTableWidgetItem(QString("100 %"));
            }
            else
            {
                if(dbi->filters[i].getTransmissions().size() <= dbi->channels.size())
                {
                    double ftrans = dbi->filters[i].getTransmissions().at(c);

                    itemF = new QTableWidgetItem(QString("%0 %").arg(ftrans));

                    if(ftrans == 100.0)
                    {
                        tooltip = QString("Warning: transmission was not defined in file (Calculate with 100% transmission for this channel");
                        itemF->setTextColor(Qt::red);
                    }
                    else{
                        tooltip = QString("Filter transmission for Channel %0").arg(c+1);
                    }
                }
                else
                {
                    // ignore transmission values (channel does not exist)
                    continue;
                }
            }

            itemF->setToolTip(tooltip);

            itemF->setFlags(Qt::ItemIsEnabled);
            itemF->setTextAlignment(Qt::AlignRight);

            table_filters->setItem(i, c+1, itemF);
        }
        table_filters->setRowHeight(i, 20);
    }
}


void LiiSettingsEditor::onApplyChanges()
{
    if(db_data->size() == 0)
    {
        MSG_STATUS("No item selected. Add item to list first");
        return;
    }

    DbEditorWidget::onApplyChanges();

    LIISettings* modSettings = dbm->getLIISetting(currentIndex);

    QString oldfname = modSettings->filename;
    QString newfname = leFname->text();

    try
    {
        for(int i=0;i<db_data->size();i++)
        {
            if(newfname != oldfname && db_data->at(i)->filename == newfname  )
                throw LIISimException("Invalid filename, file already exists: "+newfname);
        }

        modSettings->filename = newfname.toLatin1().data();
        modSettings->name = leName->text().toLatin1().data();
        modSettings->description = leDescr->text().toLatin1().data();

        dbm->modifiedContent(modSettings);
        QString msg = "applied changes to: "+newfname;
        MSG_STATUS(msg);
        if(newfname != oldfname){
            dbm->deleteFile(oldfname.toLatin1().data());
            msg.append(" deleted: "+oldfname);
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


void LiiSettingsEditor::onAddItemToList()
{
    DbEditorWidget::onAddItemToList();

    // get filename
    QString fname = getValidDefaultFileName();

    try
    {
        LIISettings* m = new LIISettings;
        m->name = defaultName.toLatin1().data();
        m->filename = fname.toLatin1().data();

        dbm->addContentToDB(m);
        MSG_STATUS("added: "+fname);
    }
    catch(LIISimException e)
    {
        MSG_STATUS(e.what());
        butAdd->setEnabled(true);
    }

   // updateView();
    butAdd->setEnabled(true);
}


void LiiSettingsEditor::onRemoveCurrentSelectionFromList()
{
    DbEditorWidget::onRemoveCurrentSelectionFromList();
}
