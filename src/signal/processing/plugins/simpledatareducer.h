#ifndef SIMPLEDATAREDUCER_H
#define SIMPLEDATAREDUCER_H

#include "../processingplugin.h"
#include <QList>

/**
 * @brief Example ProcessingPlugin which reduces the amount of
 * signal data points by changing the signal's time step.
 * @ingroup ProcessingPlugin-Implementations
 */
class SimpleDataReducer : public ProcessingPlugin
{
    Q_OBJECT


public:

    explicit SimpleDataReducer( ProcessingChain *parentChain);


    /** @brief name of the description .hml file
      * @dtails the description .html file shoud be located
      * in the resources/pluginData/ directory (see PluginManager )*/
    static QString descriptionFileName;

    /** @brief name of icon file (for PluginManager)
      * @dtails the icon image file shoud be located
      * in the resources/pluginData/ directory (see PluginManager )*/
    static  QString iconFileName;


    static  QString pluginName;

    static QList<Signal::SType> supportedSignalTypes;

    // implementations, overrides of virtual base class functions
    QString getName();
    bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
    void setFromInputs();


    QString getParameterPreview();

private:

    /** @brief time step factor used for data reduction */
    int dtFactor;

};

#endif // SIMPLEDATAREDUCER_H
