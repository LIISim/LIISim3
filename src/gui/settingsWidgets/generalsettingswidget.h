#ifndef GENERALSETTINGSWIDGET_H
#define GENERALSETTINGSWIDGET_H

#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include "../utils/labeledcombobox.h"


/**
 * @brief The GeneralSettingsWidget class serves as a GUI-Editor for
 * the GeneralSettings.
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class GeneralSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit GeneralSettingsWidget(QWidget *parent = 0);

    QPushButton *buttonTutorial;

private:

    QVBoxLayout* layMainV;

    /// @brief Combobox for selection of core-count for import
    LabeledComboBox* cbImportCores;
    LabeledComboBox *cbLocales;
    QCheckBox *checkboxAutoSaveSettings;
    QSpinBox *spinboxAutoSaveTime;
    QPushButton *buttonResetSplitter;

signals:

public slots:

private slots:

    void onGeneralSettingsChanged();

    void onCoreCountImportEdited(int idx);
    void onLocalesCurrentIndexChanged(int index);

    void onCheckboxAutoSaveSettingsStateChanged(int state);
    void onSpinboxAutoSaveTimeValueChanged(int value);

    void onButtonClicked();

};

#endif // GENERALSETTINGSWIDGET_H
