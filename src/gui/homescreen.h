#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>
#include <QToolBar>
#include <QSplitter>
#include <QCheckBox>
#include <QGridLayout>

#include "../logging/logmessagewidget.h"

/**
 * @brief The HomeScreen class
 * @ingroup GUI
 */
class HomeScreen : public QWidget
{
    Q_OBJECT
public:
    explicit HomeScreen(QWidget *parent = 0);
    ~HomeScreen();

    QToolBar* ribbonToolbar(){return m_ribbonToolbar;}

private:

    QToolBar* m_ribbonToolbar;
    QGridLayout* grid;
    QSplitter* splith0;

    LogMessageWidget* logview;

    QCheckBox* checkBoxLoadDataAtStartup;

    QAction *actionSaveAsDefault;

signals:
    void showTutorials();

public slots:

private slots:

    void onCheckBoxLoadDataAtStartupToggled(bool state);
    void onActionSaveAsDefaultTriggered();
    void onGeneralSettingsChanged();

    void onGeneralSettingsShowTutorial();
};

#endif // HOMESCREEN_H
