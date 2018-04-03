#include "visibilitybutton.h"

#include "core.h"

VisibilityButton::VisibilityButton(QWidget *parent) : QPushButton(parent)
{
    setToolTip("Toggle Visibility");
    setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
    mChecked = true;

    setStyleSheet("QPushButton { border-style: none}");

    connect(this, SIGNAL(clicked(bool)), SLOT(onClicked()));
}


bool VisibilityButton::isChecked()
{
    return mChecked;
}


void VisibilityButton::setChecked(bool checked)
{
    mChecked = checked;
    if(checked)
        setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
    else
        setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));

    emit visibilityToggled(mChecked);
}


void VisibilityButton::onClicked()
{
    this->blockSignals(true);
    setChecked(!mChecked);
    this->blockSignals(false);
    emit visibilityToggled(mChecked);
}

