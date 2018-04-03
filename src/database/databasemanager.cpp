#include "databasemanager.h"


#include "../general/LIISimException.h"

#include "../database/databasecontent.h"

#include <QStringList>
#include <QFileInfoList>
#include <QDir>
#include <QDirIterator>
#include <QStack>
#include <QThread>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <QDebug>
#include "../core.h"

int DatabaseManager::ident_count = 0;

DatabaseManager::DatabaseManager()
{
    MSG_DETAIL_1("init DatabaseManager");
    isLoading = false;

    // load equation list
    eqList = new EquationList();
}


DatabaseManager::~DatabaseManager()
{
    cleanUpDB();    
}


// GETTERS
QString DatabaseManager::getDataBasePath()
{
    return Core::instance()->generalSettings->databaseDirectory();
}


QList<DatabaseContent *> *DatabaseManager::getGases()
{
    return &db_gases;
}

QList<DatabaseContent *> *DatabaseManager::getMaterials()
{
    return &db_materials;
}

QList<DatabaseContent *> *DatabaseManager::getLIISettings()
{
    return &db_lIISettings;
}

QList<DatabaseContent *> *DatabaseManager::getGasMixtures()
{
    return &db_gasMixtures;
}

QList<DatabaseContent *> *DatabaseManager::getLaserEnergy()
{
    return &db_laserEnergy;
}

QList<DatabaseContent*> *DatabaseManager::getSpectra()
{
    return &db_spectrum;
}

QList<DatabaseContent*> *DatabaseManager::getTransmissions()
{
    return &db_transmission;
}


GasProperties* DatabaseManager::getGas(int index)
{
    if(db_gases.isEmpty())return NULL;
    return dynamic_cast<GasProperties*>(db_gases.at(index));
}

Material* DatabaseManager::getMaterial(int index)
{
    if(db_materials.isEmpty())return NULL;
    return dynamic_cast<Material*>(db_materials.at(index));
}

LIISettings* DatabaseManager::getLIISetting(int index)
{
    if(db_lIISettings.isEmpty())return NULL;
    return dynamic_cast<LIISettings*>(db_lIISettings.at(index));
}

GasMixture* DatabaseManager::getGasMixture(int index)
{
    if(db_gasMixtures.isEmpty())return NULL;
    return dynamic_cast<GasMixture*>(db_gasMixtures.at(index));
}

LaserEnergy* DatabaseManager::getLaserEnergy(int index)
{
    if(db_laserEnergy.isEmpty()) return NULL;
    return dynamic_cast<LaserEnergy*>(db_laserEnergy.at(index));
}

SpectrumDBE* DatabaseManager::getSpectrum(int index)
{
    if(db_spectrum.isEmpty()) return NULL;
    return dynamic_cast<SpectrumDBE*>(db_spectrum.at(index));
}

TransmissionDBE* DatabaseManager::getTransmission(int index)
{
    if(db_transmission.isEmpty()) return NULL;
    return dynamic_cast<TransmissionDBE*>(db_transmission.at(index));
}


GasProperties* DatabaseManager::gas(int dbc_id)
{
    for(int i = 0; i<db_gases.size(); i++)
        if(db_gases.at(i)->ident == dbc_id)
            return dynamic_cast<GasProperties*>(db_gases.at(i));
    return 0;
}

GasMixture* DatabaseManager::gasMixture(int dbc_id)
{
    for(int i = 0; i<db_gasMixtures.size(); i++)
        if(db_gasMixtures.at(i)->ident == dbc_id)
            return dynamic_cast<GasMixture*>(db_gasMixtures.at(i));
    return 0;
}

Material* DatabaseManager::material(int dbc_id)
{
    for(int i = 0; i<db_materials.size(); i++)
        if(db_materials.at(i)->ident == dbc_id)
            return dynamic_cast<Material*>(db_materials.at(i));
    return 0;
}

LIISettings* DatabaseManager::liiSetting(int dbc_id)
{
    for(int i = 0; i<db_lIISettings.size(); i++)
    {
        if(db_lIISettings.at(i)->ident == dbc_id)
            return dynamic_cast<LIISettings*>(db_lIISettings.at(i));
    }
    return 0;
}


LaserEnergy* DatabaseManager::laserEnergy(int dbc_id)
{
    for(int i = 0; i < db_laserEnergy.size(); i++)
    {
        if(db_laserEnergy.at(i)->ident == dbc_id)
            return dynamic_cast<LaserEnergy*>(db_laserEnergy.at(i));
    }
    return NULL;
}


SpectrumDBE* DatabaseManager::spectrum(int dbc_id)
{
    for(int i = 0; i < db_spectrum.size(); i++)
    {
        if(db_spectrum.at(i)->ident == dbc_id)
            return dynamic_cast<SpectrumDBE*>(db_spectrum.at(i));
    }
    return NULL;
}


TransmissionDBE* DatabaseManager::transmission(int dbc_id)
{
    for(int i = 0; i < db_transmission.size(); i++)
    {
        if(db_transmission.at(i)->ident == dbc_id)
            return dynamic_cast<TransmissionDBE*>(db_transmission.at(i));
    }
    return NULL;
}



/**
 * @brief scan the database directory recursivley
 * @param databasepath
 */
void DatabaseManager::slot_scanDatabase()
{
    // Reset all messages related to database content
    MSG_ONCE_RESET_GROUP("SpectroscopicMaterial");

    QString databasepath = Core::instance()->generalSettings->databaseDirectory();

     if(!isLoading)
        isLoading = true;
     else
     {
         qDebug() << "DatabaseManager already loading!";
         return;
     }

    try
    {
        MSG_NORMAL("Scanning database ("+databasepath+")...");
        cleanUpDB();

        QStringList nameFilter("*.txt");
        QDir db_dir(databasepath);

        if(!db_dir.exists())
        {
            throw LIISimException("Database directory does not exist ("+databasepath+")",ERR_IO);
        }

        // set filters on directory
        db_dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        db_dir.setNameFilters(nameFilter);

        // iterate through all subdirectories and push all ".txt" files on stack
        QDirIterator it(db_dir, QDirIterator::Subdirectories);
        QStack<QString> filesStack ;

        while(it.hasNext()) {
           filesStack.push(it.next());
        }

        // create property files and fill database vectors
        QString curRelativeFileName;

        for(int i = 0; i < filesStack.size(); i++)
        {
            curRelativeFileName = filesStack.at(i);

            // strip down to relative filename
            curRelativeFileName.remove(databasepath);
            loadFileToDatabase(curRelativeFileName);
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }

    // link gasproperty pointers to gasmixtures + check if requested gases exist
    QMultiMap<GasMixture*, Property>::iterator it = gasmixRequests.begin();
    int noGases = db_gases.size();
    GasProperties* currentGas;

    while (it != gasmixRequests.end())
    {
        QString gasfname = it.value().identifier;
        double x = it.value().parameter[0];
        GasMixture* gasMix = it.key();

        bool gasMatch = false;
        for(int i = 0; i< noGases; i++)
        {
            currentGas = getGas(i);

            if(currentGas->filename == gasfname)
            {
                gasMatch = true;
                gasMix->addGas(currentGas,x);
            }
        }

        if(!gasMatch)
        {
            MESSAGE("DatabaseManager:: gas mixture " + gasMix->name + ": gas " + gasfname + " does not exist in database!",WARNING);
        }
        ++it;
    }
    gasmixRequests.clear();

    // recalculate molar mass of gas mixtures
    for(int i = 0; i < db_gasMixtures.size(); i++)
    {
        GasMixture* gm = getGasMixture(i);
        gm->calculateMolarMass();
    }

    QString msg;
    msg.sprintf( "Scan finished: \n\t%d gases \n\t%d materials \n\t%d liiSettings \n\t%d gas mixtures \n\t%d laser energy settings \n\t%d spectra \n\t%d transmissions",
                 db_gases.size(),
                 db_materials.size(),
                 db_lIISettings.size(),
                 db_gasMixtures.size(),
                 db_laserEnergy.size(),
                 db_spectrum.size(),
                 db_transmission.size());

    MESSAGE(msg,NORMAL);
    MSG_STATUS(msg);

    emit signal_scanDatabaseFinished();
    emit signal_contentChanged();
     isLoading = false;
}


/**
 * @brief load file from given filename and add content to database
 * @param filename  - name of the file in directory: DATABASE_PATH
 * @return
 */
void DatabaseManager::loadFileToDatabase(const QString & filename)
{
    //!!! TODO: check if last sign in line is semicolon or modify read in algorithm
    //!!! TODO: skip empty lines
    //!!! TODO: add comment char # and skip these lines
    //!!! TODO: remove whitespace between parameters and seperator ;
    //!!! TODO: (optional) additional var for unit

    // define output var
    varList varlist;
    channelList channellist;
    filterList filterlist;

    QString file_location = Core::instance()->generalSettings->databaseDirectory() + filename;

    QFile file(file_location.toLatin1().data());

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error: File could not be opened (" << filename.toLatin1().data() << ")";

        throw LIISimException("File could not be opened: "+file_location, ERR_IO);
    }
    else
    {        
        // define input processing vars
        QString line;
        QStringList token;
        size_t num_channels = 0;

        // read first line
        line = file.readLine();
        token = line.split(";");

        if(token.size() < 4)
        {
            MESSAGE("Unknown file format [structure] ("+filename+")", WARNING);
            return;
        }

        QString name        = token[0];
        QString type        = token[1];
        QString version     = token[2];
        QString description = token[3];

        // FOUND NONSENSE IN DATABASE
        if(type != "Gas" &&
                type != "Material" &&
                type != "LIISettings" &&
                type != "GasMixture" &&
                type != "LaserEnergy" &&
                type != "Spectrum" &&
                type != "Transmission")
        {
            MESSAGE("Unknown file format [type] ("+filename+")", WARNING);
            return;
        }

        // read lines
        while(!file.atEnd())
        {            
            token.clear(); //clear this var for each line

            line = file.readLine();

            // skip line if line has less than 7 chars
            //if(line.length() < 7) continue;

            // skip line if first char is "#"
            QString comment_char = line.left(1);
            if(comment_char == "#") continue;

           // process line
            token = line.split(";");

            for (int ii = token.size(); ii < 12; ii++)
            {
                token.append("0");
            }

            // create Property object from line content
            Property property;

            if(type == "Gas" || type == "Material")
            {
                    property.name          = token.at(0);
                    property.type          = token.at(1);
                    property.parameter[0]  = token.at(2).toDouble();
                    property.parameter[1]  = token.at(3).toDouble();
                    property.parameter[2]  = token.at(4).toDouble();
                    property.parameter[3]  = token.at(5).toDouble();
                    property.parameter[4]  = token.at(6).toDouble();
                    property.parameter[5]  = token.at(7).toDouble();
                    property.parameter[6]  = token.at(8).toDouble();
                    property.parameter[7]  = token.at(9).toDouble();
                    property.parameter[8]  = token.at(10).toDouble();
                    property.source        = token.at(11);

                    if(property.source.isEmpty()
                            || property.source == "0"
                            || property.source == "\n")
                        property.source = "no source";
            }
            else if(type == "GasMixture")
            {
                if(token.at(0) == "gas_component")
                {
                    property.name          = "gas_component";                    
                    //property.description   = token.at(1);               // full name of gas component
                    property.identifier    = token.at(1);               // filename of gas component
                    property.parameter[0]  = token.at(2).toDouble();    // mole fraction: x_i
                }
                else // further properties of this gas mixture
                {
                    property.name          = token.at(0);
                    property.type          = token.at(1);
                    property.parameter[0]  = token.at(2).toDouble();
                    property.parameter[1]  = token.at(3).toDouble();
                    property.parameter[2]  = token.at(4).toDouble();
                    property.parameter[3]  = token.at(5).toDouble();
                    property.parameter[4]  = token.at(6).toDouble();
                    property.parameter[5]  = token.at(7).toDouble();
                    property.parameter[6]  = token.at(8).toDouble();
                    property.parameter[7]  = token.at(9).toDouble();
                    property.parameter[8]  = token.at(10).toDouble();
                    property.source        = token.at(11);
                    if(property.source.isEmpty()
                            || property.source == "0"
                            || property.source == "\n")
                        property.source = "no source";
                }
            }
            else if(type == "LIISettings")
            {
                    property.name          = token.at(0);

                    // save Channel objects in vector "channels"
                    if(token.at(0) == "channel")
                    {
                        property.type          = "channel";

                        Channel channel;

                        channel.wavelength             = token.at(1).toDouble();
                        channel.bandwidth              = token.at(2).toDouble();
                        channel.calibration            = token.at(3).toDouble();                        
                        channel.pmt_gain               = token.at(4).toDouble();
                        channel.pmt_gain_formula_A     = token.at(5).toDouble();
                        channel.pmt_gain_formula_B     = token.at(6).toDouble();
                        channel.offset                 = token.at(7).toDouble();

                        num_channels++;

                        channellist.push_back(channel);
                        continue; // skip varlist.insert()
                    }
                    else if(token[0] == "filter")
                    {
                        property.type          = "filter";

                        Filter filter;

                        filter.identifier = token.at(1);

                        size_t id = 0;
                        for (size_t ii = 2; ii < 12; ii++)
                        {
                            // loop only for available channels
                            if(id >= num_channels)
                                break;

                            // TODO: check if filter transmission is greater than 1
                            if(token.at(ii) == "0" || token.at(ii) == "")
                            {
                                MSG_ERR(tr("Filter: ")+token.at(1)+tr("% - Channel not found: ")+QString::number(channellist.at(id).wavelength));
                                // set transmission for this channel to 1
                                token[ii] = "100";
                            }

                            filter.list.insert(std::pair<int,double>(channellist.at(id).wavelength, token.at(ii).toDouble()));
                            id++;
                        }
                        // ignore default 100% filter definition in LIISettings file!
                        if(filter.identifier != "100"
                                && filter.identifier != "no Filter"
                                && filter.identifier != "100.0"
                                && filter.identifier != LIISettings::defaultFilterName)
                            filterlist.push_back(filter);
                        continue; // skip varlist.insert()
                    }
                    else
                    {
                        property.type          = "const";
                        property.parameter[0]  = token.at(1).toDouble();
                        property.source        = token.at(2);
                    }
            }
            else if(type == "LaserEnergy")
            {
                property.parameter[0] = token.at(0).toDouble();
                property.parameter[1] = token.at(1).toDouble();
                property.parameter[2] = token.at(2).toDouble();
            }
            else if(type == "Spectrum")
            {
                if(token.at(0) == "group")
                {
                    property.type = token.at(0);
                    property.name = token.at(1);
                }
                else
                {
                    property.parameter[0] = token.at(0).toDouble();
                    property.parameter[1] = token.at(1).toDouble();
                }
            }
            else if(type == "Transmission")
            {
                if(token.at(0) == "group")
                {
                    property.type = token.at(0);
                    property.name = token.at(1);
                }
                else
                {
                    property.parameter[0] = token.at(0).toDouble();
                    property.parameter[1] = token.at(1).toDouble();
                }
            }

            // add custom Property object to list
            varlist.insert(std::pair<QString, Property>(property.name, property));
        }

        file.close();

        if(type == "Gas")      // FOUND GAS
        {
            GasProperties* dbc = new GasProperties;
            dbc->ident = ident_count++;
            dbc->name = name;
            dbc->type = type;
            dbc->version = version;
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);
            db_gases.push_back(dbc);

        }
        else if(type  == "Material") // FOUND MATERIAL
        {
            Material* dbc =  new Material;
            dbc->ident = ident_count++;
            dbc->name = name;
            dbc->type = type;
            dbc->version = version;
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);
            db_materials.push_back(dbc);
        }
        else if(type == "GasMixture") // FOUND GASMIXTURE
        {
            GasMixture* dbc =  new GasMixture;
            dbc->ident = ident_count++;
            dbc->name = name;
            dbc->type = type;
            dbc->version = version;
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);
            db_gasMixtures.push_back(dbc);

            for( varList::const_iterator it = varlist.begin(); it != varlist.end(); it++)
            {
                // add only gas_component properties to the requestedGases list
               if(it->first == "gas_component")
               {
                    gasmixRequests.insert(dbc,it->second);
               }
            }
        }
        else if(type == "LIISettings") // FOUND LIISETTINGS
        {
            LIISettings* dbc =  new LIISettings;
            dbc->ident = ident_count++;
            dbc->name = name;
            dbc->type = type;
            dbc->version = version;
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);   //save settings
            dbc->channels = channellist; //save channels

            dbc->filters = filterlist << dbc->filters; //insert filters read to filterlist
            db_lIISettings.push_back(dbc);

            /*
            qDebug() << "DBM: LIISettings " << dbc->name;
            for(int ii = 0; ii < dbc->filters.size(); ii++)
                qDebug() << "    " << dbc->filters[ii].identifier << "channel map size: " << dbc->filters[ii].list.size();
            qDebug() << "\n ";*/
        }
        else if(type == "LaserEnergy")
        {
            LaserEnergy *dbc = new LaserEnergy;
            dbc->ident = ident_count++;
            dbc->name = name;
            dbc->type = type;
            dbc->version = version;
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);

            db_laserEnergy.push_back(dbc);
        }
        else if(type == "Spectrum")
        {
            SpectrumDBE *dbc = new SpectrumDBE;
            dbc->ident = ident_count++;
            dbc->name = name.trimmed();
            dbc->type = type.trimmed();
            dbc->version = version.trimmed();
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);

            db_spectrum.push_back(dbc);
        }
        else if(type == "Transmission")
        {
            TransmissionDBE *dbc = new TransmissionDBE;
            dbc->ident = ident_count++;
            dbc->name = name.trimmed();
            dbc->type = type.trimmed();
            dbc->version = version.trimmed();
            dbc->description = description.trimmed();
            dbc->filename = filename.toLatin1().data();
            dbc->initVars(varlist);

            db_transmission.push_back(dbc);
        }
    }
}


/**
 * @brief MaterialManager::saveFile
 * @param filename - name of the file in directory: DATABASE_PATH
 * @param property_file - PropertyFile object
 */
void DatabaseManager::saveFile(DatabaseContent * content)
{   
    QString file_location = Core::instance()->generalSettings->databaseDirectory().toLatin1().data() + content->filename;
    QFile file(file_location);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw LIISimException(tr("Cannot write to: ")+file_location,ERR_IO);
    }

    QTextStream file_output(&file);
    // write header line
     file_output << content->name << ";"
                 << content->type << ";"
                 << content->version << ";"
                 << content->description << ";\n";


    // write var_list

    //std::sort(property_file.var_list.begin(), property_file.var_list.end(), property_file.sortVars);
    varList var_list = content->getVarList();

    for( varList::const_iterator it = var_list.begin(); it != var_list.end(); it++)
    {
        if(content->type == "Gas" || content->type == "Material")
        {
            file_output << it->second.name << ";"
                        << it->second.type << ";"
                        << it->second.parameter[0] << ";"
                        << it->second.parameter[1] << ";"
                        << it->second.parameter[2] << ";"
                        << it->second.parameter[3] << ";"
                        << it->second.parameter[4] << ";"
                        << it->second.parameter[5] << ";"
                        << it->second.parameter[6] << ";"
                        << it->second.parameter[7] << ";"
                        << it->second.parameter[8] << ";"
                        << it->second.source  << ";\n";
        }
        else if(content->type == "GasMixture")
        {
            if(it->second.name == "gas_component")
            {
                file_output << "gas_component;"
                            //<< it->second.description   << ";"      // full name of gas component
                            << it->second.identifier    << ";"      // filename of gas component
                            << it->second.parameter[0]  <<";\n";    // mole fraction: x_i
            }
            else
            {
                //further properties of this gas mixture
                file_output << it->second.name << ";"
                            << it->second.type << ";"
                            << it->second.parameter[0] << ";"
                            << it->second.parameter[1] << ";"
                            << it->second.parameter[2] << ";"
                            << it->second.parameter[3] << ";"
                            << it->second.parameter[4] << ";"
                            << it->second.parameter[5] << ";"
                            << it->second.parameter[6] << ";"
                            << it->second.parameter[7] << ";"
                            << it->second.parameter[8] << ";"
                            << it->second.source  << ";\n";
            }
        }
        else if(content->type == "LIISettings")
        {
            // write first standard settings then channel list
            file_output << it->second.name << ";"                     
                        << it->second.parameter[0] << ";"
                        << it->second.source << ";\n";
        }
    }

    //add channels filters to end of LIISettings database file
    if(content->type == "LIISettings")
    {
        LIISettings* liisettings = dynamic_cast<LIISettings*>(content);
        channelList channel_list = liisettings->channels;
        filterList filter_list = liisettings->filters;

        for(size_t i=0; i < channel_list.size(); i++)
        {
            file_output << "channel;"
                        << channel_list[i].wavelength << ";"
                        << channel_list[i].bandwidth << ";"
                        << channel_list[i].calibration << ";"                        
                        << channel_list[i].pmt_gain << ";"
                        << channel_list[i].pmt_gain_formula_A << ";"
                        << channel_list[i].pmt_gain_formula_B << ";"
                        << channel_list[i].offset << ";\n";
        }

        for(size_t i=0; i < filter_list.size(); i++)
        {
            if(filter_list[i].identifier != "no Filter")
            {
                file_output << "filter;"
                            << filter_list[i].identifier << ";";
                for(size_t j=0; j < channel_list.size(); j++)
                {
                    file_output << QString::number(filter_list[i].list.find(channel_list.at(j).wavelength)->second) << ";";
                }
                file_output << ";\n";
            }
        }
    }
    qDebug() << "DBManager: wrote " << file_location;
    file.close();
}


/**
 * @brief delete a file at given location inside the database
 * @param filename
 */
void DatabaseManager::deleteFile(QString filename)
{
    QString fname = Core::instance()->generalSettings->databaseDirectory() + filename;
    bool res = QFile::remove(fname);

    if(!res){
        MSG_ERR("failed to delete file: "+fname);
    }
}


/**
 * @brief add Pointer to DatabaseContent to Database
 * @param content Pointer to Object which shoul be added
 * @details DatabaseManager takes responsibility of deleting the passed object!
 */
void DatabaseManager::addContentToDB(DatabaseContent *content)
{
    if(content->type == "Gas")
    {
        db_gases.push_back(content);
    }
    else if(content->type == "Material")
    {
        db_materials.push_back(content);
    }
    else if(content->type == "GasMixture")
    {
        db_gasMixtures.push_back(content);
    }
    else if(content->type == "LIISettings")
    {
        db_lIISettings.push_back(content);
    }
    else if(content->type == "LaserEnergy")
    {
        db_laserEnergy.push_back(content);
    }
    else if(content->type == "Spectrum")
    {
        db_spectrum.push_back(content);
    }
    else if(content->type == "Transmission")
    {
        db_transmission.push_back(content);
    }
    else
    {
        QString ctype =content->type;
        delete content;
        throw LIISimException("DBM: failed to add content to Database! invalid type: "+ ctype);
    }
    content->ident = ident_count++;
    saveFile(content);
    emit signal_contentChanged();
}


/**
 * @brief should be called if any changes have been made to database content!
 * @param content
 */
void DatabaseManager::modifiedContent(DatabaseContent *content)
{
    if(content->type == "Gas")
    {
        // update all gas mixtures, which use content
        int noMixes = db_gasMixtures.size();
        GasMixture* curMix;
        GasProperties* curGas = dynamic_cast<GasProperties*>(content);

        for(int i = 0; i < noMixes; i++)
        {
            curMix = getGasMixture(i);
            if(curMix->contains(curGas) >= 0)   // check if gas is in mixture
                saveFile(curMix);             // update GasMixture file
        }
    }
    saveFile(content);

    qDebug() << "DatabaseManager: modified "<< content->ident << Core::instance()->modelingSettings->material().ident;

    emit signal_contentChanged( content->ident );
}


/**
 * @brief delete DatabaseContent form database
 * @param content
 */
void DatabaseManager::removeContentFromDB(DatabaseContent *content)
{
    int index;
    if(content->type == "Gas")
    {
        if(db_gases.contains(content)){

            // remove gas from all mixtures
            int noMixes = db_gasMixtures.size();
            GasMixture* curMix;
            GasProperties* curGas = dynamic_cast<GasProperties*>(content);

            for(int i = 0; i < noMixes; i++)
            {
                curMix = getGasMixture(i);
                if(curMix->removeGas(curGas))   // remove gas from current mixture
                    saveFile(curMix);           // update GasMixture file if gas was part of mixture
            }

            // finally delete the GasPropertie's file and pointer
            deleteFile(content->filename);
            index = db_gases.indexOf(content);
            delete db_gases.at(index);
            db_gases.removeAt(index);
        }
    }
    else if(content->type == "Material")
    {
        if(db_materials.contains(content)){
            deleteFile(content->filename);
            index = db_materials.indexOf(content);
            delete db_materials.at(index);        
            db_materials.removeAt(index);
        }
    }
    else if(content->type == "LIISettings")
    {
        if(db_lIISettings.contains(content)){
            deleteFile(content->filename);
            index = db_lIISettings.indexOf(content);
            delete db_lIISettings.at(index);
            db_lIISettings.removeAt(index);
        }
    }
    else if(content->type == "GasMixture")
    {
        if(db_gasMixtures.contains(content)){
            deleteFile(content->filename);
            index = db_gasMixtures.indexOf(content);

            GasMixture* gm = static_cast<GasMixture*>(db_gasMixtures.at(index));
            gm->clearAllGases();
            delete db_gasMixtures.at(index);
            db_gasMixtures.removeAt(index);
        }
    }
    else if(content->type == "LaserEnergy")
    {
        if(db_laserEnergy.contains(content))
        {
            deleteFile(content->filename);
            index = db_laserEnergy.indexOf(content);
            delete db_laserEnergy.at(index);
            db_laserEnergy.removeAt(index);
        }
    }
    else if(content->type == "Spectrum")
    {
        if(db_spectrum.contains(content))
        {
            deleteFile(content->filename);
            index = db_spectrum.indexOf(content);
            delete db_spectrum.at(index);
            db_spectrum.removeAt(index);
        }
    }
    else if(content->type == "Transmission")
    {
        if(db_transmission.contains(content))
        {
            deleteFile(content->filename);
            index = db_transmission.indexOf(content);
            delete db_transmission.at(index);
            db_transmission.removeAt(index);
        }
    }
    emit signal_contentChanged();
}


/**
 * @brief free memory of all DatabaseContent
 */
void DatabaseManager::cleanUpDB()
{
    // clear current database content
    while(!db_gasMixtures.isEmpty())
    {
        DatabaseContent* p = db_gasMixtures.first();
        static_cast<GasMixture*>(p)->clearAllGases();
        delete p;
        db_gasMixtures.pop_front();
    }
    while(!db_gases.isEmpty())
    {
        DatabaseContent* p = db_gases.first();
        delete p;
        db_gases.pop_front();
    }

    while(!db_materials.isEmpty())
    {
        DatabaseContent* p = db_materials.first();
        delete p;
        db_materials.pop_front();
    }

    while(!db_lIISettings.isEmpty())
    {
        DatabaseContent* p = db_lIISettings.first();
        delete p;
        db_lIISettings.pop_front();
    }

    while(!db_laserEnergy.isEmpty())
    {
        DatabaseContent* p = db_laserEnergy.first();
        delete p;
        db_laserEnergy.pop_front();
    }
    while(!db_spectrum.isEmpty())
    {
        DatabaseContent *p = db_spectrum.first();
        delete p;
        db_spectrum.pop_front();
    }
    while(!db_transmission.isEmpty())
    {
        DatabaseContent *p = db_transmission.first();
        delete p;
        db_transmission.pop_front();
    }
}


int DatabaseManager::indexOfGas(QString fname)
{
    for(int i=0; i<db_gases.size(); i++)
        if(db_gases.at(i)->filename == fname)
            return i;
    return -1;
}

int DatabaseManager::indexOfGasMixture(QString fname)
{
    for(int i=0; i<db_gasMixtures.size(); i++)
        if(db_gasMixtures.at(i)->filename == fname)
            return i;
    return -1;
}

int DatabaseManager::indexOfLIISettings(QString fname)
{
    for(int i=0; i<db_lIISettings.size(); i++)
        if(db_lIISettings.at(i)->filename == fname)
            return i;
    return -1;
}

int DatabaseManager::indexOfMaterial(QString fname)
{    
    for(int i=0; i<db_materials.size(); i++)
        if(db_materials.at(i)->filename == fname)
            return i;
    return -1;
}


int DatabaseManager::indexOfLaserEnergy(QString fname)
{
    for(int i = 0; i < db_laserEnergy.size(); i++)
        if(db_laserEnergy.at(i)->filename == fname)
            return i;
    return -1;
}


int DatabaseManager::indexOfSpectrum(QString fname)
{
    for(int i = 0; i < db_spectrum.size(); i++)
        if(db_spectrum.at(i)->filename == fname)
            return i;
    return -1;
}


int DatabaseManager::indexOfTransmission(QString fname)
{
    for(int i = 0; i < db_transmission.size(); i++)
        if(db_transmission.at(i)->filename == fname)
            return i;
    return -1;
}
