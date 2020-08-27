/*
  Action Timeline
  Copyright 2020 Karl Robillard

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


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
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QStandardItemModel>
#include <QWidgetAction>
#include "PixmapChooser.h"
#include "Timeline.h"

#define CSTR(qs)    qs.toLocal8Bit().constData()
#define RGB_RESOLVE qRgb(238, 232, 205)
#define RGB_SELECT  qRgb(135, 206, 235)

enum ColorLabelType
{
    CTYPE_NAME,
    CTYPE_ACTION
};


//----------------------------------------------------------------------------


int ActionTable::defineAction( const char* aname, const char* end, int dur )
{
    int id = _entry.size() >> 1;

    _entry.push_back( _strings.size() );
    _entry.push_back( dur );

    _strings.insert( _strings.end(), aname, end );
    _strings.push_back( '\0' );

    return id;
}


int ActionTable::actionId( const char* str ) const
{
    int count = _entry.size();
    for( int i = 0; i < count; i += 2 )
    {
        if( strcmp( _strings.data() + _entry[i], str ) == 0 )
            return i >> 1;
    }
    return -1;
}


//----------------------------------------------------------------------------


// QLabel with direct color control.
class ColorLabel : public QLabel
{
public:
    ColorLabel( const QString& text, QWidget* parent = NULL )
        : QLabel(text, parent), fill(false), tokenCount(0) {}

    void setColor( const QColor& col )
    {
        QPalette mpal( palette() );
        mpal.setColor( QPalette::Text, col );
        setPalette( mpal );
    }

    void setBase( const QColor& col )
    {
        QPalette mpal( palette() );
        mpal.setColor( QPalette::Base, col );
        setPalette( mpal );
        fill = true;
    }

    void clearBase() {
        fill = false;
    }

    void addToken( int n )
    {
        if( tokenCount < 6 )
        {
            token[ tokenCount ] = n;
            tokenDur[ tokenCount ] = 255;
            ++tokenCount;
            update();
        }
    }

    void removeToken( int index )
    {
        --tokenCount;
        for( int i = index; i < tokenCount; ++i )
        {
            token[i] = token[i+1];
            tokenDur[i] = tokenDur[i+1];
        }
        update();
    }

    short ctype;
    bool  fill;
    uint8_t token[6];
    uint8_t tokenDur[6];
    uint8_t tokenCount;

    static std::vector<QPixmap*> tokenPixmap;

protected:
    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        QFontMetrics fm( fontMetrics() );
        QColor pcol = palette().color( QPalette::Text );
        QBrush br;
        int h = height();

        if( fill )
        {
            br.setStyle( Qt::SolidPattern );
            br.setColor( palette().color( QPalette::Base ) );
        }

        p.setPen( pcol );
        p.setBrush( br );
        p.drawRect( 0, 0, width()-1, h-1 );
#ifdef CL_CENTER
        int pad = (h - fm.height()) / 2;
        p.drawText( 4, h - fm.descent() - pad, text() );
#else
        p.drawText( 4, fm.ascent() + 3, text() );
#endif

        int tokX = width() - tokenCount*18;
        int tokY = h - 18;
        for( int i = 0; i < tokenCount; ++i, tokX += 18 )
        {
            p.drawPixmap( tokX, tokY, *tokenPixmap[ token[i] ] );
        }
    }
};


std::vector<QPixmap*> ColorLabel::tokenPixmap;


//----------------------------------------------------------------------------


class TokenMenu : public QMenu
{
public:
    TokenMenu( const QString& title, QWidget* parent )
        : QMenu(title, parent)
    {
        _wact = new QWidgetAction(this);
        PixmapChooser* pmc = new PixmapChooser;
        _wact->setDefaultWidget( pmc );
        addAction( _wact );
    }

    PixmapChooser* chooser() {
        return static_cast<PixmapChooser*>( _wact->defaultWidget() );
    }

private:
    QWidgetAction* _wact;
};


//----------------------------------------------------------------------------


#define SUBJECT_NONE    -1
#define SUBJECT_WIDTH   132


static int _subjectHeight( const ColorLabel* cl )
{
    int fh = cl->fontMetrics().height();
    if( cl->tokenCount )
        fh *= 2;
    return fh + 6;
}


Timeline::Timeline( const ActionTable* at, QWidget* parent )
    : QWidget(parent), _actions(at)
{
    int leftMargin = 0;

    _pixPerSec = 70;
    _startTime = 0;
    _turnDur = 6;
    _subject = SUBJECT_NONE;

    setAcceptDrops(true);
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    _lo = new QVBoxLayout(this);
    _lo->setSpacing(0);
    _lo->setContentsMargins( leftMargin, 11, 0, 0 );


    QAction* act = new QAction( this );
    act->setShortcut( QKeySequence::Delete );
    connect( act, SIGNAL(triggered(bool)), this, SLOT(deleteLastAction()) );
    addAction( act );

    makeTimeScale( _pixPerSec );
    _scale = new QLabel( this );
    _scale->setFrameShape( QFrame::NoFrame );    // Does nothing?
    _scale->setAlignment( Qt::AlignTop );
    _scale->move( leftMargin + SUBJECT_WIDTH, 0 );
    _scale->setFixedWidth( _pixPerSec * _turnDur );
    _scale->setPixmap( _timeScale );

    _tokenMenu = new TokenMenu("Add Token", this);
    PixmapChooser* pmc = _tokenMenu->chooser();
    pmc->setPixmaps( ColorLabel::tokenPixmap );
    connect( pmc, SIGNAL(selected(int)), SLOT(recordToken(int)) );
}


void Timeline::prepareTokenMenu( QMenu* menu )
{
    _tokenItem = -1;
    _tokenMenu->chooser()->deselect();
    _tokenMenuTop = menu;
    menu->addMenu( _tokenMenu );
}


void Timeline::recordToken( int n )
{
    //printf( "KR record %d\n", n );
    _tokenItem = n;

    // NOTE: Must manually close menu when using QWidgetAction.
    // See QTBUG-10427 which may be fixed in Qt 5.15.

    //_tokenAct->trigger();     // This does nothing.
    _tokenMenuTop->hide();      // Causes exec to exit with NULL QAction.
}


void Timeline::recordTokenRem( int n )
{
    _tokenRemoved = n;
    _tokenMenuTop->hide();
}


void Timeline::makeTimeScale( int pixPerSec )
{
    QImage img( pixPerSec * 10, 10, QImage::Format_RGB888 );
    img.fill( Qt::black );

    QPainter p( &img );
    p.setBrush( Qt::white );
    int x = 0;
    for( int i = 0; i < 5; ++i, x += pixPerSec * 2 )
        p.drawRect( x, 0, pixPerSec-1, img.height() );
    _timeScale.convertFromImage( img );
}


void Timeline::select( int index )
{
    if( _subject != index )
    {
        ColorLabel* cl;
        if( (cl = selectedNameLabel()) )
        {
            cl->setColor( Qt::black );
            cl->clearBase();
        }

        _subject = index;

        if( (cl = selectedNameLabel()) )
        {
            cl->setColor( Qt::white );
            cl->setBase( QColor(RGB_SELECT) );
        }
    }
}


/*
  Return layout of selected subject row or NULL if none is selected.
*/
QBoxLayout* Timeline::selectedLayout()
{
    if( hasSelection() )
    {
        QBoxLayout* slo;
        QLayoutItem* item = _lo->itemAt( _subject );
        if( item && (slo = static_cast<QBoxLayout*>(item->layout())) )
            return slo;
    }
    return NULL;
}


ColorLabel* Timeline::selectedNameLabel()
{
    QLayout* slo = selectedLayout();
    if( slo )
    {
        ColorLabel* cl;
        QLayoutItem* item = slo->itemAt( 0 );
        if( item && (cl = static_cast<ColorLabel*>(item->widget())) )
            return cl;
    }
    return NULL;
}


void Timeline::saveImage()
{
#ifndef _WIN32
    QString fn( "/tmp/action-%1-%2sec.jpeg" );
    QImage img( size(), QImage::Format_RGB888 );
    render( &img );
    img.save( fn.arg( QCoreApplication::applicationPid() ).arg( _startTime ) );
#endif
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
            sc = slo->count() - 1;
            for( int ai = 1; ai < sc; ++ai )
            {
                item = slo->itemAt(ai);
                w = item->geometry().width();
                if( (wid = item->widget()) )
                {
                    if( rem < w )
                        wid->setFixedWidth( w - rem );
                    else
                        wid->deleteLater();
                }
                rem -= w;
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


void Timeline::setTurnDuration( int sec )
{
    _turnDur = sec;
    _scale->setFixedWidth( _pixPerSec * _turnDur );
}


void Timeline::addSubject( const QString& name, bool sel )
{
#if 1
    ColorLabel* cl = new ColorLabel(name);
    cl->ctype = CTYPE_NAME;
    cl->setFixedSize( SUBJECT_WIDTH, _subjectHeight(cl) );
    cl->setColor( Qt::black );

    QBoxLayout* slo = new QHBoxLayout;
    slo->addWidget( cl );
    slo->addStretch();
    _lo->addLayout( slo );
#else
    _lo->addWidget( new QLabel(name) );
#endif

    if( sel )
        select( _lo->count() - 1 );
}


int Timeline::subjectCount() const
{
    return _lo->count();
}


bool Timeline::appendAction( int id )
{
    QBoxLayout* slo = selectedLayout();
    if( slo )
    {
        ColorLabel* cl = new ColorLabel( _actions->name(id) );
        cl->ctype = CTYPE_ACTION;
        cl->setFixedSize( _pixPerSec * _actions->duration(id),
                          _subjectHeight(cl) );
        cl->setColor( Qt::darkGray );

        slo->insertWidget( slo->count() - 1, cl );
        return true;
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

    int id = _actions->actionId(CSTR(name));
    if( id >= 0 && appendAction( id ) )
        ev->acceptProposedAction();
}


void Timeline::contextMenuEvent(QContextMenuEvent* ev)
{
    QWidget* wid = childAt( ev->pos() );
    if( wid )
    {
        QMenu menu;
        QAction* resolv = NULL;
        QAction* done   = NULL;
        QAction* resize = NULL;
        QAction* rename;
        QAction* act;
        ColorLabel* cl = static_cast<ColorLabel*>( wid );

        if( cl->ctype == CTYPE_ACTION )
        {
            resolv = menu.addAction( "Resolve" );
            done   = menu.addAction( "Mark Done" );
            resize = menu.addAction( "Set Duration" );
        }
        else
        {
            prepareTokenMenu( &menu );

            _tokenRemoved = -1;
            if( cl->tokenCount )
            {
                TokenMenu* rtok = new TokenMenu("Remove Token", &menu);
                PixmapChooser* pmc = rtok->chooser();
                pmc->setColumns( cl->tokenCount );
                for( int i = 0; i < cl->tokenCount; ++i )
                    pmc->addPixmap( ColorLabel::tokenPixmap[ cl->token[i] ] );
                connect(pmc, SIGNAL(selected(int)), SLOT(recordTokenRem(int)));
                menu.addMenu( rtok );
            }
        }
        rename = menu.addAction( "Rename" );
        menu.addSeparator();
        menu.addAction( "Delete" );

        act = menu.exec( ev->globalPos() );
        //printf( "KR act %p\n", act );
        if( act )
        {
            if( act == resolv )
            {
                emit resolve(cl);
            }
            else if( act == done )
            {
                QString text( cl->text() );
                text.append( ' ' );
                text.append( QChar(0x2713) );
                cl->setText( text );
                cl->setBase( QColor(RGB_RESOLVE) );
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
                        double(cl->width()) / double(_pixPerSec), 0.1, 10.0,
                        1, &ok );
                if( ok )
                    cl->setFixedWidth( int(dur * _pixPerSec) );
            }
            else    // delete
            {
                if( cl->ctype == CTYPE_ACTION )
                    wid->deleteLater();
                else
                    deleteSubject( subjectAt( ev->pos() ) );
            }
        }
        else if( cl->ctype == CTYPE_NAME )
        {
            if( _tokenItem >= 0 )
            {
                cl->addToken( _tokenItem );
                cl->setFixedHeight( _subjectHeight(cl) );
            }
            else if( _tokenRemoved >= 0 )
            {
                cl->removeToken( _tokenRemoved );
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


void Timeline::deleteSubject( int i )
{
    QLayoutItem* item = _lo->takeAt(i);
    if( item )
    {
        QWidget* wid;
        QLayout* slo = item->layout();
        int sc = slo->count() - 1;
        for( int ai = 0; ai < sc; ++ai )
        {
            item = slo->itemAt(ai);
            if( item && (wid = item->widget()) )
                delete wid;
        }
        delete item;
    }
}


/*
  Return the last action of the selected subject or NULL if there is none.
*/
ColorLabel* Timeline::lastAction()
{
    QBoxLayout* slo = selectedLayout();
    if( slo && slo->count() > 2 )
    {
        QWidget* wid;
        QLayoutItem* item = slo->itemAt( slo->count() - 2 );
        if( item && (wid = item->widget()) )
            return static_cast<ColorLabel*>( wid );
    }
    return NULL;
}


void Timeline::deleteLastAction()
{
    ColorLabel* cl = lastAction();
    if( cl )
        cl->deleteLater();
}


void Timeline::mousePressEvent(QMouseEvent* ev)
{
    int n = subjectAt( ev->pos() );
    if( n != SUBJECT_NONE )
        select(n);
}


void Timeline::orderSubject( int dir )
{
    int count = _lo->count();
    if( count > 1 && hasSelection() )
    {
        int n;
        if( dir < 0 )
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

        // With Qt 5.12.5 takeAt() sets the item layout parent to 0, but
        // insertItem() below does not set it again.  If the parent is not set
        // again here then no further widgets can be added to the subject
        // layout and the item->geometry() will be 0,0,0,0.

        QLayout* slo = item->layout();
        if( ! slo->parent() )
            slo->setParent( _lo );

        _lo->insertItem( n, item );
        _subject = n;   // No need to call select().
    }
}


void Timeline::wheelEvent(QWheelEvent* ev)
{
    if( ev->modifiers() & Qt::ShiftModifier )
    {
        orderSubject( (ev->angleDelta().y() > 0) ? -1 : 1 );
    }
    else
    {
        int n = 0;
        int count = _lo->count();
        if( count && hasSelection() )
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


struct InitialAction
{
    const char* name;
    int dur;
};

#define ACT_COUNT   14
static const InitialAction _initAction[ ACT_COUNT ] =
{
    { "Walk 10", 3 },
    { "Run 10",  1 },
    { "Attack",  5 },
    { "Defend",  5 },
    { "Shoot",   5 },
    { "Aimed Shot", 7 },
    { "Quick Shot", 4 },
    { "Drink",   5 },
    { "Draw",    1 },
    { "Equip",   6 },
    { "Pickup",  3 },
    { "Throw",   3 },
    { "Wait 1",  1 },
    { "Wait 2",  2 }
};


ActionTimeline::ActionTimeline( QWidget* parent ) : QWidget(parent)
{
    setWindowTitle( "Action Timeline" );

    _tl = new Timeline( &_at );
    connect( _tl, SIGNAL(resolve(ColorLabel*)), SLOT(rollDice(ColorLabel*)) );

    _actList = new QListWidget;
    _actList->setDragEnabled(true);
    _actList->setMaximumWidth( 180 );
    _actList->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );
    connect( _actList, SIGNAL(itemActivated(QListWidgetItem*)),
             SLOT(appendAction(QListWidgetItem*)) );

    QPushButton* add = new QPushButton;
    add->setIcon( QIcon(":/icon/new_pc-32.png") );
    connect( add, SIGNAL(clicked(bool)), this, SLOT(newSubject()) );

    QPushButton* up = new QPushButton;
    connect( up, SIGNAL(clicked(bool)), this, SLOT(subjectUp()) );

    QPushButton* down = new QPushButton;
    connect( down, SIGNAL(clicked(bool)), this, SLOT(subjectDown()) );

    _turn = new QComboBox;
    _turn->addItem( "6 sec" );
    _turn->addItem( "10 sec" );
    connect( _turn, SIGNAL(currentIndexChanged(int)),
             SLOT(turnDurationChanged(int)) );

    QPushButton* adv = new QPushButton;
    connect( adv, SIGNAL(clicked(bool)), this, SLOT(advance()) );

    _time = new QLineEdit;
    _time->setFixedSize( 60, _turn->sizeHint().height() );
    _time->setValidator( new QIntValidator(0, 999, this) );
    connect( _time, SIGNAL(textEdited(const QString&)),
             this, SLOT(timeEdited()) );

    QPushButton* roll = new QPushButton;
    {
    QIcon icon;
    icon.addFile( ":/icon/d20-32.png", QSize(32,32) );
    icon.addFile( ":/icon/d20-24.png", QSize(24,24) );
    icon.addFile( ":/icon/d20-16.png", QSize(16,16) );
    roll->setIcon( icon );
    }
    connect( roll, SIGNAL(clicked(bool)), SLOT(rollDiceLast()) );

    _dice = new QComboBox;
    _dice->setEditable( true );
    _dice->setMinimumWidth( 120 );
    _dice->addItem( "d20" );
    _dice->addItem( "d20+d3" );
    _dice->addItem( "3d6" );

    QPushButton* about = new QPushButton( "?" );
    about->setFixedWidth( roll->sizeHint().width() );
    connect( about, SIGNAL(clicked(bool)), SLOT(showAbout()) );

    QStyle* st = QApplication::style();
    up->setIcon  ( st->standardIcon(QStyle::SP_ArrowUp) );
    down->setIcon( st->standardIcon(QStyle::SP_ArrowDown) );
    adv->setIcon ( st->standardIcon(QStyle::SP_MediaPlay) );

    QBoxLayout* lo = new QHBoxLayout;
    lo->addWidget( add );
    lo->addWidget( up );
    lo->addWidget( down );
    lo->addSpacing( 32 );
    lo->addWidget( _turn );
    lo->addWidget( adv );
    lo->addWidget( _time );
    lo->addSpacing( 32 );
    lo->addWidget( roll );
    lo->addWidget( _dice );
    lo->addStretch();
    lo->addWidget( about );

    QGridLayout* grid = new QGridLayout(this);
    grid->addWidget( _tl,      0, 0 );
    grid->addWidget( _actList, 0, 1, 2, 2 );
    grid->addLayout( lo,       1, 0 );

    addQAction( QKeySequence(Qt::Key_F5),         SLOT(rollDiceLast()) );
    addQAction( QKeySequence(Qt::CTRL+Qt::Key_T), SLOT(advance()) );
    addQAction( QKeySequence::HelpContents,       SLOT(showAbout()) );
    addQAction( QKeySequence::Quit,               SLOT(close()) );

    // Built-in character actions.
    for( int i = 0; i < ACT_COUNT; ++i )
    {
        const char* name = _initAction[i].name;
        _at.defineAction( name, name + strlen(name), _initAction[i].dur );
        new QListWidgetItem( QString(name), _actList, i );
    }

    showTime( 0 );
}


void ActionTimeline::addQAction( const QKeySequence& key, const char* slot )
{
    QAction* act = new QAction( this );
    act->setShortcut( key );
    connect( act, SIGNAL(triggered(bool)), slot );
    addAction( act );
}


void ActionTimeline::parseArgs( int argc, char** argv )
{
    std::vector<char> nameBuf;
    char* cp;
    bool select = true;

    for( int i = 0; i < argc; ++i )
    {
        if( (cp = strchr(argv[i], ':')) )
        {
            nameBuf.assign( argv[i], cp );
            nameBuf.push_back( '\0' );

            int dur = atoi(cp+1);
            if( dur < 1 )
                dur = 1;
            else if( dur > 10 )
                dur = 10;

            int id = _at.actionId( nameBuf.data() );
            if( id < 0 )
            {
                id = _at.defineAction( argv[i], cp, dur );
                new QListWidgetItem( QString(_at.name(id)), _actList, id );
            }
            else
            {
                // Override duration of existing action.
                _at.setDuration( id, dur );
            }
        }
        else
        {
            _tl->addSubject( argv[i], select );
            select = false;
        }
    }
}


void ActionTimeline::newSubject()
{
    _tl->addSubject( "<unnamed>" );
    // TODO: Open Rename dialog.
}


void ActionTimeline::subjectUp()
{
    _tl->orderSubject( -1 );
}


void ActionTimeline::subjectDown()
{
    _tl->orderSubject( 1 );
}


void ActionTimeline::turnDurationChanged(int index)
{
    _tl->setTurnDuration( index ? 10 : 6 );
    showTime( _tl->startTime(), false );
}


void ActionTimeline::appendAction(QListWidgetItem* item)
{
    _tl->appendAction( item->type() );
}


void ActionTimeline::showTime( int sec, bool setEditField )
{
    QString str;
    int turnDur = _turn->currentIndex() ? 10 : 6;

    str.sprintf( "Turn %d, Time %02d:%02d",
                 sec / turnDur + 1, sec / 60, sec % 60 );

    if( setEditField )
        _time->setText( QString::number( sec ) );
    _time->setToolTip( str );
}


void ActionTimeline::advance()
{
    int turnDur = _turn->currentIndex() ? 10 : 6;

    _tl->saveImage();
    _tl->advance( turnDur );

    showTime( _tl->startTime() );
}


void ActionTimeline::timeEdited()
{
    int sec = _time->text().toInt();
    _tl->setStartTime( sec );
    showTime( sec, false );
}


#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#define DICE_ROLL(n)    (qrand() % n + 1)
#else
#include <QRandomGenerator>
#define DICE_ROLL(n)    (QRandomGenerator::global()->bounded(n) + 1)
#endif

#include "evalDice.c"


static void _emit(void* user, int n)
{
    static_cast< QVector<int>* >(user)->push_back( n );
}


void ActionTimeline::rollDice( ColorLabel* cl )
{
    if( cl )
    {
        QVector<int> buf;
        int len, n;
        int total = evalDice( CSTR(_dice->currentText()), _emit, &buf );

        QString str( cl->text() );
        if( (len = buf.size()) > 1 )
        {
            str.append( " (" );
            for( int i = 0; i < len; ++i )
            {
                n = buf[i];
                if( i && n >= 0 )
                    str.append( '+' );
                str.append( QString::number( n ) );
            }
            str.append( ')' );
        }
        str.append( ' ' );
        str.append( QString::number(total) );

        cl->setText( str );
        cl->setBase( QColor(RGB_RESOLVE) );
    }
}


void ActionTimeline::rollDiceLast()
{
    rollDice( _tl->lastAction() );
}


void ActionTimeline::showAbout()
{
    QString str(
        "<h2>Action Timeline 0.7</h2>\n"
        "%1, &copy; 2020 Karl Robillard\n"
        "<p>Icons from game-icons.net are &copy; by"
        " Carl Olsen, Delapouite, Lorc, sbed & Viscious Speed"
        " and used under the CC BY 3.0 license.</p>\n"
        "<h4>Key Commands</h4>\n"
        "<table>\n"
        "<tr><td width=\"64\">Del</td><td>Delete last action</td>"
        "<tr><td>F5</td> <td>Resolve last action</td>"
        "<tr><td>CTRL+T</td> <td>Advance to next turn</td>"
        "</table>\n"
    );

    QMessageBox* about = new QMessageBox(this);
    about->setWindowTitle( "About Action Timeline" );
    about->setIconPixmap( QPixmap(":/icon/app-32.png") );
    about->setTextFormat( Qt::RichText );
    about->setText( str.arg( __DATE__ ) );
    about->show();
}


#define TOKEN_COUNT 20
static const char* tokenFile[ TOKEN_COUNT ] =
{
    ":/icon/abstract-097.png",
    ":/icon/awareness.png",
    ":/icon/batwing-emblem.png",
    ":/icon/bleeding-wound.png",
    ":/icon/cobweb.png",
    ":/icon/crown-of-thorns.png",
    ":/icon/dead-eye.png",
    ":/icon/dripping-blade.png",
    ":/icon/drop.png",
    ":/icon/flame.png",
    ":/icon/icicles-aura.png",
    ":/icon/key.png",
    ":/icon/octogonal-eye.png",
    ":/icon/primitive-torch.png",
    ":/icon/psychic-waves.png",
    ":/icon/rough-wound.png",
    ":/icon/skull-crossed-bones.png",
    ":/icon/spikes.png",
    ":/icon/sprint.png",
    ":/icon/terror.png"
};


int main( int argc, char** argv )
{
    QApplication app( argc, argv );

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    qsrand( time(NULL) );
#endif

    for( int i = 0; i < TOKEN_COUNT; ++i )
        ColorLabel::tokenPixmap.push_back( new QPixmap( tokenFile[i] ) );

    QIcon icon;
    icon.addFile( ":/icon/app-32.png", QSize(32,32) );
    icon.addFile( ":/icon/app-24.png", QSize(24,24) );
    icon.addFile( ":/icon/app-16.png", QSize(16,16) );
    app.setWindowIcon( icon );

    ActionTimeline win;
    win.resize( 980, 350 );
    win.show();
    if( argc > 1 )
        win.parseArgs( argc-1, argv+1 );
    if( ! win.subjectCount() )
        win.newSubject();
    return app.exec();
}
