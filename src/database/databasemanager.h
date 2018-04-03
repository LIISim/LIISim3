#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


#include <QString>
#include <QObject>
#include <QMultiMap>


#include "../general/LIISimMessageType.h"
#include "../database/structure/gasproperties.h"
#include "../database/structure/material.h"
#include "../database/structure/liisettings.h"
#include "../database/structure/gasmixture.h"
#include "../database/structure/laserenergy.h"
#include "../database/structure/spectrumdbe.h"
#include "../database/structure/transmissiondbe.h"

#include "../gui/utils/mathml/equationlist.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
private:
    QString databasepath;

    bool isLoading;

    // database gases, materials ...

    QList<DatabaseContent*>  db_gases;
    QList<DatabaseContent*>  db_materials;
    QList<DatabaseContent*>  db_lIISettings;
    QList<DatabaseContent*>  db_gasMixtures;
    QList<DatabaseContent*>  db_laserEnergy;
    QList<DatabaseContent*>  db_spectrum;
    QList<DatabaseContent*>  db_transmission;

    QMultiMap<GasMixture*,Property> gasmixRequests;

    void cleanUpDB();

    static int ident_count;
public:
    DatabaseManager();
    ~DatabaseManager();



    void loadFileToDatabase(const QString &filename);

    // removes file from database
    void removeContentFromDB(DatabaseContent *content);
    void addContentToDB(DatabaseContent *content);
    void modifiedContent(DatabaseContent* content);

    void saveFile(DatabaseContent *content);
    void deleteFile(QString filename);

    EquationList* eqList;

    QList<DatabaseContent*> *getGases();
    QList<DatabaseContent*> *getMaterials();
    QList<DatabaseContent*> *getLIISettings();
    QList<DatabaseContent*> *getGasMixtures();
    QList<DatabaseContent*> *getLaserEnergy();
    QList<DatabaseContent*> *getSpectra();
    QList<DatabaseContent*> *getTransmissions();

    GasProperties *getGas(int index);
    Material *getMaterial(int index);
    LIISettings *getLIISetting(int index);
    GasMixture *getGasMixture(int index);
    LaserEnergy *getLaserEnergy(int index);
    SpectrumDBE *getSpectrum(int index);
    TransmissionDBE* getTransmission(int index);

    GasProperties* gas(int dbc_id);
    Material* material(int dbc_id);
    LIISettings* liiSetting(int dbc_id);
    GasMixture* gasMixture(int dbc_id);
    LaserEnergy* laserEnergy(int dbc_id);
    SpectrumDBE* spectrum(int dbc_id);
    TransmissionDBE* transmission(int dbc_id);

    int indexOfGas(QString fname);
    int indexOfMaterial(QString fname);
    int indexOfGasMixture(QString fname);
    int indexOfLIISettings(QString fname);
    int indexOfLaserEnergy(QString fname);
    int indexOfSpectrum(QString fname);
    int indexOfTransmission(QString fname);

    QString getDataBasePath();

public slots:

    void slot_scanDatabase();


signals:

    /**
     * @brief used to notify DatabaseEditor to (re)initialize all GUIs
     */
    void signal_scanDatabaseFinished();


    /**
     * @brief signal_contentChanged is emitted if the content of the database has changed, should be handeled by all widgets which use contents from the database
     * @param type the type of the modified db content
     * @param dbc_indent id of dbc
     */
    void signal_contentChanged(int dbc_indent = -1);

};

#endif // DATABASEMANAGER_H
