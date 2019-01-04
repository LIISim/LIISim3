#include "mrundetailswidget.h"

#include <QHeaderView>
#include <QFileInfo>

#include <core.h>

#include "signal/mrun.h"
#include "settings/mrunsettings.h"
#include "numberlineedit.h"
#include "liisettingscombobox.h"
#include "udpeditor.h"


MRunDetailsWidget::MRunDetailsWidget(QWidget *parent) : QTreeWidget(parent)
{
    run = nullptr;
    udpEditor = nullptr;

    setColumnCount(2);
    setRootIsDecorated(false);
    QStringList header;
    header << "Run Details" << " ";
    setHeaderLabels(header);

    //save run settings
    saveSettings = new QTreeWidgetItem(this);
    checkboxOverwrite = new QCheckBox("Overwrite Run Settings", this);
    checkboxOverwrite->setToolTip("If this Option is enabled the "
                                  "Run's settings file will\nbe overwritten "
                                  "on program shutdown.");
    buttonForceSave = new QPushButton("Save", this);
    buttonForceSave->setToolTip("Overwrites the current run settings directly, without waiting for program shutdown.");
    setItemWidget(saveSettings, 0, checkboxOverwrite);
    setItemWidget(saveSettings, 1, buttonForceSave);
    connect(checkboxOverwrite, SIGNAL(stateChanged(int)), SLOT(onCheckboxOverwriteStateChanged(int)));
    connect(buttonForceSave, SIGNAL(clicked(bool)), SLOT(onButtonForceSaveClicked()));

    //run name
    runName = new QTreeWidgetItem(this);
    runName->setText(0, "Name");
    runName->setToolTip(0, "Name of the measurement run");
    editRunName = new QLineEdit(this);
    editRunName->setPlaceholderText("No run selected");
    setItemWidget(runName, 1, editRunName);
    connect(editRunName, SIGNAL(editingFinished()), SLOT(onLineEditingFinished()));

    //run description
    runDescription = new QTreeWidgetItem(this);
    runDescription->setText(0, "Description");
    runDescription->setToolTip(0, "Description of the measurement run");
    editRunDescription = new QLineEdit(this);
    setItemWidget(runDescription, 1, editRunDescription);
    connect(editRunDescription, SIGNAL(editingFinished()), SLOT(onLineEditingFinished()));

    //laser fluence
    laserFluence = new QTreeWidgetItem(this);
    laserFluence->setText(0, "Laser Fluence (mJ/mm²)");
    laserFluence->setToolTip(0, "Laser Fluence (mJ/mm²)");
    editLaserFluence = new NumberLineEdit(NumberLineEdit::DOUBLE, this);
    setItemWidget(laserFluence, 1, editLaserFluence);
    connect(editLaserFluence, SIGNAL(editingFinished()), SLOT(onLineEditingFinished()));

    //lii settings
    liiSettings = new QTreeWidgetItem(this);
    liiSettings->setText(0, "LII Settings");
    liiSettings->setToolTip(0, "Current LII settings");
    cbLiisettings = new LIISettingsComboBox(this);
    setItemWidget(liiSettings, 1, cbLiisettings);
    connect(cbLiisettings, SIGNAL(currentIndexChanged(int)), SLOT(onLIISettingsChanged()));

    //nd filter
    ndFilter = new QTreeWidgetItem(this);
    ndFilter->setText(0, "ND-Filter");
    ndFilter->setToolTip(0, "Neutral density filter settings, also see LII settings");
    cbNDFilter = new QComboBox(this);
    setItemWidget(ndFilter, 1, cbNDFilter);
    connect(cbNDFilter, SIGNAL(currentIndexChanged(int)), SLOT(onFilterChanged()));

    //pmt channel gain
    pmtChannelGain = new QTreeWidgetItem(this);
    pmtChannelGain->setText(0, "PMT Channel Gain Voltages (V)");
    pmtChannelGain->setFirstColumnSpanned(true);
    setFirstItemColumnSpanned(pmtChannelGain, true);

    //pmt measured channel gain
    pmtChannelMeasured = new QTreeWidgetItem(this);
    pmtChannelMeasured->setText(0, "PMT Channel Gain Measured (V)");
    pmtChannelMeasured->setFirstColumnSpanned(true);
    setFirstItemColumnSpanned(pmtChannelMeasured, true);

    //acquisition mode
    acquisitionMode = new QTreeWidgetItem(this);
    acquisitionMode->setText(0, "Acquisition Mode");

    //laser setpoint
    laserSetpoint = new QTreeWidgetItem(this);
    laserSetpoint->setText(0, "Laser Setpoint");

    //laser position
    laserPosition = new QTreeWidgetItem(this);
    laserPosition->setText(0, "Laser Position");

    //import directory
    importDirectory = new QTreeWidgetItem(this);
    importDirectory->setText(0, "Import Directory");

    //loaded files
    loadedFiles = new QTreeWidgetItem(this);
    loadedFiles->setText(0, "Loaded Files");
    setFirstItemColumnSpanned(loadedFiles, true);

    //user-defined parameters
    userDefinedParameters = new QTreeWidgetItem(this);
    userDefinedParameters->setText(0, "User-Defined Parameters");
    buttonEditUDP = new QPushButton("Edit", this);
    buttonEditUDP->setStyleSheet("QPushButton { background: none }");
    setItemWidget(userDefinedParameters, 1, buttonEditUDP);
    connect(buttonEditUDP, SIGNAL(clicked(bool)), SLOT(onButtonEditUDPClicked()));

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));

    resizeColumnToContents(0);
    setColumnWidth(1, 110);

    setRun(run);
}

void MRunDetailsWidget::setRun(MRun *mrun)
{
    if(run)
       run->disconnect(this);

    run = mrun;

    updateName();
    updateDescription();
    updateLIISettings();
    updateFilter();
    updateLaserFluence();
    updatePMTGain();
    updateSignalIO();
    updateUDP();

    if(!run)
    {
        buttonForceSave->setEnabled(false);

        acquisitionMode->setDisabled(true);
        acquisitionMode->setText(1, "");
        acquisitionMode->setForeground(0, QBrush(Qt::gray));

        laserSetpoint->setDisabled(true);
        laserSetpoint->setText(1, "");
        laserSetpoint->setForeground(0, QBrush(Qt::gray));

        laserPosition->setDisabled(true);
        laserPosition->setText(1, "");
        laserPosition->setForeground(0, QBrush(Qt::gray));

        return;
    }

    buttonForceSave->setEnabled(!run->getChangedSettings().isEmpty());

    //buttonForceSave->setEnabled(checkboxOverwrite->isChecked());

    //acquisition mode
    acquisitionMode->setDisabled(false);
    acquisitionMode->setText(1, run->getAcquisitionMode());
    acquisitionMode->setForeground(0, QBrush(Qt::black));

    //laser setpoint
    laserSetpoint->setDisabled(false);
    laserSetpoint->setText(1, QString::number(run->getLaserSetpoint()));
    laserSetpoint->setForeground(0, QBrush(Qt::black));

    //laser position
    laserPosition->setDisabled(false);
    laserPosition->setText(1, QString::number(run->getLaserPosition()));
    laserPosition->setForeground(0, QBrush(Qt::black));

    connect(run, SIGNAL(dataChanged(int,QVariant)), SLOT(onMRunDataChanged(int,QVariant)));
    connect(run, SIGNAL(destroyed(int)), SLOT(onMRunDestroyed()));
}


void MRunDetailsWidget::updateFilter()
{
    if(!run)
    {
        ndFilter->setDisabled(true);
        ndFilter->setBackgroundColor(0, Qt::white);
        ndFilter->setForeground(0, QBrush(Qt::gray));
        cbNDFilter->clear();
        cbNDFilter->setEnabled(false);
    }
    else
    {
        ndFilter->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::NDFilter))
        {
            ndFilter->setForeground(0, QBrush(QColor("indianred")));
            if(!buttonForceSave->isEnabled())
                buttonForceSave->setEnabled(true);
        }
        else
            ndFilter->setForeground(0, QBrush(Qt::black));
        cbNDFilter->setEnabled(true);

        cbNDFilter->blockSignals(true);
        cbNDFilter->clear();

        filterList flist = run->liiSettings().filters;
        for(int i = 0; i < flist.size(); i++)
            cbNDFilter->addItem(flist.at(i).identifier);

        if(run->filterIdentifier() == Filter::filterNotSetIdentifier)
        {
            cbNDFilter->setToolTip("Filter has not been set in run settings, please set a filter and save the run settings");
            if(cbNDFilter->findText(Filter::filterNotSetIdentifier) == -1)
                cbNDFilter->addItem(Filter::filterNotSetIdentifier);
            cbNDFilter->setCurrentText(Filter::filterNotSetIdentifier);
        }

        if(!run->liiSettings().isFilterDefined(run->filterIdentifier()))
        {
            ndFilter->setBackgroundColor(0, Qt::red);
            cbNDFilter->setToolTip(QString("Filter '%0' not defined in LIISettings '%1'").arg(run->filterIdentifier()).arg(run->liiSettings().name));
            QString name = QString("undefined filter ('%0')").arg(run->filterIdentifier());
            if(cbNDFilter->findText(name) == -1)
                cbNDFilter->addItem(name, run->filterIdentifier());
            cbNDFilter->setCurrentText(name);
            cbNDFilter->blockSignals(false);
            return;
        }

        ndFilter->setBackgroundColor(0, Qt::white);

        cbNDFilter->setCurrentText(run->filterIdentifier());

        cbNDFilter->blockSignals(false);
    }
}


void MRunDetailsWidget::updateName()
{
    if(!run)
    {
        runName->setDisabled(true);
        runName->setForeground(0, QBrush(Qt::gray));
        editRunName->setText("");
        editRunName->setEnabled(false);
        editRunName->setFrame(false);
    }
    else
    {
        runName->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::Name))
        {
            runName->setForeground(0, QBrush(QColor("indianred")));
            if(!buttonForceSave->isEnabled())
                buttonForceSave->setEnabled(true);
        }
        else
            runName->setForeground(0, QBrush(Qt::black));
        editRunName->setEnabled(true);
        editRunName->setFrame(true);
        editRunName->setText(run->name);
        editRunName->home(false);
    }
}


void MRunDetailsWidget::updateDescription()
{
    editRunDescription->blockSignals(true);
    if(!run)
    {
        runDescription->setDisabled(true);
        runDescription->setForeground(0, QBrush(Qt::gray));
        editRunDescription->setText("");
        editRunDescription->setEnabled(false);
        editRunDescription->setFrame(false);
    }
    else
    {
        runDescription->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::Description))
        {
            runDescription->setForeground(0, QBrush(QColor("indianred")));
            if(!buttonForceSave->isEnabled())
                buttonForceSave->setEnabled(true);
        }
        else
            runDescription->setForeground(0, QBrush(Qt::black));
        editRunDescription->setEnabled(true);
        editRunDescription->setFrame(true);
        editRunDescription->setText(run->description());
        editRunDescription->home(false);
    }
    editRunDescription->blockSignals(false);
}


void MRunDetailsWidget::updateLIISettings()
{
    if(!run)
    {
        liiSettings->setDisabled(true);
        liiSettings->setForeground(0, QBrush(Qt::gray));
        cbLiisettings->setEnabled(false);
    }
    else
    {
        liiSettings->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::LIISettings))
        {
            liiSettings->setForeground(0, QBrush(QColor("indianred")));
            if(!buttonForceSave->isEnabled())
                buttonForceSave->setEnabled(true);
        }
        else
            liiSettings->setForeground(0, QBrush(Qt::black));
        cbLiisettings->setEnabled(true);
        cbLiisettings->setCurrentText(run->liiSettings().name);
    }
}


void MRunDetailsWidget::updateLaserFluence()
{
    if(!run)
    {
        laserFluence->setDisabled(true);
        laserFluence->setForeground(0, QBrush(Qt::gray));
        editLaserFluence->setText("");
        editLaserFluence->setEnabled(false);
        editLaserFluence->setFrame(false);
    }
    else
    {
        laserFluence->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::LaserFluence))
            laserFluence->setForeground(0, QBrush(QColor("indianred")));
        else
            laserFluence->setForeground(0, QBrush(Qt::black));
        editLaserFluence->setEnabled(true);
        editLaserFluence->setFrame(true);
        editLaserFluence->setValue(run->laserFluence());
    }
}


void MRunDetailsWidget::updatePMTGain()
{
    for(int i = 0; i < pmtChannelGain->childCount(); i++)
        pmtChannelGain->child(i)->setHidden(true);

    for(int i = 0; i < pmtChannelMeasured->childCount(); i++)
        pmtChannelMeasured->child(i)->setHidden(true);

    if(!run)
    {
        pmtChannelGain->setDisabled(true);
        pmtChannelGain->setForeground(0, QBrush(Qt::gray));
        pmtChannelMeasured->setDisabled(true);
        pmtChannelMeasured->setForeground(0, QBrush(Qt::gray));
    }
    else
    {
        pmtChannelGain->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::PMTChannelGain))
        {
            pmtChannelGain->setForeground(0, QBrush(QColor("indianred")));
            if(!buttonForceSave->isEnabled())
                buttonForceSave->setEnabled(true);
        }
        else
            pmtChannelGain->setForeground(0, QBrush(Qt::black));
        pmtChannelMeasured->setDisabled(false);
        pmtChannelMeasured->setForeground(0, QBrush(Qt::black));

        QList<int> channelIDs = run->channelIDs(Signal::RAW);

        for(int i = 0; i < channelIDs.size(); i++)
        {
            if(!editPMTGainList.contains(channelIDs.at(i)))
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(pmtChannelGain);
                item->setText(0, QString("Channel %0").arg(channelIDs.at(i)));

                NumberLineEdit *nle = new NumberLineEdit(NumberLineEdit::DOUBLE, this);
                setItemWidget(item, 1, nle);
                connect(nle, SIGNAL(editingFinished()), SLOT(onPMTGainEditingFinished()));

                QPair<QTreeWidgetItem*, NumberLineEdit*> pair(item, nle);

                editPMTGainList.insert(channelIDs.at(i), pair);
            }

            if(!pmtMeasuredGainList.contains(channelIDs.at(i)))
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(pmtChannelMeasured);
                item->setText(0, QString("Channel %0").arg(channelIDs.at(i)));

                pmtMeasuredGainList.insert(channelIDs.at(i), item);
            }

            editPMTGainList.value(channelIDs.at(i)).second->setValue(run->pmtGainVoltage(channelIDs.at(i)));
            editPMTGainList.value(channelIDs.at(i)).first->setHidden(false);

            pmtMeasuredGainList.value(channelIDs.at(i))->setText(1, QString::number(run->pmtReferenceGainVoltage(channelIDs.at(i))));
            pmtMeasuredGainList.value(channelIDs.at(i))->setHidden(false);
        }

        pmtChannelGain->setExpanded(true);
        pmtChannelMeasured->setExpanded(true);
    }
}


void MRunDetailsWidget::updateSignalIO()
{
    while(loadedFiles->childCount() > 0)
        delete loadedFiles->takeChild(0);

    if(!run)
    {
        importDirectory->setDisabled(true);
        importDirectory->setText(1, "");
        importDirectory->setToolTip(1, "");
        importDirectory->setForeground(0, QBrush(Qt::gray));

        loadedFiles->setDisabled(true);
        loadedFiles->setForeground(0, QBrush(Qt::gray));
    }
    else
    {
        SignalFileInfoList fileList = run->importRequest().flist;

        if(!fileList.isEmpty())
        {
            QFileInfo fi(fileList.first().filename);

            importDirectory->setDisabled(false);
            importDirectory->setText(1, fi.absolutePath());
            importDirectory->setToolTip(1, fi.absolutePath());
            importDirectory->setForeground(0, QBrush(Qt::black));

            loadedFiles->setDisabled(false);
            loadedFiles->setForeground(0, QBrush(Qt::black));
            for(SignalFileInfo sfi : fileList)
            {
                fi = QFileInfo(sfi.filename);
                QTreeWidgetItem *item = new QTreeWidgetItem(loadedFiles);
                item->setText(0, fi.fileName());
                item->setFirstColumnSpanned(true);
            }
            loadedFiles->setExpanded(true);
        }
    }
}


void MRunDetailsWidget::updateUDP()
{
    while(userDefinedParameters->childCount() > 0)
        delete userDefinedParameters->takeChild(0);

    if(!run)
    {
        userDefinedParameters->setDisabled(true);
        userDefinedParameters->setForeground(0, QBrush(Qt::gray));
        buttonEditUDP->setEnabled(false);
    }
    else
    {
        userDefinedParameters->setDisabled(false);
        if(run->getChangedSettings().contains(MRun::UserChangableSettingTypes::UserDefinedParameters))
        {
            userDefinedParameters->setForeground(0, QBrush(QColor("indianred")));
            if(!buttonForceSave->isEnabled())
                buttonForceSave->setEnabled(true);
        }
        else
            userDefinedParameters->setForeground(0, QBrush(Qt::black));
        buttonEditUDP->setEnabled(true);

        for(auto udp : run->userDefinedParameters.keys())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(userDefinedParameters);
            item->setText(0, udp);
            item->setToolTip(0, udp);
            item->setText(1, run->userDefinedParameters.value(udp).toString());
            item->setToolTip(1, run->userDefinedParameters.value(udp).toString());
        }

        userDefinedParameters->setExpanded(true);
    }
}


void MRunDetailsWidget::onCheckboxOverwriteStateChanged(int state)
{
    Core::instance()->guiSettings->setValue("rundetails", "overwrite", state == Qt::Checked);
    //emit Core::instance()->guiSettings->settingsChanged();
    if(run)
    {
        if(run->getChangedSettings().isEmpty())
            buttonForceSave->setEnabled(state == Qt::Checked);
        else
            buttonForceSave->setEnabled(true);
    }
}


void MRunDetailsWidget::onButtonForceSaveClicked()
{
    if(!run)
        return;

    if(run->saveCurrentRunSettings())
        setRun(run);
}


void MRunDetailsWidget::onButtonEditUDPClicked()
{
    if(!run)
        return;

    if(!udpEditor)
        udpEditor = new UDPEditor(this);

    udpEditor->setMRun(run);
    udpEditor->exec();

    updateUDP();
}


void MRunDetailsWidget::onLineEditingFinished()
{
    if(!run)
        return;

    if(QObject::sender() == editRunName)
    {
        run->setName(editRunName->text());
        updateName();
    }
    else if(QObject::sender() == editRunDescription)
    {
        run->setDescription(editRunDescription->text());
        updateDescription();
    }
    else if(QObject::sender() == editLaserFluence)
    {
        run->setLaserFluence(editLaserFluence->getValue());
        updateLaserFluence();
    }
}


void MRunDetailsWidget::onLIISettingsChanged()
{
    if(!run)
        return;

    QString filter = run->filterIdentifier();

    // before changing the run's LiiSettings:
    // store current Filter selection dependent on current LiiSettings, but only if the filter
    // is valid -> defined in LiiSettings and well defined in run settings
    //if(mrun->liiSettings().isFilterDefined(mrun->filterIdentifier()) &&
    //   !(mrun->filterIdentifier() == Filter::filterNotSetItendifier))
    //    filterSelections.insert(mrun->liiSettings().ident, mrun->filterIdentifier());

    // update the run's LiiSettings, also modifies the run's current Filter!
    run->setLiiSettings(cbLiisettings->currentLIISettings());

    run->setFilter(filter);

    // reset the Filter selection using the LiiSettings->Filter map if contains a filter for
    // the mrun, set to "no Filter" otherwise
    //if(filterSelections.contains(mrun->liiSettings().ident))
    //    mrun->setFilter( filterSelections.value(mrun->liiSettings().ident ));
    //else
    //    mrun->setFilter("no Filter");

    updateFilter();
}


void MRunDetailsWidget::onFilterChanged()
{
    if(!run)
        return;

    QString itemText = cbNDFilter->currentText();
    if(itemText.startsWith("undefined"))
        itemText = cbNDFilter->currentData().toString();

    run->setFilter(itemText);
}


void MRunDetailsWidget::onPMTGainEditingFinished()
{
    if(!run)
        return;

    for(auto e : editPMTGainList.keys())
        if(editPMTGainList.value(e).second == QObject::sender())
            run->setPmtGainVoltage(e, editPMTGainList.value(e).second->getValue());
}


void MRunDetailsWidget::onMRunDataChanged(int pos, QVariant value)
{
    //name
    if(pos == 0)
        updateName();
    //description
    else if(pos == 3)
        updateDescription();
    //lii settings
    else if(pos == 4)
        updateLIISettings();
    //laser fluence
    else if(pos == 5)
        updateLaserFluence();
    //pmt channel gain
    else if(pos == 6)
        updatePMTGain();
    //filter
    else if(pos == 7)
        updateFilter();
    //signal io information
    else if(pos == 8)
        updateSignalIO();
}


void MRunDetailsWidget::onMRunDestroyed()
{
    run = nullptr;
    setRun(run);
}


void MRunDetailsWidget::onGUISettingsChanged()
{
    checkboxOverwrite->setChecked(Core::instance()->guiSettings->value("rundetails", "overwrite", false).toBool());
    if(run && !checkboxOverwrite->isChecked())
        buttonForceSave->setEnabled(!run->getChangedSettings().isEmpty());
    else
        buttonForceSave->setEnabled(checkboxOverwrite->isChecked());
}
