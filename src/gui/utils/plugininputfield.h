#ifndef PLUGININPUTFIELD_H
#define PLUGININPUTFIELD_H

#include <QVariant>
#include <QWidget>
#include <QString>
#include "../../signal/processing/processingplugininputlist.h"

/**
 * @brief The PluginInputField class defines an interface
 * which allows derived classes to serve as an inputfield
 * for ProcessingPlugins
 * @ingroup GUI-Utilities
 */
class PluginInputField : public QWidget
{
    Q_OBJECT

public:
    explicit PluginInputField(QWidget *parent = 0) : QWidget(parent){}

    /** @brief identifier of the plugin's input */
    QString pluginParamIdentifier;

    /** @brief inputtype of the plugin parameter */
    ProcessingPluginInput::Type pluginInputType;

    /** @brief inititalize plugin */
    virtual void init() = 0;

    /** @brief set the plugin parameter value */    
    virtual void setPluginParamValue(QVariant value) = 0;

    /** @brief set the plugin parameter minimum value */
    virtual void setPluginParamMinValue(QVariant minValue) = 0;

    /** @brief set the plugin parameter maximum value */
    virtual void setPluginParamMaxValue(QVariant maxValue) = 0;

    /** @brief set label text */
    virtual void setPluginLabelText(QString labelText) = 0;

    /** @brief set list of labels (used by CHECKBOX_GROUP) */
    virtual void setPluginLabelList(QList<QString> labels) = 0;

    /** @brief set list of values (used by CHECKBOX_GROUP)  */
    virtual void setPluginValueList(QList<QVariant> values) = 0;

    /** @brief get the current value from */
    virtual QVariant getPluginParamValue() = 0;

    /** @brief visibility in processingplugintreewidget */
    bool visible;

signals:

    /** @brief signal wich an input field
      * emits if the user has changed the field's content */
    void parameterChanged();
};


#endif // PLUGININPUTFIELD_H
