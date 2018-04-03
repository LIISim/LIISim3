#include "fititerationresult.h"

#include <QStringList>

FitIterationResult::FitIterationResult(int noParams)
    : QVector(noParams,0.0)
{

}

FitIterationResult::~FitIterationResult()
{

}

QString FitIterationResult::toString()
{
    QString res;
    for(int i = 0; i < size(); i++)
    {
        res.append(QString::number(at(i))+"\t");
    }
    return res;
}

void FitIterationResult::writeToXML(QXmlStreamWriter &w)
{
    w.writeStartElement("FitIterationResult");

    QString values;
    for(int i = 0; i < size(); i++)
    {
        values.append(QString::number(this->at(i)));
        if(i!=size()-1)
           values.append(";");
    }

    w.writeAttribute("values",values);
    w.writeEndElement(); // FitIterationResult
}

bool FitIterationResult::readFromXml(QXmlStreamReader &r)
{
    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "FitIterationResult")
        return false;

    QXmlStreamAttributes a = r.attributes();
    QStringList values = a.value("values").toString().split(';');

    for(int i =0; i < values.size(); i++)
        this->append(values[i].toDouble());

    return true;
}



