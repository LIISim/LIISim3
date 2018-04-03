#include "processingplugin.h"

#include "../../general/LIISimException.h"
#include "../../core.h"
#include "../mrun.h"
#include "processingchain.h"
#include "processingpluginconnector.h"
#include "plugins/multisignalaverage.h"
#include "plugins/temperaturecalculator.h"
#include "pluginfactory.h"
#include "ppstepbuffer.h"

/**
 * @brief ProcessingPlugin::ProcessingPlugin Constructor
 * @param stype   type of signal which should be processed
 * @param parent  parent object (default = 0)
 */
ProcessingPlugin::ProcessingPlugin(ProcessingChain *parentChain)
    : DataItem(parentChain)
{
    this->m_pchain = parentChain;
    this->mrun  = parentChain->mrun();
    this->stype = parentChain->getSignalType();

    stepBufferFlag = true;
    m_dirty       = true;
    executeSyncronized  = false;
    mProcessingError = false;
    m_activated = true;
    positionInChain     = -1;

    stepBuffer = new PPStepBuffer;
    stepBuffer->data.resize(boost::extents[mrun->sizeAllMpoints()][channelCount()]);
    QList<int> chids = mrun->channelIDs(stype);
    for(int i = 0; i < chids.size(); i++)
    {
        m_chid_to_bufferidx.insert(chids[i],i);
    }
    p_validations.fill(0, mrun->sizeAllMpoints());
    p_calculatedMPts = 0;

    m_data.append(metaObject()->className());
    m_data.append(m_activated);
    m_data.append(m_dirty);

    m_linkState = -1;
    m_previousLinkState = 0;
    m_ppc = 0;

    m_plotVisibility = true; // default visibility

    preserveStdev = false;
}

ProcessingPlugin::~ProcessingPlugin()
{
    delete stepBuffer;
}



/**
 * @brief ProcessingPlugin::getStype get signaltype which has been applied to this plugin
 * @return  Signal::SType
 */
Signal::SType ProcessingPlugin::getStype()
{
    return stype;
}


/**
 * @brief ProcessingPlugin::getInputs get inputs for ProcessingPlugin
 * @return List of ProcessingPluginInputs
 */
ProcessingPluginInputList ProcessingPlugin::getInputs()
{
    return inputs;
}


/**
 * @brief ProcessingPlugin::getShortDescription
 * @return short information message about the plugin
 */
QString ProcessingPlugin::getShortDescription()
{
    return shortDescription;
}


/**
 * @brief ProcessingPlugin::linkType
 * @return
 */
int ProcessingPlugin::linkType()
{
    if(m_ppc)
        return m_ppc->pluginLinkType();
    else
        return 0;
}


/**
 * @brief ProcessingPlugin::linkID return the id of the ProcessingpluginConnector
 * of this plugin or -1 if there is no connector
 * @return
 */
int ProcessingPlugin::linkID()
{
    if(m_ppc)
        return m_ppc->id();
    else
        return -1;
}


/**
 * @brief ProcessingPlugin::setParameters plugin parameters from a list of ProcessingPluginInpus.
 * This method emits the dataChanged signal (position 3).
 * @param inputs
 * @details Checks if the given inputs are valid. Derived classes should
 * implement the setFromInputs() method to assign individual member parameters.
 */
void ProcessingPlugin::setParameters(const ProcessingPluginInputList &newInputs)
{
    // check if the given input list contains the the same inputs as member list!
    //if(this->inputs.size() != newInputs.size())
    //    throw LIISimException("ProcessingPlugin::setInputs ("+getName()+ "): invalid inputs!");

    ProcessingPluginInputList newInputsCorrected = newInputs;

    for(int i = 0; i < newInputs.size(); i++)
    {
        if(i >= this->inputs.size())
        {
            throw LIISimException("ProcessingPlugin::setInputs ("
                                  + getName()+ "): no matching input field "
                                  + newInputs.at(i).identifier
                                  + " for Processing step of run '"
                                  + this->mrun->getName() +"'");
        }

        if(newInputs.at(i).identifier != this->inputs.at(i).identifier ||
           newInputs.at(i).type       != this->inputs.at(i).type )
        {
            QString t = newInputs.at(i).typeToString();
            QString t0 = this->inputs.at(i).typeToString();
            throw LIISimException("ProcessingPlugin::setInputs ("
                                  + getName()+ "): no matching input field "
                                  + newInputs.at(i).identifier
                                  + " for type " + t + " "
                                  + this->inputs.at(i).identifier);


        }

        // if current processingplutiginput is a combobox, do not overwrite
        // the current input's selection options.
        // Instead do only copy the selected value (first item in value string)
        // TODO: pretty ugly !!!
        if(newInputs.at(i).type == ProcessingPluginInput::COMBOBOX)
        {
            QStringList instrlist = inputs.at(i).value.toString().split(";");
            QStringList newstrlist = newInputs.at(i).value.toString().split(";");

            if(instrlist.size() == 0 || newstrlist.size() == 0)
                continue;

            QString new_cbval = newstrlist.at(0);

            for(int j = 1; j < instrlist.size(); j++)
                new_cbval.append(QString(";%0").arg(instrlist.at(j)));

            QString ident = newInputs.at(i).identifier;
            newInputsCorrected.setValue(ident,new_cbval);

        }

    }

    // assign inputs
    this->inputs = newInputsCorrected;

    setFromInputs();

    // set dirty
    setDirty(true);
    emit dataChanged(3,true);

}


/**
 * @brief ProcessingPlugin::reset reset plugin state after all signals have been processed
 * @details this slot is called after all processing tasks have have been finished processing
 */
void ProcessingPlugin:: reset()
{
    p_calculatedMPts =0;

    // clear step buffer data if flag is set
    if(!stepBufferFlag)
        cleanupStepBuffer();

    setDirty(false);
}


void ProcessingPlugin::cleanupStepBuffer()
{
    int n1 = stepBuffer->data.shape()[0];
    int n2 = stepBuffer->data.shape()[1];

    for(int m = 0; m < n1; m++)
        for(int c = 0; c < n2; c++)
            stepBuffer->data[m][c].data.clear();

   // stepBuffer->data.resize(boost::extents[mrun->sizeAllMpoints()][channelCount()]);
    stepBuffer->data.resize(boost::extents[0][0]);

   // delete stepBuffer;
  //  stepBuffer = new PPStepBuffer;
}


void ProcessingPlugin::initializeCalculation()
{
    if((stepBuffer->data.shape()[0] != mrun->sizeAllMpoints()) ||
        stepBuffer->data.shape()[1] != channelCount())
    {
        stepBuffer->data.resize(boost::extents[mrun->sizeAllMpoints()][channelCount()]);
        p_validations.resize(mrun->sizeAllMpoints());

        m_chid_to_bufferidx.clear();
        QList<int> chids = mrun->channelIDs(stype);
        for(int i = 0; i < chids.size(); i++)
        {
            m_chid_to_bufferidx.insert(chids[i],i);
        }
    }

    p_validations.fill(0);

    //Assign empty step buffer signals, can be skipped to increase calculation performance
    /*
    Signal s;
    for(int m = 0; m < mrun->sizeAllMpoints(); m++)
        for(int c = 0; c < channelCount(); c++)
        {
            stepBuffer->data[m][c] =s;
        }
     */
}


bool ProcessingPlugin::processSignal(const Signal & in, Signal & out, int mpIdx)
{
    bool ret = processSignalImplementation(in, out, mpIdx);

    /**
     * --- PRESERVE STANDARD DEVIATION ---
     * standard deviation container is cleared after each processing
     * step, if stdev cannot be calculated by the plugin
     *
     * preserveStdev = true:
     *      allows to keep the stdev if processing step if the values of stdev are
     *      not changing as for example additive arithmetic operations
     *      (i.e., SimpleSignalReducer, GetSection, Baseline, SwapChannels,...)
     */

    if(!preserveStdev)
        out.stdev.clear();

    return ret;
}


/**
 * @brief ProcessingPlugin::setActivated activates or deactivates this plugin.
 * If a plugin is deactivated, it will be skipped during calculation.
 * This method emits the dataChanged signal (position 1).
 * @param activated
 */
void ProcessingPlugin::setActivated(bool activated)
{
    if(mrun->isBusy())
    {
        qDebug() << "ProcessingPlugin::setActivated: cannot change state during calculation";
        return;
    }
    if(m_activated == activated)
        return;

    m_activated = activated;
    if(!m_pchain)return;
    int nextPos = this->position()+1;
    if( nextPos < m_pchain->childCount())
        m_pchain->getPlug(nextPos)->setDirty(true);
    emit dataChanged(1, m_activated);


    if(m_plotVisibility && !activated)
    {
        setPlotVisibility(false);
    }
    else if(activated && !m_plotVisibility)
    {
        setPlotVisibility(true);
    }
    if(activated)
        setDirty(true);;
}


/**
 * @brief ProcessingPlugin::setDirty sets the parameters of this plugin dirty.
 * This ensures that the signal data of this plugin is recalculated during next
 * signal processing operation.
 * This method emits the dataChanged signal (position 2).
 * @param dirty
 */
void ProcessingPlugin::setDirty(bool dirty)
{
    m_dirty = dirty;

    if(!m_pchain)return;
    int nextPos = this->position()+1;
    if( nextPos < m_pchain->childCount())
        m_pchain->getPlug(nextPos)->setDirty(true);
    emit dataChanged(2, m_dirty);
}


/**
 * @brief ProcessingPlugin::setStepBuffering
 * @param state
 */
void ProcessingPlugin::setStepBufferEnabled(bool state)
{
   stepBufferFlag = state;

   // clear step buffer data if flag is set
   if(!stepBufferFlag)
   {
       cleanupStepBuffer();
       setPlotVisibility(false);
   }

   emit dataChanged(6, stepBufferFlag);

}


/**
 * @brief ProcessingPlugin::setLinkState set state of plugin link.
 * This method emits the dataChanged signal (position 4).
 * Possible scenarios: "process all" -> "process group" or "process single mrun"
 * and "process group" to "process single mrun"
 * TODO: add checks, and implementation in PluginConnector!
 * @param t
 */
void ProcessingPlugin::setLinkState(int t)
{
    //qDebug() << this << "setLinkState " << m_linkState << "->" << t;

    if(t == m_linkState)
        return;

    m_previousLinkState = m_linkState;
    m_linkState = t;

    // create a ppc!
    if(!m_ppc && m_linkState > -1)
    {
        m_ppc = new ProcessingPluginConnector(this);

    }

    if(m_linkState == -1)
        m_ppc = 0;

    emit dataChanged(4,t);
}


/**
 * @brief ProcessingPlugin::setPlotVisibility sets visibility of plugin in plot.
 * This method emits the dataChanged signal (position 5).
 * @param value new value
 */
void ProcessingPlugin::setPlotVisibility(bool value)
{
    if(value && (!m_activated || !stepBufferFlag))
        return;
    m_plotVisibility = value;
    emit dataChanged(5,value);
}



bool ProcessingPlugin::busy()
{
    bool res = false;

   // if(m_activated)
        res = p_calculatedMPts < mrun->sizeAllMpoints();
  /*  else
    {
        ProcessingPlugin* prev = m_pchain->getPlug(positionInChain-1);
        if(prev)
            return prev->busy();
    }*/

    return res;
}


Signal ProcessingPlugin::processedSignal(int mPoint, int channelID)
{
    Signal s;
    if(mPoint < 0 || mPoint >= stepBuffer->data.shape()[0])
        return s;
    if(!mrun->isValidChannelID(channelID, stype))
        return s;

    int bidx = m_chid_to_bufferidx.value(channelID, -1);

    if(stepBuffer->data.shape()[1] < bidx || bidx == -1)
        return s;

    //boost::multi_array<Signal,2>::index mMax = p_stepBuffer.index_bases()[0]+ p_stepBuffer.shape();

    return stepBuffer->data[mPoint][m_chid_to_bufferidx[channelID]];
}


Signal ProcessingPlugin::processedSignalPreviousStep(int mPoint, int channelID)
{
    Signal s;
    ProcessingPlugin* prev = m_pchain->getPlug(positionInChain-1);
    if(prev)
        return prev->processedSignal(mPoint,channelID);
    else
    {
        MPoint* mp = mrun->getPre(mPoint);
        if(!mp)return s;
        return mp->getSignal(channelID,stype);
    }
}


bool ProcessingPlugin::validAt(int mPoint)
{
    bool result = false;
    int noCh = channelCount();

    //if(stype == Signal::TEMPERATURE)
    //    noCh = 1;

    // is valid if all channels (noCh) pass validation
    result = (p_validations.value(mPoint,0) == noCh);


    //qDebug() << "ProcessingPlugin::validAt " << positionInChain << m_activated << result << p_validations.value(mPoint,0);
    return result;
}


bool ProcessingPlugin::validAtPreviousStep(int mPoint)
{
    ProcessingPlugin* prev = m_pchain->getPlug(positionInChain - 1);
    if(prev)
    {
        return prev->validAt(mPoint);
    }
    else
        return true;
}


void ProcessingPlugin::processMPoints(int mStart, int mEnd)
{

   /* QList<QVariant> procUpdate0;
    procUpdate0 << 0;
    procUpdate0 << (int)this->thread();
    procUpdate0 << mrun->id();
    procUpdate0 << m_pchain->id();
    procUpdate0 << this->id();
    procUpdate0 << mStart;
    procUpdate0 << mEnd;
    procUpdate0 << QTime::currentTime();*/


    int p = positionInChain;
    QList<int> chids = mrun->channelIDs(stype);
    int noCh = channelCount();

    int noMpts = mrun->sizeAllMpoints();

    if(noMpts != stepBuffer->data.shape()[0] ||
       noMpts != p_validations.size())
    {
        qDebug() << "ProcessingPlugin::processMPoints ERROR: wrong step buffer dimensions: " + this->getName();
        return;
    }

    bool msaMode = (m_pchain->msaPosition() > -1 && positionInChain >= m_pchain->msaPosition());

    ProcessingPlugin* prev = m_pchain->getPlug(p-1);


    if( mStart < 0 || mEnd >= noMpts || mStart > mEnd )
    {
        QString msg;
        msg.sprintf("ProcessingPlugin::processMPoints: invalid MPoint range: %d to %d",mStart, mEnd);

        throw LIISimException(msg);
    }

    /*
     * we do not have to care about sync here.
     * chains containing plugins, which require sync, are
     * executed single threaded! (see ProcessingTask::checkMT())
     *
    if(prev && executeSyncronized)
    {
        while(prev->busy())
        {
            // actually do nothing
        }
    }*/

    int newEnd = mEnd;
    if(msaMode && mStart == 0)
    {
        newEnd = mStart;
    }

    if(msaMode && mStart > 0)
    {

    }
    else
    {
        // process mPoints
        for(int m = mStart; m <= newEnd; m++)
        {
           /* QList<QVariant> procUpdate1;
            procUpdate1 << 1;
            procUpdate1 << (int)this->thread();
            procUpdate1 << mrun->id();
            procUpdate1 << m_pchain->id();
            procUpdate1 << this->id();
            procUpdate1 << mStart;
            procUpdate1 << mEnd;
            procUpdate1 << QTime::currentTime();*/

            bool prevValid = validAtPreviousStep(m);
            int bufcidx;

            for(int c = 0; c < chids.size(); c++)
            {
                bufcidx = m_chid_to_bufferidx[chids[c]];
                Signal si;

                if(prev)
                {
                    si = prev->stepBuffer->data[m][bufcidx];
                }
                else
                {
                    MPoint* mpre = mrun->getPre( m );
                    si = mpre->getSignal(chids[c],stype);
                }

                Signal so = si;

                // clear output data container
                so.data.clear();
                so.stdev.clear();

                bool res = false;

                res = true;

                // temperature calculators ignore previous validation
                if(m_activated && getName() == TemperatureCalculator::pluginName)
                {
                    res = processSignal(si, so, m);
                }
                else if(m_activated && prevValid)
                    res = processSignal(si, so, m);
                else
                {
                    res = prevValid;
                    so = si;
                }

                // squeeze QVector to release unused memory
                so.data.squeeze();

                stepBuffer->data[m][bufcidx] = so;
                p_validations[m] += res;

                if(positionInChain == m_pchain->noPlugs() - 1)
                {
                    MPoint * mpost = mrun->getPost(m);
                    mpost->setSignal( so, chids[c], stype );
                }

                if(msaMode)
                    for(int mm = 1; mm < noMpts; mm++)
                       p_validations[mm] += res;
            }
        }

        int bufcidx;
        for(int m = mStart; m <= newEnd; m++)
        {
            if(p_validations[m] != noCh)
            {
                for(int c = 0;  c < chids.size(); c++)
                {
                    bufcidx = m_chid_to_bufferidx[chids[c]];
                    Signal so = stepBuffer->data[m][bufcidx];
                    so.data.clear();
                    stepBuffer->data[m][bufcidx] = so;

                    if(positionInChain == m_pchain->noPlugs() - 1)
                    {
                        MPoint * mpost = mrun->getPost(m);
                        mpost->setSignal( so, chids[c], stype );
                    }
                }
            }
            p_calculatedMPts++;
        }

        if(msaMode)
        {
            p_calculatedMPts = mEnd - mStart + 1;
        }
    }

/*    procUpdate0 << QTime::currentTime();
    emit procUpdateMessage(procUpdate0);*/
}


int ProcessingPlugin::channelCount()
{
    return mrun->getNoChannels(stype);
}


void ProcessingPlugin::printDebugTree(int pos, int level) const
{
    QString offset = "";
    for(int i = 0; i < level; i++)
    {
        offset.append("   ");
    }

    QString namestr = QString(this->metaObject()->className());

    int ltype = -1;
    if(m_ppc)
        ltype = m_ppc->pluginLinkType();

    QString state = QString("[a=%0 v=%1 lt=%2 ls=%3] ")
            .arg(m_activated)
            .arg(m_plotVisibility)
            .arg(ltype)
            .arg(m_linkState);
    QString params = inputs.toString();

    QString str = QString(offset + namestr +"(id %0) "+state+params)
        .arg(this->id());

    MSG_DETAIL_1(str);

   /* int nextlevel = level+1;

    for(int i = 0; i < m_children.size(); i++)
    {
        m_children.at(i)->printDebugTree(pos,nextlevel);
    }*/
}


unsigned long ProcessingPlugin::stepBufferSignals()
{
    return stepBuffer->numberOfSignals();
}


unsigned long ProcessingPlugin::stepBufferDataPoints()
{
    return stepBuffer->numberOfDataPoints();
}
