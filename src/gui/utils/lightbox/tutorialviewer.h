#ifndef TUTORIALVIEWER_H
#define TUTORIALVIEWER_H

#include <QWidget>
#include <QMap>

#include "qlightboxwidget.h"
#include "tutorialview.h"

class TutorialViewer : public QWidget
{
    Q_OBJECT
public:
    explicit TutorialViewer(QWidget *parent);

    /** @brief lightBox container to display TutorialViews   */
    QLightBoxWidget* lightBox;

    void addView(TutorialView *view);

    void showView();

    void showTutorial(QString moduleName);

    QRect getViewRect(QString objName, QString moduleName);
    QRect getTopBarViewRect(QString objName);

    void setFirstStart();

private:
    QList<TutorialView*> views;

    QMap<QString, QList<TutorialView*>*> _views;

    QList<TutorialView*> *currentViews;

    int currentIndex;
    QString currentModule;

    bool firstStart;

signals:

private slots:
    void showLast();
    void showNext();
    void closeView();

};

#endif // TUTORIALVIEWER_H
