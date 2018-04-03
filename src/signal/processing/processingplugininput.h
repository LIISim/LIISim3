#ifndef PROCESSINGPLUGININPUT_H
#define PROCESSINGPLUGININPUT_H


#include <QVariant>
#include <QString>

/**
 * @brief The ProcessingPluginInput class defines an input field for
 * any ProcessingPlugin.
 * @ingroup Signal-Processing
 * @details This class is used to store informations about the current value
 * and minimum/maximum input values and a description text for that input field.
 */
class ProcessingPluginInput
{
public:

    ProcessingPluginInput();


    /** @brief available input types */
    enum Type { DOUBLE_FIELD, INTEGER_FIELD, CHECKBOX, COMBOBOX, NOGUI, CHECKBOX_GROUP};

    void setTypeFromInt(int i);

    QString typeToString() const;

    QString toString() const;

    /** @brief groupID Input group for toggling visibility of parameter groups
     */
    int groupID;

    /** @brief flag indicating if this input field should be visible in GUI */
    bool visible;

    /** @brief flag, enables/disables editing of the input field */
    bool enabled;

    /** @brief input type */
    Type type;

    /** @brief identifier of input field, should be unique within plugin! */
    QString identifier;

    /** @brief holds text for label (shown in user interface) */
    QString labelText;

    /** @brief holds text for tooltip (allows more detailed desciption of parameter) */
    QString tooltip;

    /** @brief holds current value of ProcessingPluginInput */
    QVariant value;

    /** @brief limits true if field has limits*/
    bool limits;

    /** @brief minimum value (used for DOUBLE_FIELD and INTEGER_FIELD) */
    QVariant minValue;

    /** @brief maximum value (used for DOUBLE_FIELD and INTEGER_FIELD) */
    QVariant maxValue;

    /** @brief checkboxGroupValues (used to save CHECKBOX_GROUP values)    */
    QList<QVariant> checkboxGroupValues;

    /** @brief checkboxGroupValues (used to save CHECKBOX_GROUP label text)    */
    QList<QString> checkboxGroupLabels;
};

#endif // PROCESSINGPLUGININPUT_H
