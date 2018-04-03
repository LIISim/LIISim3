#include "channelvisualcontrolwidget.h"
#include "core.h"

ChannelVisualControlWidget::ChannelVisualControlWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout();

    buttonVisible = new QToolButton(this);
    //buttonVisible->setText("Visible");
    buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
    buttonVisible->setStyleSheet("QToolButton { border-style: none}");
    buttonVisible->setMaximumWidth(18);

    buttonLayerUp = new QToolButton(this);
    buttonLayerUp->setText("UP");
    buttonLayerUp->setIcon(QIcon(Core::rootDir + "resources/icons/visible_front.png"));
    buttonLayerUp->setStyleSheet("QToolButton { border-style: none}");
    buttonVisible->setMaximumWidth(18);

    buttonLayerDown = new QToolButton(this);
    buttonLayerDown->setText("DN");
    buttonLayerDown->setIcon(QIcon(Core::rootDir + "resources/icons/visible_back.png"));
    buttonLayerDown->setStyleSheet("QToolButton { border-style: none}");
    buttonVisible->setMaximumWidth(18);

    layout->addWidget(buttonVisible);
    layout->addWidget(buttonLayerUp);
    layout->addWidget(buttonLayerDown);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);
}

