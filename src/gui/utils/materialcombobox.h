#ifndef MATERIALCOMBOBOX_H
#define MATERIALCOMBOBOX_H

#include "labeledcombobox.h"

/**
 * @brief The MaterialComboBox class is a Combobox
 * which is used for the selection of the current modelingsetting's
 * material.
 */
class MaterialComboBox : public LabeledComboBox
{
    Q_OBJECT
public:
    explicit MaterialComboBox(QWidget *parent = 0);
    ~MaterialComboBox();

signals:

public slots:

private slots:

    void onCurrentIndexChanged(int idx);
    void onModelingSettingsChanged();
    void onDatabaseContentChanged(int id);

private:
    bool mute;
};

#endif // MATERIALCOMBOBOX_H
