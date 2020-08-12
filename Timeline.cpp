#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <QApplication>
#include <QBoxLayout>
#include <QPushButton>
#include <QDropEvent>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QStandardItemModel>
#include "Timeline.h"

#define CSTR(qs)    qs.toLocal8Bit().constData()

#define ACT_COUNT   9
static const char* actionName[ ACT_COUNT ] = {
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

static const int actionDur[ ACT_COUNT ] = {
    2, 2, 3, 2, 5, 1, 5, 1, 2
};


static int actionId( const char* str )
{
    for( int i = 0; i < ACT_COUNT; ++i )
    {
        if( strcmp( actionName[i], str ) == 0 )
            return i;
    }
    return 0;
}


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
        int h = height();
        int pad = (h - fm.height()) / 2;

        p.setPen( pcol );
        p.setBrush( QBrush() );
        p.drawRect( 0, 0, width()-1, h-1 );
        p.drawText( 4, h - fm.descent() - pad, text() );
    }
};


//----------------------------------------------------------------------------


Timeline::Timeline( QWidget* parent ) : QWidget(parent)
{
    _pixPerSec = 70;
    _startTime = 0;
    _subject = -1;

    setAcceptDrops(true);
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    _lo = new QVBoxLayout(this);
    _lo->setSpacing(0);
}


void Timeline::select( int index )
{
    if( _subject != index )
    {
        ColorLabel* cl;
        if( (cl = selectedNameLabel()) )
            cl->setColor( Qt::black );

        _subject = index;

        if( (cl = selectedNameLabel()) )
            cl->setColor( Qt::darkYellow );
    }
}


ColorLabel* Timeline::selectedNameLabel()
{
    if( hasSelection() )
    {
        ColorLabel* cl;
        QLayout* slo;
        QLayoutItem* item = _lo->itemAt( _subject );
        if( item && (slo = item->layout()) )
        {
            item = slo->itemAt( 0 );
            if( item && (cl = static_cast<ColorLabel*>(item->widget())) )
                return cl;
        }
    }
    return NULL;
}


void Timeline::removeSelected()
{
    if( hasSelection() )
    {
        //TODO
    }
}


void Timeline::addSubject( const QString& name )
{
#if 1
    ColorLabel* cl = new ColorLabel(name);
    cl->setFixedSize( 120, 20 );
    cl->setColor( Qt::black );

    QBoxLayout* slo = new QHBoxLayout;
    slo->addWidget( cl );
    slo->addStretch();
    _lo->addLayout( slo );
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

    if( hasSelection() &&
        ev->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist") )
    {
         ev->acceptProposedAction();
    }
}


void Timeline::dropEvent(QDropEvent* ev)
{
    QStandardItemModel model;
    model.dropMimeData( ev->mimeData(), Qt::CopyAction, 0, 0, QModelIndex() );
    QString name( model.item(0, 0)->text() );
    //printf( "drop %s\n", CSTR(name) );

    int duration = actionDur[ actionId(CSTR(name)) ];

    ColorLabel* cl = new ColorLabel( name );
    cl->setFixedSize( _pixPerSec * duration - 1, 20 );
    cl->setColor( Qt::darkGray );

    assert( hasSelection() );

    QBoxLayout* slo;
    QLayoutItem* item = _lo->itemAt( _subject );
    if( item && (slo = static_cast<QBoxLayout*>(item->layout())) )
    {
        slo->insertWidget( slo->count() - 1, cl );
        ev->acceptProposedAction();
    }
}


void Timeline::contextMenuEvent(QContextMenuEvent* ev)
{
    QWidget* wid = childAt( ev->pos() );
    if( wid )
    {
        QMenu menu;
        QAction* rename;
        QAction* resize;
        QAction* act;

        rename = menu.addAction( "Rename" );
        resize = menu.addAction( "Set Duration" );
        menu.addSeparator();
        menu.addAction( "Delete" );

        act = menu.exec( ev->globalPos() );
        if( act )
        {
            ColorLabel* cl = static_cast<ColorLabel*>( wid );
            if( act == rename )
            {
                bool ok;
                QString text = QInputDialog::getText(this, "Rename", "Name:",
                                        QLineEdit::Normal, cl->text(), &ok );
                if( ok && ! text.isEmpty() )
                    cl->setText( text );
            }
            else if( act == resize )
            {
                bool ok;
                double dur = QInputDialog::getDouble(this,
                        "Set Duration", "Duration:",
                        double(cl->width()) / double(_pixPerSec), 0.1, 5.0,
                        1, &ok );
                if( ok )
                    cl->setFixedWidth( int(dur * _pixPerSec) - 1 );
            }
            else
            {
                // TODO: Remove subject row if wid is name.
                wid->deleteLater();
            }
        }
    }
}


void Timeline::mousePressEvent(QMouseEvent* ev)
{
    QPoint pnt( ev->pos() );
    QLayoutItem* item;
    int count = _lo->count();
    for( int i = 0; i < count; ++i )
    {
        item = _lo->itemAt(i);
        if( item->geometry().contains( pnt ) )
        {
            select(i);
            return;
        }
    }
}


void Timeline::wheelEvent(QWheelEvent* ev)
{
    int count = _lo->count();
    if( ! count )
        return;

    int n = 0;
    if( hasSelection() )
    {
        if( ev->angleDelta().y() > 0 )
        {
            n = (_subject > 0) ? _subject-1 : count-1;
        }
        else
        {
            n = _subject+1;
            if( n == count )
                n = 0;
        }
    }
    select(n);
}


//----------------------------------------------------------------------------


ActionTimeline::ActionTimeline( QWidget* parent ) : QWidget(parent)
{
    setWindowTitle( "Action Timeline" );

    _tl = new Timeline;

    QListWidget* list = new QListWidget;
    list->setDragEnabled(true);
    list->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );
    for( int i = 0; i < ACT_COUNT; ++i )
        list->addItem( QString(actionName[i]) );

    QPushButton* btn = new QPushButton("+");
    connect( btn, SIGNAL(clicked(bool)), this, SLOT(newSubject()) );

    QLabel* label = new QLabel("Scale:");
    label->setAlignment( Qt::AlignRight );

    QDoubleSpinBox* spin = new QDoubleSpinBox;
    spin->setRange( 0.1, 5.0 );
    spin->setSingleStep( 0.1 );
    spin->setValue( 1.0 );

    QGridLayout* grid = new QGridLayout(this);
    grid->addWidget( _tl,   0, 0, 1, 2 );
    grid->addWidget( list,  0, 2, 1, 2 );
    grid->addWidget( btn,   1, 0 );
    grid->addWidget( label, 1, 2 );
    grid->addWidget( spin,  1, 3 );
    grid->setColumnStretch( 1, 1 );
}


void ActionTimeline::loadSubjects( int count, char** names )
{
    if( count > 0 )
    {
        for( int i = 0; i < count; ++i )
            _tl->addSubject( names[i] );
        _tl->select( 0 );
    }
}


void ActionTimeline::newSubject()
{
    _tl->addSubject( "<unnamed>" );
}


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    ActionTimeline win;
    win.resize( 980, 270 );
    win.show();
    if( argc > 1 )
        win.loadSubjects( argc-1, argv+1 );
    else
        win.newSubject();
    return app.exec();
}
