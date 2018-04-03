#ifndef TUTORIALVIEW_H
#define TUTORIALVIEW_H

#include <QObject>
#include <QColor>

/**
 * @brief The TutorialView class stores tutorial view information
 */
class TutorialView : public QObject
{
    Q_OBJECT
public:
    explicit TutorialView(QObject *parent = 0);
    TutorialView(QString objName,
                 QString caption,
                 QString text,
                 Qt::Alignment flag,
                 QString moduleTabName,
                 bool textColorBlack = false,
                 bool topBarItem = false);

    /** @brief objectName objectName of widget that should be highlighted (setObjectName();)   */
    QString objName;

    /** @brief caption large text    */
    QString caption;
    QString text;
    Qt::Alignment align;

    /** @brief moduleTabName contains tab name used for this view (Database, Signal Processing,...) */
    QString moduleTabName;

    Qt::AlignmentFlag flag;

    /** @brief viewed set if user has seen this   */
    bool viewed = false;

    bool textColorBlack;

    bool topBarItem;

signals:

public slots:
};

#endif // TUTORIALVIEW_H
