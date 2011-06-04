//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: lyrics.cpp 3708 2010-11-16 09:54:31Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "lyrics.h"
#include "al/xml.h"
#include "system.h"
#include "measure.h"
#include "score.h"
#include "sym.h"
#include "segment.h"
#include "painter.h"

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setTextStyle(TEXT_STYLE_LYRIC1);
      _no          = 0;
      _ticks       = 0;
      _syllabic    = SINGLE;
      _verseNumber = 0;
      }

Lyrics::Lyrics(const Lyrics& l)
   : Text(l)
      {
      _no  = l._no;
      _ticks = l._ticks;
      _syllabic = l._syllabic;
      if (l._verseNumber)
            _verseNumber = new Text(*l._verseNumber);
      else
            _verseNumber = 0;
      QList<Line*> _separator;
      foreach(Line* l, l._separator)
            _separator.append(new Line(*l));
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::~Lyrics()
      {
      delete _verseNumber;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Lyrics::scanElements(void* data, void (*func)(void*, Element*))
      {
      if (_verseNumber)
            func(data, _verseNumber);
      func(data, this);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(XmlReader* r)
      {
      while (r->readElement()) {
            QString val;
            if (r->readInt("no", &_no))
                  ;
            else if (r->readString("syllabic", &val)) {
                  if (val == "single")
                        _syllabic = SINGLE;
                  else if (val == "begin")
                        _syllabic = BEGIN;
                  else if (val == "end")
                        _syllabic = END;
                  else if (val == "middle")
                        _syllabic = MIDDLE;
                  else
                        printf("bad syllabic property\n");
                  }
            else if (r->readInt("ticks", &_ticks))
                  ;
            else if (r->tag() == "Number") {
                  _verseNumber = new Text(score());
                  _verseNumber->read(r);
                  _verseNumber->setParent(this);
                  }
            else if (!Text::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(Element* el)
      {
      el->setParent(this);
      if (el->type() == LINE)
            _separator.append((Line*)el);
      else if (el->type() == TEXT && el->subtype() == TEXT_LYRICS_VERSE_NUMBER)
            _verseNumber = static_cast<Text*>(el);
      else
            printf("Lyrics::add: unknown element %s\n", el->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Lyrics::remove(Element* el)
      {
      if (el->type() == LINE)
            _separator.removeAll((Line*)el);
      else if (el->type() == TEXT && el->subtype() == TEXT_LYRICS_VERSE_NUMBER)
            _verseNumber = 0;
      else
            printf("Lyrics::remove: unknown element %s\n", el->name());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lyrics::draw(Painter* p) const
      {
      Text::draw(p);
      foreach(const Line* l, _separator) {
            p->translate(l->pos());
            l->draw(p);
            p->translate(-l->pos());
            }
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Lyrics::canvasPos() const
      {
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      qreal yp = y();
	  if(system)
	      yp = yp + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Lyrics::layout()
      {
      if (!styled())
            _textStyle = (_no % 2) ? TEXT_STYLE_LYRIC2 : TEXT_STYLE_LYRIC1;
      Text::layout();
      qreal lh             = lineSpacing();
      qreal noteHeadWidth2 = symbols[score()->symIdx()][quartheadSym].width(magS()) * .5;

      Segment* seg   = segment();
      System* sys    = seg->measure()->system();
      const QList<Lyrics*>* ll = &(chordRest()->lyricsList());

      int line       = ll->indexOf(this);
      qreal y       = lh * line + point(score()->styleS(ST_lyricsDistance))
                       + sys->staff(staffIdx())->bbox().height();
      qreal x;
      //
      // left align if syllable has a number
      //
      if (_ticks == 0 && (align() & ALIGN_HCENTER) && !_verseNumber)
            x = noteHeadWidth2 - bbox().width() * .5;
      else
            x = 0.0;
      setPos(x, y);
      if (_verseNumber)
            _verseNumber->layout();
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

int Lyrics::endTick() const
      {
      return segment()->tick() + ticks();
      }

