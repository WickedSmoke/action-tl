#include <stdio.h>
#include <string.h>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
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
#include <QRandomGenerator>
#include <QStandardItemModel>
#include "Timeline.h"

#define CSTR(qs)    qs.toLocal8Bit().constData()

#define ACT_COUNT   12
static const char* actionName[ ACT_COUNT ] = {
    "Walk 10",
    "Run 10",
    "Attack",
    "Defend",

    "Shoot",
    "Drink",
    "Draw",
    "Equip",

    "Pickup",
    "Throw",
    "Wait 1",
    "Wait 2"
};

static const int actionDur[ ACT_COUNT ] = {
    3, 1, 5, 4,   5, 5, 1, 6,   3, 3, 1, 2
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


#define SUBJECT_NONE    -1
#define SUBJECT_HEIGHT(cl)  cl->fontMetrics().height()+6


Timeline::Timeline( QWidget* parent ) : QWidget(parent)
{
    _pixPerSec = 70;
    _startTime = 0;
    _subject = SUBJECT_NONE;

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


void Timeline::saveImage()
{
    QString fn( "/tmp/action-%1-%2sec.jpeg" );
    QImage img( size(), QImage::Format_RGB888 );
    render( &img );
    img.save( fn.arg( QCoreApplication::applicationPid() ).arg( _startTime ) );
}


void Timeline::advance( int sec )
{
    QLayout* slo;
    QLayoutItem* item;
    QWidget* wid;
    int count, sc;
    int pdur, rem, w;

    if( sec < 1 )
        return;

    pdur = _pixPerSec * sec;
    count = _lo->count();
    for( int i = 0; i < count; ++i )
    {
        item = _lo->itemAt(i);
        if( item && (slo = item->layout()) )
        {
            rem = pdur;
            sc = slo->count();
            for( int ai = 1; ai < sc; ++ai )
            {
                item = slo->itemAt(ai);
                wid = item->widget();

                w = item->geometry().width();
                if( rem < w )
                {
                    if( wid )
                        wid->setFixedWidth( w - rem );
                }
                else
                {
                    rem -= w + 1;
                    if( wid )
                        wid->deleteLater();
                }

                if( rem < 1 )
                    break;
            }
        }
    }

    _startTime += sec;
}


void Timeline::setStartTime( int sec )
{
    _startTime = sec;
}


void Timeline::addSubject( const QString& name )
{
#if 1
    ColorLabel* cl = new ColorLabel(name);
    cl->setFixedSize( 120, SUBJECT_HEIGHT(cl) );
    cl->setColor( Qt::black );

    QBoxLayout* slo = new QHBoxLayout;
    slo->addWidget( cl );
    slo->addStretch();
    _lo->addLayout( slo );
#else
    _lo->addWidget( new QLabel(name) );
#endif
}


bool Timeline::appendAction( int id )
{
    if( hasSelection() )
    {
        QBoxLayout* slo;
        QLayoutItem* item = _lo->itemAt( _subject );
        if( item && (slo = static_cast<QBoxLayout*>(item->layout())) )
        {
            ColorLabel* cl = new ColorLabel( actionName[id] );
            cl->setFixedSize( _pixPerSec * actionDur[id] - 1,
                              SUBJECT_HEIGHT(cl) );
            cl->setColor( Qt::darkGray );

            slo->insertWidget( slo->count() - 1, cl );
            return true;
        }
    }
    return false;
}


void Timeline::dragEnterEvent(QDragEnterEvent* ev)
{
#if 0
    QStringList fmt = ev->mimeData()->formats();
    for (int i = 0; i < fmt.size(); ++i)
        printf( "fmt %s\n", CSTR(fmt.at(i)) );
#endif

    if( ev->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist") )
         ev->acceptProposedAction();
}


void Timeline::dragMoveEvent(QDragMoveEvent* ev)
{
    int n = subjectAt( ev->pos() );
    if( n != SUBJECT_NONE )
        select(n);
    if( hasSelection() )
        ev->acceptProposedAction();
}


void Timeline::dropEvent(QDropEvent* ev)
{
    QStandardItemModel model;
    model.dropMimeData( ev->mimeData(), Qt::CopyAction, 0, 0, QModelIndex() );
    QString name( model.item(0, 0)->text() );
    //printf( "drop %s\n", CSTR(name) );

    if( appendAction( actionId(CSTR(name)) ) )
        ev->acceptProposedAction();
}


void Timeline::contextMenuEvent(QContextMenuEvent* ev)
{
    QWidget* wid = childAt( ev->pos() );
    if( wid )
    {
        QMenu menu;
        QAction* resolv;
        QAction* rename;
        QAction* resize;
        QAction* act;

        resolv = menu.addAction( "Resolve" );
        rename = menu.addAction( "Rename" );
        resize = menu.addAction( "Set Duration" );
        menu.addSeparator();
        menu.addAction( "Delete" );

        act = menu.exec( ev->globalPos() );
        if( act )
        {
            ColorLabel* cl = static_cast<ColorLabel*>( wid );
            if( act == resolv )
            {
                int n = QRandomGenerator::global()->bounded(20) + 1;
                QString text( cl->text() );
                text.append( " %1" );
                cl->setText( text.arg(n) );
            }
            else if( act == rename )
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
                        double(cl->width()) / double(_pixPerSec), 0.1, 9.0,
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


int Timeline::subjectAt(const QPoint& pnt) const
{
    QLayoutItem* item;
    int count = _lo->count();
    for( int i = 0; i < count; ++i )
    {
        item = _lo->itemAt(i);
        if( item->geometry().contains( pnt ) )
            return i;
    }
    return SUBJECT_NONE;
}


void Timeline::mousePressEvent(QMouseEvent* ev)
{
    int n = subjectAt( ev->pos() );
    if( n != SUBJECT_NONE )
        select(n);
}


void Timeline::wheelEvent(QWheelEvent* ev)
{
    int count = _lo->count();

    if( ev->modifiers() & Qt::ShiftModifier )
    {
        if( count > 1 && hasSelection() )
        {
            int n;
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

            QLayoutItem* item = _lo->takeAt( _subject );
            _lo->insertItem( n, item );
            _subject = n;   // No need to call select().
        }
    }
    else if( count )
    {
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
    {
        new QListWidgetItem( QString(actionName[i]), list, i );
    }
    connect( list, SIGNAL(itemActivated(QListWidgetItem*)),
             this, SLOT(addAction(QListWidgetItem*)) );

    QPushButton* add = new QPushButton("+");
    connect( add, SIGNAL(clicked(bool)), this, SLOT(newSubject()) );

    _turn = new QComboBox;
    _turn->addItem( "6 sec" );
    _turn->addItem( "10 sec" );

    QPushButton* adv = new QPushButton(">>");
    connect( adv, SIGNAL(clicked(bool)), this, SLOT(advance()) );

    _time = new QLineEdit( "0" );
    _time->setFixedSize( 60, _turn->sizeHint().height() );
    _time->setValidator( new QIntValidator(0, 999, this) );
    connect( _time, SIGNAL(textEdited(const QString&)),
             this, SLOT(timeEdited()) );

    QBoxLayout* lo = new QHBoxLayout;
    lo->addWidget( add );
    lo->addSpacing( 20 );
    lo->addWidget( _turn );
    lo->addWidget( adv );
    lo->addWidget( _time );
    lo->addStretch();

    QGridLayout* grid = new QGridLayout(this);
    grid->addWidget( _tl,   0, 0 );
    grid->addWidget( list,  0, 1, 2, 2 );
    grid->addLayout( lo,    1, 0 );
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


void ActionTimeline::addAction(QListWidgetItem* item)
{
    _tl->appendAction( item->type() );
}


void ActionTimeline::advance()
{
    _tl->saveImage();
    _tl->advance( _turn->currentIndex() ? 10 : 6 );
    _time->setText( QString::number( _tl->startTime() ) );
}


void ActionTimeline::timeEdited()
{
    _tl->setStartTime( _time->text().toInt() );
}


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    ActionTimeline win;
    win.resize( 980, 350 );
    win.show();
    if( argc > 1 )
        win.loadSubjects( argc-1, argv+1 );
    else
        win.newSubject();
    return app.exec();
}
