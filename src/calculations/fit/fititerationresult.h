#ifndef FITITERATIONRESULT_H
#define FITITERATIONRESULT_H

#include <QVector>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

/**
 * @brief The FitIterationResult class stores
 */
class FitIterationResult : public QVector<double>
{
public:
    FitIterationResult(int noParams);
    ~FitIterationResult();

    void writeToXML(QXmlStreamWriter &w);
    bool readFromXml(QXmlStreamReader &r);
    QString toString();
};

#endif // FITITERATIONRESULT_H
