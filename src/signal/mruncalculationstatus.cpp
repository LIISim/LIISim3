#include "mruncalculationstatus.h"

MRunCalculationStatus::MRunCalculationStatus(QObject *parent) : QObject(parent)
{
    m_state = UNDEF;

    processing = false;
}

/**
 * @brief MRunCalculationStatus::addMessage add a status message to the
 * message list. The current status value will be updated if the new status
 * messeage has a higer priority (ERROR > WARN > OK > UNDEFINED)
 * @param state
 * @param message
 */
void MRunCalculationStatus::addMessage(StateType state, QString message)
{
    m_messages << QPair<StateType,QString>(state,message);
    if(state > m_state)
        m_state = state;
    emit changed();
}

/**
 * @brief MRunCalculationStatus::reset clears all status messages, sets
 * status value to 'undefined'
 */
void MRunCalculationStatus::reset()
{
    m_messages.clear();
    m_state = UNDEF;
    emit changed();
}

/**
 * @brief MRunCalculationStatus::toString string representation
 * of status messages, used for tooltips
 * @return
 */
QString MRunCalculationStatus::toString()
{
    QString res = "Calculation Status: \n";
    for(int i = 0; i < m_messages.size(); i++)
    {
        res.append(QString("%0: %1\n")
                   .arg(stateTypeToString(m_messages[i].first))
                   .arg(m_messages[i].second));
    }
    return res;
}

/**
 * @brief MRunCalculationStatus::stateTypeToString
 * @param state
 * @return
 */
QString MRunCalculationStatus::stateTypeToString(StateType state)
{
    QString res = "";
    switch(state)
    {
    case UNDEF:
        break;
    case PLUGCHANGED:
        res = "Plugin changed";
        break;
    case SUCCESS:
        res = "Ok";
        break;
    case WARN:
        res = "Warning";
        break;
    case CANCELLED:
        res = "Cancelled";
        break;
    case ERROR:
        res = "Error";
    }

    return res;
}


void MRunCalculationStatus::setCancelled()
{
    bool in = false;
    for(int i = 0; i < m_messages.size(); i++)
        if(m_messages.at(i).first == CANCELLED)
            in = true;
    if(!in)
        addMessage(CANCELLED, "Calculation cancelled.");
}


void MRunCalculationStatus::setPluginChanged()
{
    bool in = false;
    for(int i = 0; i < m_messages.size(); i++)
        if(m_messages.at(i).first == PLUGCHANGED)
            in = true;

    if(!in)
        addMessage(PLUGCHANGED, "Plugin data changed, needs to be recalculated.");
}


void MRunCalculationStatus::setProcessingFinishedSuccessful()
{
    if(m_state == PLUGCHANGED)
    {
        m_state = SUCCESS;
        for(int i = 0; i < m_messages.size(); i++)
        {
            if(m_messages.at(i).first == PLUGCHANGED)
            {
                m_messages.removeAt(i);
                i--;
            }
        }
    }
    addSuccessMessage("Calculation successful");
}

