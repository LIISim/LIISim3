#ifndef MASTERWINDOW_H
#define MASTERWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTabBar>
#include <QStackedWidget>
#include <QToolBar>
#include <QTabWidget>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>

#include "../logging/statusmessagewidget.h"
#include "utils/ribbontabbar.h"
#include "utils/lightbox/tutorialviewer.h"
//#include "utils/lightbox/qlightboxwidget.h"
#include "utils/aboutwindow.h"

class DatabaseWindow;

/**
 * @brief The MasterWindow class serves as a Parent Widget for all
 * LIISim guis (SignalEditor, AnalysisTools, etc.).
 * @ingroup GUI
 */
class MasterWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MasterWindow(QWidget *parent = 0);
    ~MasterWindow();

    static QString identifier_masterwindow;
    static QString identifier_tutorial;

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QString m_windowTitle;
    /** @brief masterWindowCount holds the number of MasterWindows created */
    static int masterWindowCount;

    // Actions
    QAction* actionNewWindow;
    QAction* actionHelp;
    QAction* actionAbout;

    //QLightBoxWidget* lightBox;
    TutorialViewer* tutorialViewer;

    // GUI
    QVBoxLayout* layout;
    RibbonTabBar* topBar;
    QStackedWidget* widgetStack;

    StatusMessageWidget* statusbar;

    QProgressBar* status_progressBar;
    QLabel* status_modelingSettingsInfo;
    QPushButton* status_abort;

    AboutWindow *aboutWindow;

    DatabaseWindow* databaseWindow;

private slots:
    void onTabChanged(int i);

    void onActionNewWindow();
    void onActionHelp();
    void onActionAbout();

    void handleIOStateChanged(bool state);
    void onModelingSettingsChanged();
    void onAbortReleased();

    void onGUISettingsChanged();

    void onShowTutorials();

    void onCurrentLocaleChanged(QLocale locale);

    void onUpdateProgressBar(int value);
};

#endif // MASTERWINDOW_H
