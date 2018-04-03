#ifndef CHANNELVISUALCONTROLWIDGET_H
#define CHANNELVISUALCONTROLWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QHBoxLayout>

class ChannelVisualControlWidget : public QWidget
{
    Q_OBJECT
public:
    ChannelVisualControlWidget(QWidget *parent = 0);

    QToolButton *buttonVisible;
    QToolButton *buttonLayerUp;
    QToolButton *buttonLayerDown;
private:
    QHBoxLayout *layout;
};

#endif // CHANNELVISUALCONTROLWIDGET_H
