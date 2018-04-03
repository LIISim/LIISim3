#include "modelingsettings.h"

#include "../core.h"

#include "../calculations/numeric.h"

#include "../database/databasemanager.h"

ModelingSettings::ModelingSettings(QObject *parent) :  SettingsBase(parent)
{
    MSG_DETAIL_1("init ModelingSettings");

    groupName = "ModelingSettings";

    // variable names in .ini file
    key_gasMixture          = "gasMixture";
    key_heatTransferModel   = "heatTransferModel";
    key_liiSettings         = "liiSettings";
    key_material            = "material";
    key_material_spec       = "materialSpec";
    key_processPressure     = "processPressure";    

    default_heatTransferModel   = "kock";

    default_gasMixture      = "gasmixtures/Kock_N2_100.txt";
    default_material        = "materials/Soot_Kock.txt";
    default_material_spec   = "materials/blackbody_particle.txt";
    default_liiSettings     = "liisettings/4CH-3C-LII.txt";

    default_processPressure = 1000.0;

    m_processPressure = default_processPressure;        
}


void ModelingSettings::setupConnections(Core* c)
{
    connect(c->getDatabaseManager(),SIGNAL(signal_contentChanged(int)),SLOT(onDBContentChanged(int)));
}


ModelingSettings::~ModelingSettings()
{
}


/**
 * @brief ModelingSettings::init this slot is called right after all key/value
 * pairs have been read from file. This method should validate input values and
 * add default values.
 */
void ModelingSettings::init()
{
    if(!settings.contains(key_gasMixture))
        settings.insert(key_gasMixture, default_gasMixture);

    if(!settings.contains(key_material))
        settings.insert(key_material, default_material);

    if(!settings.contains(key_material_spec))
        settings.insert(key_material_spec, default_material_spec);

    if(!settings.contains(key_liiSettings))
        settings.insert(key_liiSettings, default_liiSettings);

    if(!settings.contains(key_heatTransferModel))
        settings.insert(key_heatTransferModel, default_heatTransferModel);

    if(!settings.contains(key_processPressure))
        settings.insert(key_processPressure, default_processPressure);

    m_processPressure = settings.value(key_processPressure).toDouble();


    DatabaseManager* dbManager = Core::instance()->getDatabaseManager();

    // check if the requested default settings are stored in database and assign them if possible

    // Material
    QString value = settings.value(key_material).toString();
    int idx = dbManager->indexOfMaterial(value);
    if(idx == -1)
    {
        //if the last material used is not found, set the first material in the database as default
        QString msg;
        Material *material = dbManager->getMaterial(0);

        if(material != nullptr)
        {
            msg = "ModelingSettings: Material not found in database: " + value + " -> setting material to: " + material->name;
            setMaterial(material->filename);
        }
        else
            msg = "ModelingSettings: Material not found in database: " + value + "; no default could be set";

        // show only warning if database file does not exist
        if(value == "none")
            MSG_NORMAL(msg);
        else
            MSG_WARN(msg);
    }
    else
        m_material = *dbManager->getMaterial(idx);
    emit materialChanged();

    // Material (Spectroscopic)
    value = settings.value(key_material_spec).toString();
    idx = dbManager->indexOfMaterial(value);
    if(idx == -1)
    {
        //if the last spectroscopic material is not found, set the first element in the database as default
        QString msg;
        Material *material = dbManager->getMaterial(0);

        if(material != nullptr)
        {
            msg = "ModelingSettings: Material (Spectroscopic) not found in database: " + value + " -> setting to " + material->name;
            setMaterialSpec(material->filename);
        }
        else
            msg = "ModelingSettings: Material (Spectroscopic) not found in database: " + value + "; no default could be set";

        // show only warning if database file does not exist
        if(value == "none")
            MSG_NORMAL(msg);
        else
            MSG_WARN(msg);
    }
    else
        m_material_spec = *dbManager->getMaterial(idx);
    emit materialSpecChanged();

    // GasMixture
    value = settings.value(key_gasMixture).toString();
    idx = dbManager->indexOfGasMixture(value);
    if(idx == -1)
    {
        //if the last gas mixture used is could not be found, set the first gas mixture in the database as default
        QString msg;
        GasMixture *mixture = dbManager->getGasMixture(0);

        if(mixture != nullptr)
        {
            msg = "GasMixture not found in database: " + value + " -> setting to " + mixture->name;
            setGasMixture(mixture->filename);
        }
        else
            msg = "GasMixture not found in database: " + value + "; no default could be set";

        // show only warning if database file does not exist
        if(value == "none")
            MSG_NORMAL(msg);
        else
            MSG_WARN(msg);
    }
    else
       m_gasMixture = *dbManager->getGasMixture(idx);

    // LIISettings
    value = settings.value(key_liiSettings).toString();
    idx = dbManager->indexOfLIISettings(value);
    if(idx == -1)
    {
        //if the last liisettings used is not found, set the first entry in the database as default
        QString msg;
        LIISettings *liisettings = dbManager->getLIISetting(0);

        if(liisettings != nullptr)
        {
            msg = "ModelingSettings: LIISettings not found in database: " + value + " -> setting to " + liisettings->name;
            setLIISettings(liisettings->filename);
        }
        else
            msg = "ModelingSettings: LIISettings not found in database: " + value + "; no default could be set";

        // show only warning if database file does not exist
        if(value == "none")
            MSG_NORMAL(msg);
        else
            MSG_WARN(msg);
    }
    else
       m_defaultLIISettings = *dbManager->getLIISetting(idx);

    // HeatTransferModel (check if the requested heat transfer model is implemented)
    value = settings.value(key_heatTransferModel).toString();    
    idx = -1;
    for(int i=0;i < Core::instance()->heatTransferModels.size();i++)
        if(Core::instance()->heatTransferModels.at(i)->identifier == value)
            idx = i;

    if(idx == -1)
    {
        QString msg;

        if(Core::instance()->heatTransferModels.size() > 0)
        {
            msg = "ModelingSettings: Heat transfer model not found: " + value + " -> setting to " + Core::instance()->heatTransferModels.first()->name;
            setHeatTransferModel(0);
            m_heatTransferModel->setGasMix(m_gasMixture);
            m_heatTransferModel->setMaterial(m_material);
            m_heatTransferModel->setMaterialSpec(m_material_spec);
        }
        else
            msg = "ModelingSettings: Heat transfer model not found: " + value + "; no default could be set";

        // show only warning if database file does not exist
        if(value == "none")
            MSG_NORMAL(msg);
        else
            MSG_WARN(msg);
    }
    else
    {        
        m_heatTransferModel = Core::instance()->heatTransferModels.at(idx);
        m_heatTransferModel->setGasMix(m_gasMixture);
        m_heatTransferModel->setMaterial(m_material);
        m_heatTransferModel->setMaterialSpec(m_material_spec);
    }
}


bool ModelingSettings::setMaterial(const QString &fname)
{
    DatabaseManager* dbManager = Core::instance()->getDatabaseManager();
    bool res = false;

    int idx = dbManager->indexOfMaterial(fname);

    if(idx == -1)
    {
        MSG_WARN("ModelingSettings: Material name not found in database: " + fname);
        settings.insert(key_material,"none");
    }
    else
    {
        m_material = *dbManager->getMaterial(idx);

        m_heatTransferModel->setMaterial(m_material);

        settings.insert(key_material, fname);
        res = true;
    }


    emit materialChanged();
    emit settingsChanged();
    return res;
}


bool ModelingSettings::setMaterialSpec(const QString &fname)
{
    DatabaseManager* dbManager = Core::instance()->getDatabaseManager();
    bool res = false;

    int idx = dbManager->indexOfMaterial(fname);

    if(idx == -1)
    {
        MSG_WARN("ModelingSettings: Material (Spectroscopic) name not found in database: " + fname);
        settings.insert(key_material_spec,"none");
    }
    else
    {
        m_material_spec = *dbManager->getMaterial(idx);

        m_heatTransferModel->setMaterialSpec(m_material_spec);

        settings.insert(key_material_spec, fname);
        res = true;
    }

    emit materialSpecChanged();
    emit settingsChanged();
    return res;
}


bool ModelingSettings::setGasMixture(const QString &fname)
{
    DatabaseManager* dbManager = Core::instance()->getDatabaseManager();
    bool res = false;
    int idx = dbManager->indexOfGasMixture(fname);

    if(idx == -1)
    {
        MSG_WARN("ModelingSettings: GasMixture name not found in database: " + fname);
        settings.insert(key_gasMixture,"none");
    }
    else
    {
        m_gasMixture = *dbManager->getGasMixture(idx);
        settings.insert(key_gasMixture,fname);

        m_heatTransferModel->setGasMix(m_gasMixture);

        res = true;
    }

    emit settingsChanged();
    return res;
}


bool ModelingSettings::setLIISettings(const QString &fname)
{
    DatabaseManager* dbManager = Core::instance()->getDatabaseManager();
    bool res = false;
    int idx = dbManager->indexOfLIISettings(fname);
    if(idx == -1)
    {
        MSG_WARN("ModelingSettings: LIISettings name not found in database: " + fname);
        settings.insert(key_liiSettings,"none");
    }
    else
    {
        m_defaultLIISettings = *dbManager->getLIISetting(idx);
        settings.insert(key_liiSettings,fname);
        res = true;
    }
    emit settingsChanged();
    return res;
}


bool ModelingSettings::setHeatTransferModel(int index)
{
    bool res = false;

    if(index < 0 || index >= Core::instance()->heatTransferModels.size() )
    {
        MSG_WARN("ModelingSettings: invalid heat transfer model selection");
        settings.insert(key_heatTransferModel, "none");
    }
    else
    {
        m_heatTransferModel = Core::instance()->heatTransferModels.at(index);
        settings.insert(key_heatTransferModel, m_heatTransferModel->identifier);

        m_heatTransferModel->setGasMix(m_gasMixture);
        m_heatTransferModel->setMaterial(m_material);

        res = true;
    }
    emit settingsChanged();
    return res;
}


/**
 * @brief ModelingSettings::setProcessPressure
 * @param pressure new process pressure in Pa
 * @return
 */
bool ModelingSettings::setProcessPressure(double pressure)
{
    bool res = true;

    // unit: Pascal
    m_processPressure = pressure;
    settings.insert(key_processPressure ,m_processPressure);

    emit settingsChanged();
    return res;
}


/**
 * @brief ModelingSettings::onDBContentChanged this slot is executed
 * if the Properties of any DatabaseContent (Material, Gas, Gasmixture, LIISettings)
 * has been edited.
 * @param db_ident dbc id
 */
void ModelingSettings::onDBContentChanged(int db_ident)
{
    DatabaseManager* dbm = Core::instance()->getDatabaseManager();

    //handle db rescan, dbc deletion ..
    if(db_ident == -1)
    {
        // LIISettings
        int index = dbm->indexOfLIISettings(m_defaultLIISettings.filename);
        if(index < 0) // liisettings are not in db anymore ...
        {
           if(dbm->getLIISettings()->size() > 0) // pick any liisettings if possible
               m_defaultLIISettings = *dbm->getLIISetting(0);
           else
               MSG_WARN("ModelingsSettings: no liisettings found!");
        }
        else
            m_defaultLIISettings = *dbm->getLIISetting(index);

        // GasMixture
        index = dbm->indexOfGasMixture(m_gasMixture.filename);
        if(index < 0) // gasmixture is not in db anymore
        {
            if(dbm->getGasMixtures()->size() > 0) // pick any gasmixture if possible
                m_gasMixture = *dbm->getGasMixture(0);
            else
                MSG_WARN("ModelingsSettings: no gasmixtures found!");
        }
        else
            m_gasMixture = *dbm->getGasMixture(index);

        // Material
        index = dbm->indexOfMaterial(m_material.filename);
        if(index < 0) // material is not in db anymore
        {
            if(dbm->getMaterials()->size() > 0) // pick any material if possible
                m_material = *dbm->getMaterial(0);
            else
                MSG_WARN("ModelingsSettings: no material found!");
        }
        else
            m_material = *dbm->getMaterial(index);


        // Material (Spectroscopic)
        index = dbm->indexOfMaterial(m_material_spec.filename);
        if(index < 0) // material_spec is not in db anymore
        {
            if(dbm->getMaterials()->size() > 0) // pick any material if possible
                m_material_spec = *dbm->getMaterial(0);
            else
                MSG_WARN("ModelingsSettings: no material found!");
        }
        else
            m_material_spec = *dbm->getMaterial(index);

        emit materialChanged();
        emit materialSpecChanged();
        emit settingsChanged();
        return;
    }


    if(db_ident == m_material.ident)
    {
        m_material = *dbm->material(db_ident);
        m_heatTransferModel->setMaterial(m_material);
        emit materialChanged();
    }

    if(db_ident == m_material_spec.ident)
    {
        m_material_spec = *dbm->material(db_ident);
        m_heatTransferModel->setMaterialSpec(m_material_spec);
        emit materialSpecChanged();
    }

    if(db_ident == m_gasMixture.ident)
    {
        m_gasMixture = *dbm->gasMixture(db_ident);
        m_heatTransferModel->setGasMix(m_gasMixture);
    }

    if(db_ident == m_defaultLIISettings.ident)
        m_defaultLIISettings = *dbm->liiSetting(db_ident);

    emit settingsChanged();
}


/**
 * @brief ModelingSettings::copyFrom
 * @param other
 */
void ModelingSettings::copyFrom(ModelingSettings *other)
{
    m_gasMixture    = other->gasMixture();
    m_material      = other->material();
    m_material_spec = other->materialSpec();

    m_processPressure   = other->processPressure();

    m_heatTransferModel  = other->heatTransferModel()->clone();
    m_defaultLIISettings = other->defaultLiiSettings();

    settings.insert(key_heatTransferModel, m_heatTransferModel->identifier);
    settings.insert(key_material, m_material.filename);
    settings.insert(key_material_spec, m_material_spec.filename);
    settings.insert(key_gasMixture, m_gasMixture.filename);
    settings.insert(key_liiSettings, m_defaultLIISettings.filename);

    settings.insert(key_processPressure,m_processPressure);

    m_heatTransferModel->setGasMix(m_gasMixture);
    m_heatTransferModel->setMaterial(m_material);
    m_heatTransferModel->setMaterialSpec(m_material_spec);

    emit materialChanged();
    emit materialSpecChanged();
    emit settingsChanged();
}

