#include "processingplugininput.h"


/**
 * @brief ProcessingPluginInput::ProcessingPluginInput Constructor
 */
ProcessingPluginInput::ProcessingPluginInput()
{
    groupID = 0; // if groupID = 0 visibility will not be toggled
    visible = true;
    enabled = true;
    limits  = true;
    tooltip = "";
}


/**
 * @brief ProcessingPluginInput::setTypeFromInt set the
 * Type of the ProcessingPluginInput from integer representation
 * @param i integer representation of type
 */
void ProcessingPluginInput::setTypeFromInt(int i)
{
    switch (i)
    {
        case 0:
            type = DOUBLE_FIELD; break;
        case 1:
            type = INTEGER_FIELD; break;
        case 2:
            type = CHECKBOX; break;
        case 3:
            type = COMBOBOX; break;
        case 4:
            type = NOGUI; break;
        case 5:
            type = CHECKBOX_GROUP; break;
    }
}


/**
 * @brief ProcessingPluginInput::typeToString return string representation of Type
 * @return QString
 */
QString ProcessingPluginInput::typeToString() const
{
    switch (type)
    {
        case DOUBLE_FIELD:
            return "0"; break;
        case INTEGER_FIELD:
            return "1"; break;
        case CHECKBOX:
            return "2"; break;
        case COMBOBOX:
            return "3"; break;
        case NOGUI:
            return "4"; break;
        case CHECKBOX_GROUP:
            return "5"; break;
    }
}


QString ProcessingPluginInput::toString() const
{
    QString res = QString("(%0: %1)")
            .arg( labelText)
            .arg(value.toString() );
    return res;
}

