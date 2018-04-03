#ifndef VISIBILITYBUTTON_H
#define VISIBILITYBUTTON_H

#include <QPushButton>

class VisibilityButton : public QPushButton
{
    Q_OBJECT
public:
    VisibilityButton(QWidget *parent = 0);

    bool isChecked();
    void setChecked(bool checked);

private:
    bool mChecked;

private slots:
    void onClicked();

signals:
    void visibilityToggled(bool mChecked);
};

#endif // VISIBILITYBUTTON_H
