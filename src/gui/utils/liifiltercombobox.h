#ifndef LIIFILTERCOMBOBOX_H
#define LIIFILTERCOMBOBOX_H

#include <QComboBox>
#include "../../database/structure/liisettings.h"

class LIIFilterComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit LIIFilterComboBox(QWidget *parent = 0);
    ~LIIFilterComboBox();

    QString currentFilterIdentifier();

    void setAvailableFilters(LIISettings liiSettings);

private slots:
    void onIndexChanged(int index);
};

#endif // LIIFILTERCOMBOBOX_H
