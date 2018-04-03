#ifndef FILTER_H
#define FILTER_H


#include <map>
#include <QString>

class Filter
{
public:

    Filter();
    Filter(QString identifier);

    QString identifier;
    std::multimap<int, double> list;

    double getTransmission(int wavelength);

    QList<double> getTransmissions();

    static const QString filterNotSetIdentifier;
};

#endif // FILTER_H
