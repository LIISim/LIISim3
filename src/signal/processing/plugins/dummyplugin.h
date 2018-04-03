#ifndef TPCDUMMY_H
#define TPCDUMMY_H

#include "../processingplugin.h"

/**
 * @brief The DummyPlugin class
 */
class DummyPlugin : public ProcessingPlugin
{
    Q_OBJECT

private:

    QString dummyText;

public:

    explicit DummyPlugin(ProcessingChain *parentChain, QString  dummytext);
    ~DummyPlugin();

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

signals:

public slots:
};

#endif // TPCDUMMY_H
