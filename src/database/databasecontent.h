#ifndef DATABASECONTENT_H
#define DATABASECONTENT_H

#include <QString>
#include <QDebug>
#include <map>
#include "structure/property.h"
#include "../general/channel.h"
#include "../general/filter.h"


typedef  std::pair<QString, Property> varPair;
typedef  std::multimap<QString, Property> varList;
typedef  std::map<int, Property>::const_iterator varListConstIterator;

typedef  QList<Channel> channelList;
typedef  QList<Filter> filterList;

/**
 * @brief abstract base class for all classes which should be stored/loaded from database
 */
class DatabaseContent
{
public:
    DatabaseContent();
    virtual ~DatabaseContent(){}
    QString filename;
    QString name;
    QString type;
    QString version;
    QString description;
    int ident;

    /**
     * @brief return Property List of variables
     * @return
     */
    virtual varList getVarList()=0;

    /**
     * @brief set variables from List of Properties
     */
    virtual void initVars(varList)=0;
};

#endif // DATABASECONTENT_H
