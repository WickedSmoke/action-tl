#ifndef TIMELINE_H
#define TIMELINE_H
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

#include <vector>
#include <QWidget>
#include <QPixmap>

class ActionTable
{
public:
    int defineAction( const char* aname, const char* end, int dur );
    int actionId( const char* str ) const;
    const char* name( int id ) const
    {
        return _strings.data() + _entry[ id*2 ];
    }
    int duration( int id ) const
    {
        return _entry[ id*2 + 1 ];
    }
    void setDuration( int id, int dur )
    {
        _entry[ id*2 + 1 ] = dur;
    }

private:
    std::vector<char> _strings;
    std::vector<int> _entry;        // Pairs of _strings index & duration.
};

class QBoxLayout;
class QLabel;
class QMenu;
class QWidgetAction;
class ColorLabel;
class TokenMenu;

class Timeline : public QWidget
{
    Q_OBJECT
public:
    Timeline( const ActionTable*, QWidget* parent = NULL );
    void addSubject( const QString& name, bool sel = true );
    int  subjectCount() const;
    void orderSubject( int dir );
    bool hasSelection() const { return _subject >= 0; }
    void select(int);
    bool appendAction(int);
    void saveImage();
    void advance( int sec );
    int  startTime() const { return _startTime; }
    void setStartTime( int sec );
    void setTurnDuration( int sec );
    ColorLabel* lastAction();
signals:
    void resolve(ColorLabel*);
public slots:
    void deleteSubject(int);
    void deleteLastAction();
protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void mousePressEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    int  subjectAt(const QPoint& pnt) const;
private slots:
    void recordToken(int);
    void recordTokenRem(int);
private:
    void prepareTokenMenu(QMenu*);
    QBoxLayout* selectedLayout();
    ColorLabel* selectedNameLabel();
    void makeTimeScale(int);
    Timeline(const Timeline&);

    const ActionTable* _actions;
    QPixmap _timeScale;
    TokenMenu* _tokenMenu;
    QMenu* _tokenMenuTop;
    QLabel* _scale;
    QBoxLayout* _lo;
    int _pixPerSec;     // Pixels per second scale.
    int _startTime;     // Time at left side of timeline.
    int _turnDur;
    int _subject;       // Selected subject index.
    int _tokenItem;     // Selected _tokenMenu index.
    int _tokenRemoved;
};

class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class ActionTimeline : public QWidget
{
    Q_OBJECT
public:
    ActionTimeline( QWidget* parent = NULL );
    void parseArgs( int argc, char** argv );
    int  subjectCount() const { return _tl->subjectCount(); }
public slots:
    void newSubject();
    void subjectUp();
    void subjectDown();
    void turnDurationChanged(int);
    void appendAction(QListWidgetItem*);
    void advance();
    void timeEdited();
    void rollDice(ColorLabel*);
    void rollDiceLast();
    void showAbout();
private:
    void addQAction(const QKeySequence&, const char*);
    void showTime(int sec, bool setEditField = true);
    ActionTimeline(const Timeline&);

    ActionTable _at;
    Timeline* _tl;
    QListWidget* _actList;
    QComboBox* _turn;
    QLineEdit* _time;
    QComboBox* _dice;
};

#endif //TIMELINE_H
