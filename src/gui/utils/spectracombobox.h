#ifndef SPECTRACOMBOBOX_H
#define SPECTRACOMBOBOX_H

#include <QComboBox>
#include "../../database/structure/spectrumdbe.h"


/**
 * @brief The SpectraComboBox class represents
 * an intelligent combobox for the selection of spectra from the database.
 * It listens to changes of the DatabaseManager and updates
 * its selection entries based on the available Spectra.
 */
class SpectraComboBox : public QComboBox
{
    Q_OBJECT

    public:
        explicit SpectraComboBox(QWidget *parent =0);
        ~SpectraComboBox();

        SpectrumDBE currentSpectrum();

    private slots:
        void onDatabaseContentChanged(int id = -1);

};

#endif // SPECTRACOMBOBOX_H
