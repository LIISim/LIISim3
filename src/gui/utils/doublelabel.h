#ifndef DOUBLELABEL_H
#define DOUBLELABEL_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class DoubleLabel : public QWidget
{
    Q_OBJECT

public:
    DoubleLabel(QWidget *parent = 0);
    DoubleLabel(QString firstText, QString secondText, QWidget *parent = 0);

    void setFirstText(QString text);
    void setSecondText(QString text);

private:
    QHBoxLayout *layoutMain;
    QLabel *first;
    QLabel *second;
};

#endif // DOUBLELABEL_H
