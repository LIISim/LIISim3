#ifndef DA_RUNSETTINGSWIDGET_H
#define DA_RUNSETTINGSWIDGET_H

#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QToolButton>

#include "../utils/liifiltercombobox.h"
#include "signal/mrungroup.h"
#include "da_laserenergysettingswidget.h"
#include "da_userdefinedparameters.h"

class DA_RunSettingsWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit DA_RunSettingsWidget(QWidget *parent = 0);
    ~DA_RunSettingsWidget();

    double getLaserFluence();

    QMap<QString, QVariant> getParameterList();

    QTextEdit *leDescription;

    LIIFilterComboBox *cbFilter;

    QComboBox *cbMRunGroup;

    LaserEnergyControllerWidget *lecw;
    LaserEnergyMeasurementWidget *lemw;

private:
    UserDefinedParameterMasterWidget *udpmw2;
    QList<UserDefinedParameterWidget*> parameterWidgets;

    int lastGroup;
    bool lastGroupFound;
    bool udpsLoaded;

    void saveUDPs();
    void loadUDPs();

public slots:
    void onGroupsChanged();

private slots:
    void onAddUDPClicked();
    void onClearUDPClicked();
    void onRemoveUDPClicked(UserDefinedParameterWidget *widget);
    void onUDPDataChanged();

    void onDescriptionTextChanged();
    void onGroupIndexChanged(int index);

    void onGUISettingsChanged();

};

#endif // DA_RUNSETTINGSWIDGET_H
