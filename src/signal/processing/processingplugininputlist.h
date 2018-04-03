#ifndef PROCESSINGPLUGININPUTLIST_H
#define PROCESSINGPLUGININPUTLIST_H

#include <QList>
#include "processingplugininput.h"

/**
 * @brief The ProcessingPluginInputList class represents a list of
 * ProcessingPluginInputs.
 * @ingroup Signal-Processing
 * @details
 */
class ProcessingPluginInputList : public QList<ProcessingPluginInput>
{
public:
    QVariant getValue( QString identifier );
    ProcessingPluginInput* getPluginInput(QString identifier);
    void setValue(QString identifier, QVariant v);
    void setEnabled(QString identifier, bool state);
    QString toString() const;
    void showGroup(int id);
};


#endif // PROCESSINGPLUGININPUTLIST_H
