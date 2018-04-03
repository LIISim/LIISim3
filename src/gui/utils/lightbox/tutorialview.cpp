#include "tutorialview.h"

TutorialView::TutorialView(QObject *parent) : QObject(parent)
{

}


TutorialView::TutorialView(QString objName,
                           QString caption,
                           QString text,
                           Qt::Alignment flag,
                           QString moduleTabName,
                           bool textColorBlack,
                           bool topBarItem)
    : objName(objName),
    caption(caption),
    text(text),
    align(flag),
    moduleTabName(moduleTabName),
    textColorBlack(textColorBlack),
    topBarItem(topBarItem)
{
    viewed = false;
}
