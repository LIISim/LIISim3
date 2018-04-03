#include "processingplugininputlist.h"

/**
 * @brief getValue gets value of input field with given identifier
 * @param identifier identifier of ProcessingPluginInput
 * @return value in the form of a QVariant object
 */
QVariant ProcessingPluginInputList::getValue( QString identifier )
{
    for(int i=0; i<size(); i++){
        if(at(i).identifier == identifier)
        {
            if(at(i).type == ProcessingPluginInput::COMBOBOX)
            {
                QString v = at(i).value.toString();
                QStringList res = v.split(";");
                if(res.isEmpty())return "";
                return res.first();
            }
            else
                return at(i).value;
        }

    }
    return "notSet";
}


ProcessingPluginInput* ProcessingPluginInputList::getPluginInput(QString identifier)
{
    for(int i=0; i<size(); i++){
        if(at(i).identifier == identifier)
        {
            return & this->operator[](i);
        }
    }
    return  NULL;
}


void ProcessingPluginInputList::setValue(QString identifier, QVariant v)
{
    for(int i=0; i<size(); i++){
        if(at(i).identifier == identifier)
        {
            this->operator [](i).value = v;
        }
    }
}


/**
 * @brief ProcessingPluginInputList::setEnabled Sets the Enabled-State
 * of an input field.
 * @param identifier Identifier of Input Field
 * @param state new enabled state
 */
void ProcessingPluginInputList::setEnabled(QString identifier, bool state)
{
    for(int i=0; i<size(); i++)
        if(at(i).identifier == identifier)
        {
            this->operator [](i).enabled = state;
            break;
        }
}


QString ProcessingPluginInputList::toString() const
{
    QString res = "PPIList: ";

    for(int i=0; i<size(); i++){
        res.append( at(i).toString() );
    }
    return res;
}


/**
 * @brief ProcessingPluginInputList::showGroup shows shows all ProcessingPluginInput with groupID == id, ignore groupID = 0;
 * @param id
 */
void ProcessingPluginInputList::showGroup(int id)
{
     for(int i=0; i<size(); i++)
     {
        if(at(i).groupID == id)
            this->operator [](i).visible = true;
        else if(at(i).groupID != 0)
            this->operator [](i).visible = false;
     }
}


