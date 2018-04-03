#ifndef MEMUSAGEWIDGET_H
#define MEMUSAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>

#include "../utils/ribbontoolbox.h"

/**
 * @brief The MemUsageWidget class is a widget which visualizes
 * the current memory usage. Usage statistics are updated during
 * signal processing and data import. The Widget also allows
 * to bypass the memory check before signal processing.
 */
class MemUsageWidget : public RibbonToolBox
{
    Q_OBJECT

public:

    explicit MemUsageWidget(QWidget *parent = 0);
    ~MemUsageWidget();

private:

    QLabel* info;
    QCheckBox* bypass;

signals:

public slots:

private slots:

    void onBypassStateChanged(int state);
    void onGuiSettingsChanged();
    void updateInfoText();
};

#endif // MEMUSAGEWIDGET_H
