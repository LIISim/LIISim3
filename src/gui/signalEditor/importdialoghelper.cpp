#include "importdialoghelper.h"

#include "../../io/consistencycheck.h"

// --- SignalImportFileInfoElement implementation ---

/**
 * @brief SignalImportFileInfoElement::SignalImportFileInfoElement
 * @param fileInfo
 * @param parent
 * Takes a SignalFileInfo object and constructs a SignalImportFileInfoElement
 */
SignalImportFileInfoElement::SignalImportFileInfoElement(SignalFileInfo fileInfo, QWidget *parent) : QWidget(parent)
{
    loaded = false;
    error = false;
    file = fileInfo;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);

    labelFileName = new QLabel(file.filename.split("/").back()); //remove the path from the filename
    labelFileName->setToolTip(file.filename);

    //if the file is not a '*_settings' file, add signal count and type to file name label
    if(fileInfo.itype == SignalIOType::CSV_SCAN || fileInfo.itype == SignalIOType::CSV)
    {
        if(!labelFileName->text().contains("_settings.txt"))
        {
            labelFileName->setText(labelFileName->text().append(" - %0 signal(s)").arg(fileInfo.fileLineCount));

            if(file.stdevFile)
                labelFileName->setText(labelFileName->text().append(", standard deviation"));

            if(file.signalType == Signal::RAW)
                labelFileName->setText(labelFileName->text().append(", raw"));
            else if(file.signalType == Signal::ABS)
                labelFileName->setText(labelFileName->text().append(", absolute"));
        }
    }
    else if(fileInfo.itype == SignalIOType::CUSTOM)
    {
        //TODO: Any infos for custom files?
    }

    labelImportStatus = new QLabel("");
    layout->addWidget(labelFileName);
    layout->addWidget(labelImportStatus);
    setLayout(layout);
}


/**
 * @brief SignalImportFileInfoElement::setLoaded
 * @param loaded
 * Sets the status label to "Loaded" when @param loaded = true,
 * otherwise clears the label.
 */
void SignalImportFileInfoElement::setLoaded(bool loaded)
{
    if(!error)
    {
        this->loaded = loaded;

        if(loaded)
        {
            labelImportStatus->setText("Loaded");
            labelImportStatus->setStyleSheet("QLabel { color : green }");
        }
        else
            labelImportStatus->setText("");
    }
}


/**
 * @brief SignalImportFileInfoElement::setError
 * @param error
 * Sets the status label text to @param error and blocks the
 * 'setLoaded' function, e.g. a file can not be set to "Loaded"
 * if an error occured.
 */
void SignalImportFileInfoElement::setError(QString error)
{
    this->error = true;
    this->loaded = false;
    labelImportStatus->setText(error.prepend("Error: "));
    labelImportStatus->setStyleSheet("QLabel { color : crimson }");
}


// --- SignalImportInfoElement implementation ---

SignalImportInfoElement::SignalImportInfoElement(QWidget *parent) : QWidget(parent)
{
    imported = false;
    error = false;

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *layoutGeneralInfo = new QHBoxLayout;

    checkboxRunInfo = new QCheckBox(this);
    checkboxRunInfo->setChecked(true);
    checkboxRunInfo->setMaximumWidth(250);
    checkboxRunInfo->setMinimumWidth(250);

    labelRunDetails = new QLabel(this);
    labelImportStatus = new QLabel("");

    buttonShowDetails = new QToolButton;
    buttonShowDetails->setToolTip("Show Details");
    buttonShowDetails->setStyleSheet("QToolButton { border-style: none }");
    buttonShowDetails->setIcon(QIcon(Core::rootDir + "resources/icons/bullet_arrow_right.png"));

    QVBoxLayout *layoutDetails = new QVBoxLayout;
    layoutDetails->setContentsMargins(QMargins(28, 0, 0, 10));
    widgetDetails = new QWidget;
    widgetDetails->setLayout(layoutDetails);

    labelWarning = new QLabel;
    labelWarning->setVisible(false);

    labelError = new QLabel;
    labelError->setVisible(false);

    QWidget *liisettingsWidget = new QWidget;
    QHBoxLayout *liisettingsLayout = new QHBoxLayout;
    liisettingsLayout->setMargin(0);
    QLabel *labelLiisettings = new QLabel("LIISettings:");
    comboboxLIISettings = new LIISettingsCB();
    comboboxLIISettings->setCurrentGlobal();

    liisettingsLayout->addWidget(labelLiisettings);
    liisettingsLayout->addWidget(comboboxLIISettings);
    liisettingsLayout->setAlignment(labelLiisettings, Qt::AlignLeft);
    liisettingsLayout->setAlignment(comboboxLIISettings, Qt::AlignLeft);
    liisettingsWidget->setLayout(liisettingsLayout);

    layoutFiles = new QVBoxLayout;
    QLabel *labelFiles = new QLabel("Files:");
    labelFiles->setStyleSheet("QLabel { font : bold }");
    layoutFiles->addWidget(labelFiles);

    layoutDetails->addWidget(labelWarning);
    layoutDetails->addWidget(labelError);
    layoutDetails->addWidget(liisettingsWidget);
    layoutDetails->addLayout(layoutFiles);

    layoutGeneralInfo->addWidget(buttonShowDetails);
    layoutGeneralInfo->addWidget(checkboxRunInfo);
    layoutGeneralInfo->addWidget(labelRunDetails);
    layoutGeneralInfo->addWidget(labelImportStatus);
    layout->addLayout(layoutGeneralInfo);
    layout->addWidget(widgetDetails);

    setLayout(layout);
    layout->setMargin(0);

    widgetDetails->setVisible(false);

    connect(buttonShowDetails, SIGNAL(clicked(bool)), SLOT(onButtonShowDetailsClicked()));
    connect(comboboxLIISettings, SIGNAL(currentIndexChanged(int)), SLOT(onLIISettingsSelectionChanged()));
}


void SignalImportInfoElement::addSettingsFile(QString fileName)
{
    SignalFileInfo fi;
    fi.filename = fileName;
    SignalImportFileInfoElement *fie = new SignalImportFileInfoElement(fi);
    fie->setLoaded(true);
    listFileElements.push_back(fie);
    layoutFiles->addWidget(fie);
}


void SignalImportInfoElement::addFiles(SignalFileInfoList files)
{
    for(int i = 0; i < files.size(); i++)
    {
        SignalImportFileInfoElement *fie = new SignalImportFileInfoElement(files.at(i));
        listFileElements.push_back(fie);
        layoutFiles->addWidget(fie);
    }
}


/**
 * @brief SignalImportInfoElement::showDetails
 * Expands the import request to show details, for errors etc.
 */
void SignalImportInfoElement::showDetails()
{
    widgetDetails->setVisible(true);
    buttonShowDetails->setIcon(QIcon(Core::rootDir + "resources/icons/bullet_arrow_down.png"));
}


/**
 * @brief SignalImportInfoElement::setImporting
 * Called when the import starts
 */
void SignalImportInfoElement::setImporting()
{
    labelImportStatus->setText("Importing...");
    labelImportStatus->setStyleSheet("QLabel { color : orange }");
    checkboxRunInfo->setStyleSheet("QCheckBox { color : black }");
    checkboxRunInfo->setIcon(QIcon());
    comboboxLIISettings->setEnabled(false);
    if(request.flist.isEmpty())
        setImportError();
}


/**
 * @brief SignalImportInfoElement::setImported
 * @param imported
 * Called when all files are loaded, thus the complete run is loaded
 */
void SignalImportInfoElement::setImported(bool imported)
{
    if(!error)
    {
        this->imported = imported;
        if(imported)
        {
            labelImportStatus->setText("Loaded");
            labelImportStatus->setStyleSheet("QLabel { color : green }");
        }
        else
            labelImportStatus->setText("");
    }
}


/**
 * @brief SignalImportInfoElement::setImportError
 * Called when any error occured while loading the files
 */
void SignalImportInfoElement::setImportError()
{
    error = true;
    labelImportStatus->setText("Error");
    labelImportStatus->setStyleSheet("QLabel { color : crimson }");
}


/**
 * @brief SignalImportInfoElement::addError
 * @param message
 * Adds a error message to the details
 */
void SignalImportInfoElement::addError(QString message)
{
    if(!errorList.contains(message))
    {
        errorList.push_back(message);
        updateErrorLabel();
        updateCheckbox();
    }
}


/**
 * @brief SignalImportInfoElement::addWarning
 * @param message
 * Adds a warning message to the details
 */
void SignalImportInfoElement::addWarning(QString message)
{
    if(!warningList.contains(message))
    {
        warningList.push_back(message);
        updateWarningLabel();
        updateCheckbox();
    }
}


bool SignalImportInfoElement::removeError(QString message)
{
    if(errorList.contains(message))
    {
        errorList.removeAll(message);
        updateErrorLabel();
        updateCheckbox();
        return true;
    }
    return false;
}


bool SignalImportInfoElement::removeWarning(QString message)
{
    if(warningList.contains(message))
    {
        warningList.removeAll(message);
        updateWarningLabel();
        updateCheckbox();
        return true;
    }
    return false;
}


void SignalImportInfoElement::updateErrorLabel()
{
    if(errorList.isEmpty())
        labelError->setVisible(false);
    else
    {
        QString message = "Error:<br>";
        message.append(errorList.at(0));

        for(int i = 1; i < errorList.size(); i++)
        {
            message.append("<br>");
            message.append(errorList.at(i));
        }
        labelError->setText(message);
        labelError->setStyleSheet("QLabel { font : bold ; color : crimson }");
        labelError->setVisible(true);
    }
}


void SignalImportInfoElement::updateWarningLabel()
{
    if(warningList.isEmpty())
        labelWarning->setVisible(false);
    else
    {
        QString message = "Warning:<br>";
        message.append(warningList.at(0));

        for(int i = 1; i < warningList.size(); i++)
        {
            message.append("<br>");
            message.append(warningList.at(i));
        }
        labelWarning->setText(message);
        labelWarning->setStyleSheet("QLabel { font : bold ; color : coral }");
        labelWarning->setVisible(true);
    }
}


/**
 * @brief SignalImportInfoElement::updateCheckbox
 * Updates the checkbox according to existing errors or warnings
 */
void SignalImportInfoElement::updateCheckbox()
{
    QProxyStyle style;

    if(!errorList.isEmpty())
    {
        checkboxRunInfo->setStyleSheet("QCheckBox { color : crimson}");
        checkboxRunInfo->setIcon(style.standardIcon(QStyle::SP_MessageBoxCritical));
        checkboxRunInfo->setChecked(false);
    }
    else if(!warningList.isEmpty())
    {
        checkboxRunInfo->setStyleSheet("QCheckBox { color : coral}");
        checkboxRunInfo->setIcon(style.standardIcon(QStyle::SP_MessageBoxWarning));
    }
    else
    {
        checkboxRunInfo->setStyleSheet("QCheckBox { color : black}");
        checkboxRunInfo->setIcon(QIcon());
        checkboxRunInfo->setChecked(true);
    }
}


void SignalImportInfoElement::onButtonShowDetailsClicked()
{
    if(widgetDetails->isVisible())
    {
        widgetDetails->setVisible(false);
        buttonShowDetails->setIcon(QIcon(Core::rootDir + "resources/icons/bullet_arrow_right.png"));
    }
    else
    {
        widgetDetails->setVisible(true);
        buttonShowDetails->setIcon(QIcon(Core::rootDir + "resources/icons/bullet_arrow_down.png"));
    }
}


void SignalImportInfoElement::onLIISettingsSelectionChanged()
{
    if(comboboxLIISettings->currentIsGlobal())
    {
        if(channelCount > globalLIISettings.channels.size())
            addError("Channel count too high for selected LIISettings");
        else
            removeError("Channel count too high for selected LIISettings");
    }
    else
    {
        if(channelCount > comboboxLIISettings->currentLIISettings().channels.size())
            addError("Channel count too high for selected LIISettings");
        else
            removeError("Channel count too high for selected LIISettings");
    }
}


/**
 * @brief SignalImportInfoElement::setRequest
 * @param rq SignalIORequest
 * @param settings Global LIISettings
 */
void SignalImportInfoElement::setRequest(SignalIORequest rq, LIISettings settings)
{
    request = rq;
    globalLIISettings = settings;

    if(rq.itype == SignalIOType::CSV)
    {
        //if a '*_settings' file exists
        if(rq.userData.contains(30) && rq.userData.value(30).toBool())
        {
            //remove any path occurence from run/filename and add as settings file
            addSettingsFile(QString(rq.runname.split("/").back()).append("_settings.txt"));

            //if a runname is defined in the '*_settings' file
            if(rq.userData.contains(31))
            {
                if(rq.runname.contains(rq.userData.value(31).toString()))
                    checkboxRunInfo->setText(rq.runname.split("/").last());
                else //if the filename differs from the runname, set the runname as main text and add the filename in brackets
                    checkboxRunInfo->setText(rq.userData.value(31).toString().append(" (").append(rq.runname).append(")"));
                checkboxRunInfo->setToolTip("../" + rq.runname);
            }
            else
            {
                checkboxRunInfo->setText(rq.runname.split("/").last());
            }

            //get LIISettings names from database and check if LIISettings in '*_settings' file exist
            QString liisettingsName;
            QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getLIISettings();
            for(int i = 0; i < dbc.size(); i++)
            {
                if(dbc.at(i)->filename == request.userData.value(3))
                    liisettingsName = dbc.at(i)->name;
            }
            if(!liisettingsName.isEmpty())
            {
                int index = comboboxLIISettings->findText(liisettingsName);
                if(index > -1)
                    comboboxLIISettings->setCurrentIndex(index);
                else
                {
                    comboboxLIISettings->setCurrentGlobal();
                }
            }
            else
                addError(QString("LIISettings defined in '%0_settings.txt' not found in database").arg(rq.runname.split("/").back()));

            comboboxLIISettings->setEnabled(false);
            comboboxLIISettings->setToolTip(QString("LIISettings already defined in '%0_settings.txt'").arg(rq.runname.split("/").back()));
        }
        else
        {
            addWarning("No settings file found");
            checkboxRunInfo->setText(rq.runname.split("/").last());
            checkboxRunInfo->setToolTip("../" + rq.runname);
            comboboxLIISettings->setCurrentGlobal();
        }

        //add channel, file count and signal type to details
        QString runDetailsText;

        channelCount = 0;
        for(int i = 0; i < rq.flist.size(); i++)
            if(channelCount < rq.flist.at(i).channelId)
                channelCount = rq.flist.at(i).channelId;

        QList<Signal::SType> signalTypes;
        for(int i = 0; i < rq.flist.size(); i++)
        {
            if(!signalTypes.contains(rq.flist.at(i).signalType))
                signalTypes.append(rq.flist.at(i).signalType);
        }

        runDetailsText.append(QString("%0 channel - %1 file(s)").arg(channelCount).arg(rq.flist.size()));

        if(signalTypes.contains(Signal::RAW) && signalTypes.contains(Signal::ABS))
            runDetailsText.append(" - Raw and Absolute");
        else if(signalTypes.contains(Signal::RAW) && !signalTypes.contains(Signal::ABS))
            runDetailsText.append(" - Raw only");

        if(!rq.flist.isEmpty())
        {
            int lineCount = rq.flist.at(0).fileLineCount;
            bool ok = true;
            for(int i = 1; i < rq.flist.size(); i++)
                if(rq.flist.at(i).fileLineCount != lineCount)
                    ok = false;
            if(ok)
                runDetailsText.append(QString(" - %0 signal(s)").arg(lineCount));
            else
                addError("Signal count differs between files");
        }

        labelRunDetails->setText(runDetailsText);
    }
    else if(rq.itype == SignalIOType::CUSTOM)
    {
        checkboxRunInfo->setText(rq.runname);

        channelCount = 0;
        if(request.channelPerFile)
        {
            for(int i = 0; i < rq.flist.size(); i++)
                if(channelCount < rq.flist.at(i).channelId)
                    channelCount = rq.flist.at(i).channelId;
        }
        else
        {
            channelCount = request.noChannels;
        }

        if(rq.userData.contains(30) && rq.userData.value(30).toBool())
        {
            if(rq.userData.contains(31))
                addSettingsFile(rq.userData.value(31).toString());

            if(rq.userData.contains(3))
            {
                QString liisettingsName;
                QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getLIISettings();
                for(int i = 0; i < dbc.size(); i++)
                {
                    if(dbc.at(i)->filename == request.userData.value(3))
                        liisettingsName = dbc.at(i)->name;
                }
                if(!liisettingsName.isEmpty())
                {
                    int index = comboboxLIISettings->findText(liisettingsName);
                    if(index > -1)
                    {
                        comboboxLIISettings->setCurrentIndex(index);
                        comboboxLIISettings->setEnabled(false);
                        comboboxLIISettings->setToolTip(QString("LIISettings already defined in '%0'").arg(rq.runsettings_filename));
                    }
                    else
                        comboboxLIISettings->setCurrentGlobal();
                }
                else
                    addError(QString("LIISettings defined in '%0' not found in database").arg(rq.runsettings_filename));
            }
        }
        else
        {
            addWarning(QString("No settings file found with the name '%0_settings.txt' for this run").arg(rq.runname));
        }

        checkboxRunInfo->setText(checkboxRunInfo->text().append(QString(" - %0 channel").arg(channelCount)));

        if(channelCount > settings.channels.size())
            addError("Channel count too high for selected LIISettings");

        ConsistencyCheck cc;
        bool ccError = cc.check(request);

        if(ccError)
            addWarning("Consistency Check returned errors, import might lead to unexpected results.<br>See notifications for further information.");
    }

    if(rq.flist.isEmpty())
        addError("No signal files available for import");
    else
        addFiles(rq.flist);

    onLIISettingsSelectionChanged();
}


/**
 * @brief SignalImportInfoElement::setGlobalLIISettings
 * @param settings
 * Called by import dialog if global LIISettings are changed
 */
void SignalImportInfoElement::setGlobalLIISettings(LIISettings settings)
{
    globalLIISettings = settings;
    if(comboboxLIISettings->isEnabled() && comboboxLIISettings->currentIsGlobal())
        onLIISettingsSelectionChanged();
}


// --- LIISettingsCB implementation ---

LIISettingsCB::LIISettingsCB(QWidget *parent) : QComboBox(parent), identifierGlobal("Default LIISettings")
{
    setToolTip("LIISettings for measurement run");
    connect(Core::instance()->getDatabaseManager(), SIGNAL(signal_contentChanged(int)), SLOT(onDatabaseContentChanged(int)));
    onDatabaseContentChanged();
}


LIISettingsCB::~LIISettingsCB()
{

}


bool LIISettingsCB::currentIsGlobal()
{
    return currentText() == identifierGlobal;
}


void LIISettingsCB::setCurrentGlobal()
{
    int index = findText(identifierGlobal);
    if(index > -1)
        setCurrentIndex(index);
}


LIISettings LIISettingsCB::currentLIISettings()
{
    LIISettings liis;
    QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getLIISettings();
    for(int i = 0; i < dbc.size(); i++)
    {
        if(currentText() == dbc.at(i)->name)
        {
            liis = *Core::instance()->getDatabaseManager()->liiSetting(dbc.at(i)->ident);
            return liis;
        }
    }
    return liis;
}


void LIISettingsCB::onDatabaseContentChanged(int id)
{
    blockSignals(true);
    int lastIndex = currentIndex();
    QString lastText = currentText();
    QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getLIISettings();

    clear();

    int newIndex = -1;

    addItem(identifierGlobal);
    if(lastText == identifierGlobal)
        newIndex = 0;

    for(int i = 0; i < dbc.size(); i++)
    {
        addItem(dbc.at(i)->name);
        if(lastText == dbc.at(i)->name)
            newIndex = i + 1;
    }

    if(newIndex > -1)
        setCurrentIndex(newIndex);
    else if(lastIndex > -1 && lastIndex < count())
        setCurrentIndex(lastIndex);

    blockSignals(false);
}
