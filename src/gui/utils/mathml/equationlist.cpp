#include "equationlist.h"


/**
 * @brief The EquationList class
 * Qwt MathML Renderer ( see http://qwt.sourceforge.net )
 * Adapted from: https://github.com/uwerat/qwt-mml-dev (contains some mml examples)
 * http://www.mathmlcentral.com/
 * http://www.wiris.com/editor/demo/de/mathml-latex
 */
#include "externalLibraries/mathml/qwt_mathml_text_engine.h"
#include "externalLibraries/mathml/qwt_mml_document.h"

#include "../../core.h"
#include "../../general/LIISimMessageType.h"

#include <QFile>
#include <QPainter>
#include <QEvent>

#include <QLabel>


EquationList::EquationList()
{
    // take list from Property class (const, case, poly,...)
    eqTypeList = Property::eqTypeList;

    // load file contents and save them to lookup table eqList
    for(int i = 0; i < eqTypeList.size(); i++)
    {
        // filll list with raw equation strings
        QString type = eqTypeList.at(i);
        QString eqs = loadFormula(type);

        eqList.insert(type, eqs);

        // parse default equations on initialization
        QwtMathMLDocument doc;        
        doc.setContent(eqs);
        doc.setBaseFontPointSize(10);

        //doc.setFontName(QwtMathMLDocument::NormalFont, "");

        //doc.setFontName(QwtMathMLDocument::FrakturFont, "ftest");
        //doc.setFontName(QwtMathMLDocument::ScriptFont, "ftest");
        //doc.setFontName(QwtMathMLDocument::DoublestruckFont, "ftest");

        QSize dsize = doc.size();
        QPointF cpoint;


        // set minimum height
        if(dsize.height() < 15)
            dsize.setHeight(15);

        // handle only-text-rendering-problems
        if(type == "error")
        {
            dsize.setHeight(20);
            dsize.setWidth(200);
            cpoint = QPointF(5.0, 0.0);
        }
        else
            cpoint = QPointF(0.0, 0.0);

        // image will be stored in QPixmap -> will only rendered during initialization (performance issues)
        QPixmap pixmap(dsize);

        QPainter painter(&pixmap);

        painter.fillRect(pixmap.rect(), Qt::white);
        painter.translate(cpoint);

        doc.paint(&painter, QPoint( 0.0, 0.0 ) );

        eqDefList.insert(type, pixmap);

        // Debug output:
        //pixmap.save(Core::rootDir + "eq_" + type +".png");
    }
}


/**
 * @brief EquationList::defaultEq returns QLabel containing the equation pixmap
 * @param type
 * @return
 */
QLabel* EquationList::defaultEq(QString type)
{
    if(!eqDefList.contains(type))
    {
        qDebug() << QString("Eq error: %0 does not exist").arg(type);
        type = "error"; // displays resources/equations/error.mml
    }

    QLabel* lbl = new QLabel();
    lbl->setPixmap(eqDefList.value(type));
    return lbl;
}


QLabel* EquationList::parsedEq(QString type, QList<double> params)
{
    // TODO:
    // check size of params
    // render equation with parameter set

    return defaultEq(type);
}


/**
 * @brief EquationList::loadFormula loads <type> formula from resources folder
 * @param type Property type
 */
QString EquationList::loadFormula(QString type)
{
    QString fileName = Core::rootDir + QString("resources/equations/%0.mml").arg(type);
    QString err;

    QFile file( fileName );
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        err = QString("EquationList: equation file not found: %0").arg(fileName);
        MESSAGE(err ,LIISimMessageType::WARNING);
        return QString("Error");
    }

    const QByteArray document = file.readAll();
    file.close();

    QwtMathMLTextEngine eng;

    if(eng.mightRender(document))
        return document;
    else
    {
        err = QString("EquationList: syntax error while loading equation file: %0 - (please check file: https://validator.w3.org)").arg(fileName);
        MESSAGE(err ,LIISimMessageType::WARNING);
    }
    return QString("Error");
}
