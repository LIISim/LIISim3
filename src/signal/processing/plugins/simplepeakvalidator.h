#ifndef SIMPLEPEAKVALIDATOR_H
#define SIMPLEPEAKVALIDATOR_H

#include "../processingplugin.h"


/**
 * @brief The SimplePeakValidator class
 * @ingroup ProcessingPlugin-Implementations
 */
class SimplePeakValidator : public ProcessingPlugin
{
    Q_OBJECT
public:
    explicit SimplePeakValidator( ProcessingChain *parentChain);

    /** @brief name of the description .hml file
      * @dtails the description .html file shoud be located
      * in the resources/pluginData/ directory (see PluginManager )*/
    static QString descriptionFileName;

    /** @brief name of icon file (for PluginManager)
      * @dtails the icon image file shoud be located
      * in the resources/pluginData/ directory (see PluginManager )*/
    static  QString iconFileName;


    /** @brief name of processing plugin */
    static  QString pluginName;


    /** @brief a list of signal types the plugin supports */
    static QList<Signal::SType> supportedSignalTypes;

    // implementations, overrides of virtual base class functions
    QString getName();
    bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
    void setFromInputs();

    QString getParameterPreview();
private:

    // plugin parameters
    double thresh;



};

#endif // SIMPLEPEAKVALIDATOR_H
