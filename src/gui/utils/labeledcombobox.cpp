#include "labeledcombobox.h"
#include "../../database/databasecontent.h"

const QString LabeledComboBox::identifier_fromRun = "-> from current Run";

/**
 * @brief LabeledComboBox::LabeledComboBox
 * @param parent parent widget (default = 0)
 */
LabeledComboBox::LabeledComboBox(QWidget *parent) : PluginInputField(parent)
{
    choices = new QComboBox;
    init();
}


/**
 * @brief LabeledComboBox::LabeledComboBox
 * @param descr description text
 * @param hStretch horizontal stretch parameter
 * @param parent parent widget (default = 0)
 */
LabeledComboBox::LabeledComboBox(QString descr,bool hStretch, QWidget *parent) : PluginInputField(parent)
{
    choices = new QComboBox;    
    init(hStretch);
    description->setText(descr);
}


/**
  * @brief LabeledComboBox::init overloaded virtual function from PluginInputField
  */
 void LabeledComboBox::init()
 {
     init(true);
 }


/**
 * @brief initialize GUI
 */
void LabeledComboBox::init(bool hStretch)
{
    useDataBaseContent = false;
    initDbContent = false;
    fromRunEntryEnabled = false;
    dbc = NULL;

    layMainH = new QHBoxLayout;
    layMainH->setMargin(0);
    setLayout(layMainH);

    description = new QLabel;

    layMainH->addWidget(description);
    layMainH->addWidget(choices);

    if(!hStretch)
        layMainH->addStretch(-1);

    connect(choices,SIGNAL(currentIndexChanged(int)),this,SLOT(onCurrentIndexChanged(int)));
}


/**
 * @brief enables the DatabaseContent support
 * @param List of DatabaseContent pointers
 */
void LabeledComboBox::setDatabaseContent(QList<DatabaseContent*> *d)
{
    useDataBaseContent = true;
    dbc = d;
}


/**
 * @brief returns selected DatabaseContent
 * @return selected DatabaseContent or NULL (if no DatabaseContent has been added)
 */
DatabaseContent* LabeledComboBox::getSelectedDbContent()
{
    if(!useDataBaseContent)
        return NULL;

    // return NULL pointer if no selection has been made !!!
    if(choices->currentIndex()== -1 || choices->count() == 0 || choices->currentText() == identifier_fromRun)
    {
        return NULL;
    }
    return dbc->at(choices->currentIndex());
}


/**
 * @brief get index of current selected item
 * @return index of selected item
 */
int LabeledComboBox::getCurrentIndex()
{
    return choices->currentIndex();
}


/**
 * @brief LabeledComboBox::getCurrentText get text of selected item
 * @return currentText()
 */
QString LabeledComboBox::getCurrentText()
{
    return choices->currentText();
}


void LabeledComboBox::setCurrentItem(DatabaseContent *dbi)
{
    if(!useDataBaseContent || dbc == NULL || dbi == NULL)
        return;

    int idx = -1;
    for(int i=0; i<dbc->size();i++)
        if(dbc->at(i)->filename == dbi->filename)
            idx = i;

    if(idx != -1)
        choices->setCurrentIndex(idx);
}


void LabeledComboBox::setCurrentItem(int dbc_ident)
{
    if(!useDataBaseContent || dbc == NULL)
        return;
    int idx = -1;
    for(int i=0; i<dbc->size();i++)
        if(dbc->at(i)->ident == dbc_ident)
            idx = i;
    if(idx != -1)
        choices->setCurrentIndex(idx);
}


void LabeledComboBox::setCurrentIndex(int i)
{
    choices->setCurrentIndex(i);
}


/**
 * @brief adds an item to the choices list (only works if no databasecontent has been set!)
 * @param itemname
 */
void LabeledComboBox::addStringItem(QString itemname)
{
    if(useDataBaseContent)
    {
        qDebug() << "LabeledComboBox.addStringItem: cannot add item!";
        return;
    }
    choices->addItem(itemname);
}


/**
 * @brief LabeledComboBox::clearAll removes all items from the combobox
 */
void LabeledComboBox::clearAll()
{
    choices->clear();
}


void LabeledComboBox::enableFromRunEntry(bool enabled)
{
    fromRunEntryEnabled = enabled;
    slot_onDBcontentChanged();
}


bool LabeledComboBox::fromRunSelected()
{
    return choices->currentText() == identifier_fromRun;
}


/**
 * @brief reset choices list if the database content has changed
 */
void LabeledComboBox::slot_onDBcontentChanged()
{
    if(!useDataBaseContent)
        return;

//    initDbContent = true;


    int lastChoiceIdx = choices->currentIndex();

    QString lastFname = choices->currentData().toString();

    choices->clear();

    for(int i=0; i<dbc->size();i++)
    {
        choices->addItem(dbc->at(i)->name,dbc->at(i)->filename);

        // last selection is still in list => select that index
        if(lastFname == dbc->at(i)->filename)
            lastChoiceIdx = i;
    }

    if(fromRunEntryEnabled)
        choices->addItem(identifier_fromRun);

    // reselect the last index
    if(lastChoiceIdx>= choices->count())
        lastChoiceIdx = choices->count()-1;

    if(choices->count() >= 0 )
        choices->setCurrentIndex(lastChoiceIdx);

    initDbContent = false;

    // notify listener that the selection has has changed
    emit currentIndexChanged(lastChoiceIdx);

}

void LabeledComboBox::onCurrentIndexChanged(int idx)
{

    if(useDataBaseContent && dbc->size() == 0)
        return;

    if(!initDbContent)
        emit currentIndexChanged(idx);

    emit parameterChanged();
}



// implementation of the PluginInputField interface

void LabeledComboBox::setPluginParamValue(QVariant value)
{
    QStringList choices = value.toString().split(";");
    if(choices.size() == 1)
    {
        QString choice = choices.first();
        for(int i = 0; i < this->choices->count();i++)
        {
            if(this->choices->itemText(i)==choice)
            {
                this->choices->setCurrentIndex(i);
            }
        }
    }
    else if(!choices.isEmpty())
    {
        QString choice = choices.first();
        choices.removeFirst();

        for(int i = 0; i < choices.size(); i++)
        {
            addStringItem(choices.at(i));
        }
        int idx = choices.indexOf(choice);
        if(idx >= 0)
            setCurrentIndex(idx);
    }
    //field->setValue(value.toDouble());
}


void LabeledComboBox::setPluginParamMinValue(QVariant minValue)
{
    // actually not needed for combobox
}


void LabeledComboBox::setPluginParamMaxValue(QVariant maxValue)
{
   // actually not needed for combobox
}


void LabeledComboBox::setPluginLabelText(QString labelText)
{
    description->setText(labelText);
}


void LabeledComboBox::setPluginLabelList(QList<QString> labels)
{
    // not needed in this class
}

void LabeledComboBox::setPluginValueList(QList<QVariant> values)
{
    // not needed in this class
}


QVariant LabeledComboBox::getPluginParamValue()
{
    if(choices->currentIndex() == -1) return 0;

    QString res = choices->itemText(choices->currentIndex()) /*  + ";"  */;

    // do only return the currently selected item
    // (instead of whole list!)
    /*
    QString res = choices->itemText(choices->currentIndex()) + ";";
    for(int i=0; i < choices->count(); i++)
    {
        res.append(choices->itemText(i));
        if(i != choices->count()-1)
            res.append(";");
    }*/
    return res;
}
