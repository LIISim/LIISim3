#include "dummyplugin.h"

QString DummyPlugin::descriptionFileName = "tpcdummy.html"; // TODO
QString DummyPlugin::iconFileName = "iconfile"; // TODO
QString DummyPlugin::pluginName = "Dummy";

QList<Signal::SType> DummyPlugin::supportedSignalTypes = QList<Signal::SType>();

DummyPlugin::DummyPlugin(ProcessingChain *parentChain, QString dummytext) : ProcessingPlugin(parentChain)
{
    dummyText = dummytext;
    setActivated(false);
    setStepBufferEnabled(false);
    setPlotVisibility(false);
}

DummyPlugin::~DummyPlugin()
{

}

QString DummyPlugin::getName()
{
    return pluginName;
}

void DummyPlugin::setFromInputs()
{

}

bool DummyPlugin::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    out = in;
    return true;
}

QString DummyPlugin::getParameterPreview()
{
    return dummyText;
}


