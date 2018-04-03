#include "processingplugintreewidgetitem.h"
#include "../../utils/labeledlineedit.h"
#include "../../utils/labeledcombobox.h"
#include "../../utils/checkboxinput.h"
#include "../../utils/checkboxgroupinput.h"
#include "../../signal/processing/processingchain.h"
#include "../../signal/processing/plugins/temperaturecalculator.h"
#include "../../signal/processing/plugins/dummyplugin.h"
#include "../../signal/processing/processingpluginconnector.h"
#include "../../core.h"
#include "../../signal/processing/pluginfactory.h"

#include <QMouseEvent>

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


/**
 * @brief PPTreeWidgetItem::PPTreeWidgetItem
 * @param plugin
 * @param parentItem
 */
PPTreeWidgetItem::PPTreeWidgetItem(ProcessingPlugin *plugin, QTreeWidgetItem *parentItem)
    : DataItemObserverObject(plugin),
      QTreeWidgetItem(parentItem)
{
    m_plugin = plugin;
    init();
    if(parentItem)
        setupItemWidgets();
}

PPTreeWidgetItem::PPTreeWidgetItem(ProcessingPlugin *plugin, QTreeWidget *view)
    : DataItemObserverObject(plugin),
      QTreeWidgetItem(view)
{
    m_plugin = plugin;

    init();
    if(view)
        setupItemWidgets();
}

void PPTreeWidgetItem::init()
{
    m_mute = true;
    m_isTempCalc = (m_plugin->getName() == TemperatureCalculator::pluginName);
    m_isDummy = (m_plugin->getName() == DummyPlugin::pluginName);

    setToolTip(0, "Show/hide details or toggle plot visibility of " + m_plugin->getName());
    setToolTip(1, m_plugin->getShortDescription());

    setData(0,Qt::UserRole,4);
    setData(0,Qt::UserRole+1,m_plugin->id());


    int pos = m_plugin->position();
    setData(0,Qt::UserRole+2,pos);
    setData(0,Qt::UserRole+5,m_plugin->plotVisibility());

    if(m_isDummy)
        this->setFlags(Qt::ItemIsEnabled);
    else
    {
        this->setFlags(Qt::ItemIsEnabled |
                   Qt::ItemIsSelectable  |
                   Qt::ItemIsDragEnabled |
                   Qt::ItemIsUserCheckable);

        if(m_plugin->activated())
            this->setCheckState(1,Qt::Checked);
        else
            this->setCheckState(1,Qt::Unchecked);

        if(m_plugin->stepBufferEnabled())
            this->setCheckState(3,Qt::Checked);
        else
            this->setCheckState(3,Qt::Unchecked);
    }

    this->setToolTip(3,"Enable/disable intermediate result buffer\n (if disabled data are not stored to memory) \n It is recommended to disable intermediate buffers, if large data sets are processed.");

    updateNameLabel();
    updateLinkStateLabel();
    updatePlotVisibilityIcon();
    handleDirtyStateChanged(m_plugin->dirty());

    initParams();
    m_mute = false;
}



void PPTreeWidgetItem::initParams()
{
    m_mute = true;
    ProcessingPluginInputList inputs = m_plugin->getInputs();

    for(int i = 0; i < inputs.size(); i++)
    {

        PluginInputField* inputField = NULL;
        ProcessingPluginInput input = inputs.at(i);
        QString str;

        switch (input.type)
        {
            case ProcessingPluginInput::DOUBLE_FIELD:

                inputField = new LabeledLineEdit(input.labelText,NumberLineEdit::DOUBLE);

                if(input.limits)
                {
                    QString ttip = QString(input.tooltip + "\nMin: %0\nMax: %1").arg(input.minValue.toInt()).arg(input.maxValue.toInt());
                    inputField->setToolTip(ttip);
                }
                else
                    inputField->setToolTip(input.tooltip);

                break;

            case ProcessingPluginInput::INTEGER_FIELD:

                inputField = new LabeledLineEdit(input.labelText,NumberLineEdit::INTEGER);

                if(input.limits)
                {
                    QString ttip = QString(input.tooltip + "\nMin: %0\nMax: %1").arg(input.minValue.toInt()).arg(input.maxValue.toInt());
                    inputField->setToolTip(ttip);
                }
                else
                    inputField->setToolTip(inputField->toolTip());

                break;

            case ProcessingPluginInput::CHECKBOX:

                inputField = new CheckboxInput;
                inputField->setPluginLabelText(input.labelText);
                inputField->setToolTip(input.tooltip);

                break;

            case ProcessingPluginInput::CHECKBOX_GROUP:

                inputField = new CheckboxGroupInput;
                inputField->setToolTip(input.tooltip);

                // get values from input and create input fields

                inputField->setPluginLabelList(input.checkboxGroupLabels);
                inputField->setPluginValueList(input.checkboxGroupValues);
                inputField->setPluginLabelText(input.labelText);

                // now create checkboxes:
                inputField->init();

                break;

            case ProcessingPluginInput::COMBOBOX:

                inputField = new LabeledComboBox(input.labelText);
                inputField->setToolTip(input.tooltip);

                break;
        }

        if(inputField != NULL)
        {
            if(input.limits)
            {
                inputField->setPluginParamMinValue(input.minValue);
                inputField->setPluginParamMaxValue(input.maxValue);
            }
            inputField->setPluginParamValue(input.value);
            inputField->pluginInputType = input.type;
            inputField->pluginParamIdentifier = input.identifier;
            inputField->setEnabled(input.enabled);
            inputField->visible = input.visible;
            connect(inputField,SIGNAL(parameterChanged()), this, SLOT(onFieldParameterChanged()));
            m_inputFields << inputField;
        }
    }
    m_mute = false;
}


void PPTreeWidgetItem::setupItemWidgets()
{
    if(!treeWidget())
        return;

    for(int i = 0; i < m_inputFields.size(); i++)
    {
        QTreeWidgetItem* paramItem = new QTreeWidgetItem(this);

        paramItem->setFlags(Qt::ItemIsEditable |
                            Qt::ItemIsEnabled |
                            Qt::ItemIsDragEnabled);

        treeWidget()->setItemWidget(paramItem,1,m_inputFields[i]);

        // hide/show plugin parameter
        treeWidget()->setItemHidden(paramItem,!m_inputFields[i]->visible);
    }
}


/**
 * @brief PPTreeWidgetItem::reloadParams reloads all ProcesingPlugin
 * Input fields. It clears all regarding TreeItemWidgets and resets
 * the internal PluginInputField list.
 * A Reset is necessary if the number of inputfield or the modalities
 * of a PluginInputField have changed (e.g. combobox entries etc.)
 * Also see PPTreeWidgetItem::onDataChanged(7) and the ProcessingPlugin::dataChanged(7)
 * signal.
*/
void PPTreeWidgetItem::reloadParams()
{
    m_mute = true;
    //qDebug() << "PPTreeWidgetItem::resetParams() " << m_plugin->getName();

    // delete all item widgets and internal input fields
    for(int i = 0; i < m_inputFields.size(); i++)
    {
        delete this->takeChild(0);
        PluginInputField* f = m_inputFields[i] ;
     //   delete f;
    }
    m_inputFields.clear();
    m_mute = false;

    // (re) initialize input fields and setup item widgets
    initParams();
    setupItemWidgets();
}


void PPTreeWidgetItem::onFieldParameterChanged()
{
    if(!m_plugin)
        return;

    if(m_mute)
        return;

    ProcessingPluginInputList list = m_plugin->getInputs();

    for(int i = 0; i < list.size(); i++)
    {
        for(int j = 0; j < m_inputFields.size(); j++)
        {
            if(list.at(i).identifier == m_inputFields.at(j)->pluginParamIdentifier)
            {
                QVariant value = m_inputFields.at(j)->getPluginParamValue();
                list[i].value = value;
                continue;
            }
        }
    }
    m_plugin->setParameters(list);
}


/**
 * @brief ProcessingPluginTreeWidgetItem::onDataChanged
 * @param pos
 * @param value
 */
void PPTreeWidgetItem::onDataChanged(int pos, QVariant value)
{
    if(invalidData())
        return;

    if(m_mute)
        return;

    m_mute = true;
    ProcessingPluginInputList inputs;


    // checkbox->setText(plug->getName()+plug->getParameterPreview());
    switch(pos)
    {
    case 1: // activation state changed

        updateNameLabel();
        break;

    case 2: // dirty changed (implies that parameters have been edited)

        handleDirtyStateChanged(value.toBool());

        updateNameLabel();

        inputs = m_plugin->getInputs();

        for(int i=0; i<m_inputFields.size(); i++)
        {
            QVariant val = inputs.getValue(m_inputFields.at(i)->pluginParamIdentifier);
            if(val.toString() != "notSet")
                m_inputFields.at(i)->setPluginParamValue(val);
        }

        break;
    case 4: // linkstate changed
        updateLinkStateLabel();
        break;
    case 5: //plot visibility
        updatePlotVisibilityIcon();
        setData(0,Qt::UserRole+5,m_plugin->plotVisibility());
        break;
    case 6: // step buffer
        if(m_plugin->stepBufferEnabled())
            this->setCheckState(3,Qt::Checked);
        else
            this->setCheckState(3,Qt::Unchecked);
        break;
    case 7: // input fields changed
        reloadParams();
        updateNameLabel();
        break;
    }
    m_mute = false;
}


/**
 * @brief ProcessingPluginTreeWidgetItem::onDataChildInserted
 * @param child_data
 */
void PPTreeWidgetItem::onDataChildInserted(DataItem *child_data, int position)
{
    //qDebug() << "DataItemObserverWidget::onDataChildInserted";
}


/**
 * @brief ProcessingPluginTreeWidgetItem::onDataChildRemoved
 * @param child_data
 */
void PPTreeWidgetItem::onDataChildRemoved(DataItem *child_data)
{
}


/**
 * @brief ProcessingPluginTreeWidgetItem::onDataDestroyed
 */
void PPTreeWidgetItem::onDataDestroyed()
{
    m_plugin = 0;
    // qDebug() << "DataItemObserverWidget::onDataDestroyed " << data_id();
    DataItemObserverObject::onDataDestroyed();
    this->deleteLater();
}




void PPTreeWidgetItem::handleTreeItemModified(int col)
{
    //qDebug() << " ProcessingPluginTreeWidgetItem::handleTreeItemChanged " << m_mute;
    if(m_mute)
        return;

    if(col == 1)
    {
        if(this->checkState(1) == Qt::Checked)
            m_plugin->setActivated(true);
        else
            m_plugin->setActivated(false);
    }
    else if(col == 3)
    {
        m_plugin->setStepBufferEnabled(this->checkState(3) == Qt::Checked);
    }
}


/**
 * @brief PPTreeWidgetItem::updateNameLabel updates the name/parameterpreview label.
 */
void PPTreeWidgetItem::updateNameLabel()
{
    //qDebug() << "PPTreeWidgetItem: update" << m_plugin->getParameterPreview() << m_plugin->processingError();

    if(m_isDummy)
    {
        setBackgroundColor(0,Qt::white);
        setBackgroundColor(1,Qt::white);
        setBackgroundColor(2,Qt::white);
        setBackgroundColor(3,Qt::white);

        QFont f;
        f.setItalic(false);
        this->setTextColor(1,QColor("#000000"));
        this->setFont(1, f);
        this->setText(1, m_plugin->getParameterPreview() );

        return;
    }

    QString nameLabelText;

    if(m_isTempCalc)
        nameLabelText = m_plugin->getParameterPreview() ;
    else
        nameLabelText = m_plugin->getName() + m_plugin->getParameterPreview();

    // if activated first set to dirty -> updateItemColors() overwrites this color after validation
    if(m_plugin->activated())
    {
        QColor pcolor_orange    = QColor(244,176,15);

        setBackgroundColor(0, pcolor_orange);
        setBackgroundColor(1, pcolor_orange);
        setBackgroundColor(2, pcolor_orange);
        setBackgroundColor(3, pcolor_orange);

        QFont f;
        f.setItalic(false);
        this->setTextColor(1,QColor("#000000"));
        this->setFont(1, f);
        this->setText(1, nameLabelText);
        this->setCheckState(1, Qt::Checked);
    }
    else
    {
        setBackgroundColor(0,Qt::white);
        setBackgroundColor(1,Qt::white);
        setBackgroundColor(2,Qt::white);
        setBackgroundColor(3,Qt::white);

        QFont f;
        f.setItalic(true);
        this->setTextColor(1,QColor("#808080"));
        this->setFont(1, f);

        this->setText(1, nameLabelText);
        this->setCheckState(1, Qt::Unchecked);
    }
}


/**
 * @brief PPTreeWidgetItem::updatePlotVisibilityIcon draws a black or a gray eye-icon,
 * dependent on the plugin's plot visibility
 */
void PPTreeWidgetItem::updatePlotVisibilityIcon()
{
    if(m_plugin->plotVisibility())
    {
        QIcon ic(Core::rootDir + "resources/icons/visible1_32.png");
        this->setIcon(0,ic);
    }
    else
    {
        QIcon ic(Core::rootDir + "resources/icons/visible1gray_32.png");
        this->setIcon(0,ic);
    }
}


void PPTreeWidgetItem::handleDirtyStateChanged(bool state)
{  
    // this routine is called if dirty state is changed
    // if processing is finisehd updateNameLabel() is called and overwrittes background colors (mostly dummies)
    // then updateItemColors() checks for is PLugins are MSA or valid/unvalid and overwrites remaining plugins

    //qDebug() << "PPTreeWidgetItem: handle" << m_plugin->getParameterPreview() << state << m_plugin->processingError();

    if(!m_plugin->activated())
    {
        setBackgroundColor(0,Qt::white);
        setBackgroundColor(1,Qt::white);
        setBackgroundColor(2,Qt::white);
        setBackgroundColor(3,Qt::white);
        return;
    }

    if(state)
    {
        QColor pcolor_orange    = QColor(244,176,15);

        setBackgroundColor(0, pcolor_orange);
        setBackgroundColor(1, pcolor_orange);
        setBackgroundColor(2, pcolor_orange);
        setBackgroundColor(3, pcolor_orange);
    }
    else
    {        
        setBackgroundColor(0,Qt::white);
        setBackgroundColor(1,Qt::white);
        setBackgroundColor(2,Qt::white);
        setBackgroundColor(3,Qt::white);
    }
}


void PPTreeWidgetItem::updateLinkStateLabel()
{
    switch(m_plugin->linkState())
    {
        case -1: // no link

            setText(2, "" );
            setToolTip(2,"This independent processing step belongs only to this measurement run");
            break;

        case 0: // linked, individual parameters per run

            if(m_plugin->ppc())
            {
                setText(2, QString("S (%0)").arg(m_plugin->ppc()->id() ) );
                setToolTip(2,QString("Single - (linked, individual parameters) - Link-ID = %0\nThis processing step is linked to other measurement runs, but parameters are set individually for each measurement run.")
                           .arg(m_plugin->ppc()->id()));
            }
            break;


        case 1: // linked, same parameters in group

            if(m_plugin->ppc())
            {
                setText(2, QString("G (%0)").arg(m_plugin->ppc()->id() ) );
                setToolTip(2,QString("Group - (linked, group parameters) - Link-ID = %0\n This processing step is linked to other measurement runs, but parameters are set individually for each group.")
                           .arg(m_plugin->ppc()->id()));
            }           
            break;


        case 2: // linked, same parameters for all runs (global)

            if(m_plugin->ppc())
            {
                setText(2, QString("A (%0)").arg(m_plugin->ppc()->id() ) );
                setToolTip(2,QString("All - (linked, global parameters) - Link-ID = %0\nThis processing step is linked to other measurement runs and parameters are set globally.")
                           .arg(m_plugin->ppc()->id()));
            }
    }
}


void PPTreeWidgetItem::deleteData()
{
    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot remove processing step (active background tasks!)";
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
        return;
    }
    m_plugin->processingChain()->removePlug(m_plugin->position());
}


void PPTreeWidgetItem::handleClick(int col)
{
    // click on column 0 means the eye icon has been clicked
    if(col == 0)
    {
        // toggle visibility
        bool state = m_plugin->plotVisibility();
        m_plugin->setPlotVisibility(!state);
    }
}


/**
 * @brief PPTreeWidgetItem::handleDrop
 * @param items
 * @param dropAbove drop items above this item (default false)
 */
void PPTreeWidgetItem::handleDrop(QList<QTreeWidgetItem *> items, bool dropAbove)
{
    if(!m_plugin)
        return;

    for(int i = 0; i < items.size(); i++)
    {
        QTreeWidgetItem* item = items[i];

        int udat = item->data(0,Qt::UserRole).toInt();
        int did = item->data(0,Qt::UserRole+1).toInt();

        if(udat == 4)  // another processingplugin has been dropped on this plugin
        {
            ProcessingChain* pchain = m_plugin->processingChain();

            DataItem* ch = pchain->findChildById(did);
            if(!ch)continue;

            ProcessingPlugin* p = dynamic_cast<ProcessingPlugin*>(ch);
            if(!p)continue;

            delete treeWidget()->takeTopLevelItem(
                        treeWidget()->indexOfTopLevelItem(item));

            if(dropAbove)
                pchain->movePlugin(p->position(), m_plugin->position(),true);
            else
                 pchain->movePlugin(p->position(), m_plugin->position());
        }
    }
}
