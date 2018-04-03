#ifndef DATABASEEDITOR_H
#define DATABASEEDITOR_H

#include "../../general/LIISimMessageType.h"
#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QStatusBar>
#include <QLabel>

class GasEditor;
class MaterialEditor;
class DatabaseManager;
class GasMixEditor;
class LiiSettingsEditor;
class LaserEnergyEditor;
class SpectrumEditor;
class TransmissionEditor;

/**
 * @brief The DatabaseEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class DatabaseEditor : public QWidget
{
    Q_OBJECT
public:
    explicit DatabaseEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager* dbm);
private:
    DatabaseManager* db_manager;

    QVBoxLayout* lay_main_v_outer;
    QVBoxLayout* lay_main_v;
    QHBoxLayout* lay_db_path;
    QLabel* lbDbPath;
    QLabel* lbDbPathDescr;
    QPushButton* bOpenFolder;
    QPushButton* bScanDir;
    QPushButton* bChangeDir;
    QTabWidget* tabs;

    GasEditor* gasEdit;
    MaterialEditor* matEdit;
    GasMixEditor* gasmixEdit;
    LiiSettingsEditor* liisettEdit;
    LaserEnergyEditor *laserEnergyEdit;
    SpectrumEditor *spectrumEditor;
    TransmissionEditor *transmissionEditor;


signals:

    /**
     * @brief request Core to scan the database directory
     * @param databasepath
     */
    void signal_requestDbScan();

public slots:
    void  slot_startDbScan();
    void  slot_scanFinished();
    void  slot_onChangeFolder();

    void  onGeneralSettingsChanged();

private slots:

    void onOpenExplorer();
    void onDatabaseContentChanged(int dbc_indent = -1);

};

#endif // DATABASEEDITOR_H
