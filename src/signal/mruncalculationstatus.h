#ifndef MRUNSTATUS_H
#define MRUNSTATUS_H

#include <QObject>
#include <QList>
#include <QPair>

/**
 * @brief The MRunCalculationStatus class defines a signal processing
 * state for each MRun. It provides a 'success' state if signal processing
 * has been succesful and error/warning states when problems occured during
 * processing or property validation of the MRun.
 * Multiple warn/error messages are stored within a list until the status
 * is reset (automatically before calculation or manually: SE->'Reset')
 * Also see: MRun::updateCalculationStatus
 */
class MRunCalculationStatus : public QObject
{
    Q_OBJECT
public:
    explicit MRunCalculationStatus(QObject *parent = 0);

    /**
     * @brief The StateType enum
     */
    enum StateType {UNDEF, SUCCESS, PLUGCHANGED, WARN, CANCELLED, ERROR }; // possible states

    StateType currentStateType(){return m_state;}

    void addMessage(StateType state, QString message);

    void addWarining(QString message){addMessage(WARN, message);}
    void addError(QString message){addMessage(ERROR, message);}
    void addSuccessMessage(QString message){addMessage(SUCCESS, message);}
    void setPluginChanged();

    void setCancelled();
    void setProcessingFinishedSuccessful();

    void reset();
    QString toString();
    QString stateTypeToString(StateType state);

    bool isUndefined(){return m_state == UNDEF;}
    bool isSuccess(){return m_state == SUCCESS;}
    bool isWarning(){return m_state == WARN;}
    bool isCancelled(){return m_state == CANCELLED;}
    bool isPluginChanged(){return m_state == PLUGCHANGED;}
    bool isError(){return m_state == ERROR;}



private:

    /**
     * @brief m_state current state value, holds maximum state value
     * of all messages
     */
    StateType m_state;

    /**
     * @brief m_messages holds a list of messages and corresponding state
     */
    QList<QPair<StateType,QString>> m_messages;

    bool processing;

signals:

    /**
     * @brief changed This signal is emitted when the current status value
     * has changed or a new status message has been added.
     * Handled by GUI: MRunTreeWidgetItem::onRunStatusChanged()
     */
    void changed();

public slots:
};

#endif // MRUNSTATUS_H
