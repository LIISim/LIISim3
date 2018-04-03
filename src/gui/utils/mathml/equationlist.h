#ifndef EQUATIONLIST_H
#define EQUATIONLIST_H

#include <QMap>
#include <QWidget>
#include <QLabel>

/**
 * @brief The EquationList class loads and stores Property equation list
 */
class EquationList: public QWidget
{
    Q_OBJECT
public:
    EquationList();

    // retrieved from Property class
    QStringList eqTypeList; //

    QMap<QString ,QString> eqList;
    QMap<QString, QPixmap> eqDefList;

    QLabel* defaultEq(QString type);
    QLabel* parsedEq(QString type, QList<double> params);


private:
    QString loadFormula(QString type);

};

#endif // EQUATIONLIST_H
