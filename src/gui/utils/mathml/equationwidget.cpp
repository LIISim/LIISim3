#include "equationwidget.h"



/**
 * @brief The EquationWidget class
 * Qwt MathML Renderer ( see http://qwt.sourceforge.net )
 * Adapted from: https://github.com/uwerat/qwt-mml-dev (contains some mml examples)
 */

#include "externalLibraries/mathml/qwt_mml_document.h"
#include "externalLibraries/mathml/qwt_mathml_text_engine.h"

#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>




EquationWidget::EquationWidget( QWidget *parent ):
    QWidget( parent ),
    d_fontSize( 8 ),
    d_transformation( true ),
    d_scale( false ),
    d_rotation( 0 )
{
}


QString EquationWidget::formula() const
{
    return d_formula;
}


void EquationWidget::setFormula( const QString &formula )
{
    d_formula = formula;
    update();
}

void EquationWidget::setFontSize( const qreal &fontSize )
{
    d_fontSize = fontSize;
    update();
}

void EquationWidget::setTransformation( const bool &transformation )
{
    d_transformation = transformation;
    update();
}

void EquationWidget::setScale( const bool &scale )
{
    d_scale = scale;
    update();
}

void EquationWidget::setRotation( const qreal &rotation )
{
    d_rotation = rotation;
    update();
}

void EquationWidget::setDrawFrames( const bool &drawFrames )
{
    d_drawFrames = drawFrames;
    update();
}

void EquationWidget::setColors( const bool &colors )
{
    d_colors = colors;
    update();
}

void EquationWidget::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    painter.fillRect( event->rect(), Qt::white );

    renderFormula( &painter );
}

void EquationWidget::renderFormula( QPainter *painter ) const
{
    QwtMathMLDocument doc;
    doc.setContent( d_formula );
//    if ( d_colors )
//    {
//        doc.setBackgroundColor( Qt::darkCyan );
//        doc.setForegroundColor( Qt::yellow );
//    }
//    else
//    {
//        doc.setBackgroundColor( Qt::white );
//        doc.setForegroundColor( Qt::black );
//    }
    doc.setBaseFontPointSize( d_fontSize );
#ifdef MML_TEST
    doc.setDrawFrames( d_drawFrames );
#endif

    QRect docRect;
    docRect.setSize( doc.size() );
    docRect.moveCenter( rect().center() );

    if ( d_transformation )
    {
        const double scaleF = d_scale ? 2.0 : 1.0;

        painter->save();

        painter->translate( docRect.center() );
        painter->rotate( d_rotation );
        painter->scale( scaleF, scaleF );
        painter->translate( docRect.topLeft() - docRect.center() );
        doc.paint( painter, QPoint( 0.0, 0.0 ) );

        painter->restore();
    }
    else
    {
        doc.paint( painter, docRect.topLeft() );
    }
}

