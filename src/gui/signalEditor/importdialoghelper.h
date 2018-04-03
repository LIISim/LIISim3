#ifndef IMPORTDIALOGHELPER_H
#define IMPORTDIALOGHELPER_H

#include <QProxyStyle>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include "../../core.h"
#include "../../database/structure/liisettings.h"
#include "../../io/signaliorequest.h"

/**
 * @brief The LIISettingsCB class
 * Class based on ../utils/LIISettingsCombobox, but enables a "global" state, which is
 * only useful in import context, therefore the reimplementation.
 */
class LIISettingsCB : public QComboBox
{
    Q_OBJECT
public:
    explicit LIISettingsCB(QWidget *parent = 0);
    ~LIISettingsCB();

    bool currentIsGlobal();
    void setCurrentGlobal();

    LIISettings currentLIISettings();

private:
    const QString identifierGlobal;

private slots:
    void onDatabaseContentChanged(int id = -1);
};

/**
 * @brief The SignalImportFileInfoElement class
 * Representation of a single file in the import details
 * and its actual state / errors
 */
class SignalImportFileInfoElement : public QWidget
{
    Q_OBJECT
public:
    SignalImportFileInfoElement(SignalFileInfo fileInfo, QWidget *parent = 0);

    void setLoaded(bool loaded);
    void setError(QString error);

    SignalFileInfo file;
    QLabel *labelFileName;
    QLabel *labelImportStatus;
    bool loaded;
    bool error;
};


//TODO: destructor!!!
/**
 * @brief The SignalImportInfoElement class
 * Represents one import request in the UI with associated files, errors
 */
class SignalImportInfoElement : public QWidget
{
    Q_OBJECT
public:
    SignalImportInfoElement(QWidget *parent = 0);

    void showDetails();
    void setImporting();
    void setImported(bool imported);
    void setImportError();

    void setRequest(SignalIORequest rq, LIISettings settings);
    void setGlobalLIISettings(LIISettings settings);

    void addError(QString message);
    void addWarning(QString message);

    bool removeError(QString message);
    bool removeWarning(QString message);

    // ---

    SignalIORequest request;

    QToolButton *buttonShowDetails;
    QCheckBox *checkboxRunInfo;
    QLabel *labelRunDetails;
    QLabel *labelImportStatus;

    QWidget *widgetDetails;

    QLabel *labelWarning;
    QLabel *labelError;
    LIISettingsCB *comboboxLIISettings;

    QVBoxLayout *layoutFiles;
    QList<SignalImportFileInfoElement*> listFileElements;

    bool imported;
    bool error;
    int channelCount;
    QStringList errorList;
    QStringList warningList;

private:
    void addSettingsFile(QString fileName);
    void addFiles(SignalFileInfoList files);

    void updateErrorLabel();
    void updateWarningLabel();
    void updateCheckbox();

    LIISettings globalLIISettings;

private slots:
    void onButtonShowDetailsClicked();
    void onLIISettingsSelectionChanged();

};


#endif // IMPORTDIALOGHELPER_H
