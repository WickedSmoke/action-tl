#include <stdio.h>
#include <QApplication>
#include <QBoxLayout>
#include <QDropEvent>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <QPainter>
#include <QStandardItemModel>
#include "Timeline.h"

#define CSTR(qs)    qs.toLocal8Bit().constData()

#define ACT_COUNT   9
const char* actionName[ ACT_COUNT ] = {
    "Walk",
    "Run",
    "Attack",
    "Defend",
    "Drink",
    "Equip",
    "Pickup",
    "Wait 1",
    "Wait 2"
};


// QLabel with direct color control.
class ColorLabel : public QLabel
{
public:
    ColorLabel( const QString& text, QWidget* parent = NULL )
        : QLabel(text, parent) {}

    void setColor( const QColor& col )
    {
        QPalette mpal( palette() );
        mpal.setColor( QPalette::Text, col );
        setPalette( mpal );
    }
protected:
    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        QFontMetrics fm( fontMetrics() );
        QColor pcol = palette().color( QPalette::Text );

        p.setPen( pcol );
        p.setBrush( QBrush() );
        p.drawRect( 0, 0, width()-1, height()-1 );
        p.drawText( 4, height() - fm.descent(), text() );
    }
};


Timeline::Timeline( QWidget* parent ) : QWidget(parent)
{
    setAcceptDrops(true);
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    _lo = new QHBoxLayout(this);
}


void Timeline::addSubject( const QString& name )
{
#if 1
    ColorLabel* cl = new ColorLabel(name);
    cl->setFixedWidth( 120 );
    cl->setColor( Qt::black );
    _lo->addWidget( cl );
#else
    _lo->addWidget( new QLabel(name) );
#endif
}


void Timeline::dragEnterEvent(QDragEnterEvent* ev)
{
#if 0
    QStringList fmt = ev->mimeData()->formats();
    for (int i = 0; i < fmt.size(); ++i)
        printf( "fmt %s\n", CSTR(fmt.at(i)) );
#endif

    if( _lo->count() > 0 &&
        ev->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist") )
    {
         ev->acceptProposedAction();
    }
}


void Timeline::dropEvent(QDropEvent* ev)
{
    QStandardItemModel model;
    model.dropMimeData( ev->mimeData(), Qt::CopyAction, 0, 0, QModelIndex() );
    //printf( "drop %s\n", CSTR(model.item(0, 0)->text()) );

    ColorLabel* cl = new ColorLabel( model.item(0, 0)->text() );
    cl->setColor( Qt::white );
    _lo->addWidget( cl );

    ev->acceptProposedAction();
}


ActionTimeline::ActionTimeline( QWidget* parent ) : QWidget(parent)
{
    setWindowTitle( "Action Timeline" );

    QBoxLayout* lo = new QHBoxLayout(this);

    Timeline* tl = new Timeline;
    tl->addSubject( "<Person 1yg>" );
    lo->addWidget( tl );

    QGridLayout* grid = new QGridLayout;
    lo->addLayout( grid );

        QListWidget* list = new QListWidget;
        list->setDragEnabled(true);
        list->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );
        for( int i = 0; i < ACT_COUNT; ++i )
            list->addItem( QString(actionName[i]) );
        grid->addWidget( list, 0, 0, 1, 2 );

        QLabel* label = new QLabel("Scale:");
        label->setAlignment( Qt::AlignRight );
        grid->addWidget( label, 1, 0 );

        QDoubleSpinBox* spin = new QDoubleSpinBox;
        spin->setRange( 0.1, 5.0 );
        spin->setSingleStep( 0.1 );
        spin->setValue( 1.0 );
        grid->addWidget( spin, 1, 1 );
}


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    ActionTimeline win;
    win.resize( 948, 270 );
    win.show();
    return app.exec();
}
