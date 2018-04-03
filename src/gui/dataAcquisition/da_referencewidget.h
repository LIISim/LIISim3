#ifndef DA_REFERENCEWIDGET_H
#define DA_REFERENCEWIDGET_H

#include <QObject>
#include <QMap>
#include <QCheckBox>
#include <QAction>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPair>
#include <QList>
#include <QToolButton>

#include "gui/utils/ribbontoolbox.h"
#include "signal/signal.h"
#include "signal/streampoint.h"

/**
 * @brief The DA_ReferenceElementWidget class defines a gui representation
 *        for one reference signal, containing a checkbox to enable and
 *        disable drawing of the reference and a button to remove the reference
 */
class DA_ReferenceElementWidget : public QWidget
{
    Q_OBJECT

public:
    DA_ReferenceElementWidget(QString text, QString tooltip, int channel, QColor color = QColor("black"), QWidget *parent = 0);
    ~DA_ReferenceElementWidget();

    int getCheckedState();
    QString getText();
    QString getTooltip();

    QColor color;
    int channel;

private:
    QHBoxLayout *layout;
    QCheckBox *checkboxReference;
    QToolButton *buttonRemove;

private slots:
    void onCheckboxReferenceStateChanged(int state);
    void onButtonRemoveReleased();


signals:
    void removeRequest(QWidget *widget);
    void stateChanged(int state);

};



class SignalPlotWidgetQwt;

class DA_ReferenceWidget : public RibbonToolBox
{
    Q_OBJECT

public:
    DA_ReferenceWidget(QWidget *parent = 0);
    ~DA_ReferenceWidget();

    void draw(SignalPlotWidgetQwt *plot);

    QToolButton *buttonFreeze;
    QToolButton *buttonClear;

    QList<QPair<DA_ReferenceElementWidget*, Signal>> references;

private:
    void reorganise();

    unsigned int addCounter;

public slots:
    void addReference(StreamPoint point);

private slots:
    void freezeReference();
    void clearReferences();
    void elementsChanged();
    void onRemoveRequest(QWidget *widget);

signals:
    //Emitted if freeze is clicked
    void referenceRequest();
    void shouldRedraw();

    void referencesChanged(const QList<QPair<DA_ReferenceElementWidget*, Signal>> references);

};

#endif // DA_REFERENCEWIDGET_H
