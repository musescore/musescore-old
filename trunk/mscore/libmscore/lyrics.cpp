//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "lyrics.h"
#include "xml.h"
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
      setTextStyle((_no % 2) ? TEXT_STYLE_LYRIC2 : TEXT_STYLE_LYRIC1);
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

void Lyrics::scanElements(void* data, void (*func)(void*, Element*), bool)
      {
      if (_verseNumber)
            func(data, _verseNumber);
      func(data, this);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(Xml& xml) const
      {
      xml.stag("Lyrics");
      if (_no)
            xml.tag("no", _no);
      if (_syllabic != SINGLE) {
            static const char* sl[] = {
                  "single", "begin", "end", "middle"
                  };
            xml.tag("syllabic", sl[_syllabic]);
            }
      if (_ticks)
            xml.tag("ticks", _ticks);
      Text::writeProperties(xml);
      if (_verseNumber)
            _verseNumber->write(xml, "Number");
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(QDomElement e)
      {
      int   iEndTick = 0;           // used for backward compatibility

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int i = val.toInt();
            if (tag == "no")
                  _no = i;
            else if (tag == "syllabic") {
                  if (val == "single")
                        _syllabic = SINGLE;
                  else if (val == "begin")
                        _syllabic = BEGIN;
                  else if (val == "end")
                        _syllabic = END;
                  else if (val == "middle")
                        _syllabic = MIDDLE;
                  else
                        qDebug("bad syllabic property\n");
                  }
            else if (tag == "endTick") {          // obsolete
                  // store <endTick> tag value until a <ticks> tag has been read
                  // which positions this lyrics element in the score
                  iEndTick = i;
                  }
            else if (tag == "ticks")
                  _ticks = i;
            else if (tag == "Number") {
                  _verseNumber = new Text(score());
                  _verseNumber->read(e);
                  _verseNumber->setParent(this);
                  }
            else if (!Text::readProperties(e))
                  domError(e);
            }
      // if any endTick, make it relative to current tick
      if(iEndTick) {
            _ticks = iEndTick - score()->curTick;
            qDebug("Lyrics::endTick: %d  ticks %d\n", iEndTick, _ticks);
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
            qDebug("Lyrics::add: unknown element %s\n", el->name());
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
            qDebug("Lyrics::remove: unknown element %s\n", el->name());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lyrics::draw(Painter* painter) const
      {
      Text::draw(painter);
      foreach(const Line* l, _separator) {
            painter->translate(l->pos());
            l->draw(painter);
            painter->translate(-(l->pos()));
            }
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Lyrics::pagePos() const
      {
      System* system = measure()->system();
      qreal yp = y();
      if (system)
	      yp = yp + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Lyrics::layout()
      {
      if (!styled())
            _textStyle = (_no % 2) ? TEXT_STYLE_LYRIC2 : TEXT_STYLE_LYRIC1;
      Text::layout();
      qreal lh             = lineSpacing() * score()->styleD(ST_lyricsLineHeight);
      qreal noteHeadWidth2 = symbols[score()->symIdx()][quartheadSym].width(magS()) * .5;

      System* sys = measure()->system();
      if (sys == 0) {
            qDebug("lyrics layout: no system!\n");
            abort();
            }
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
//   paste
//---------------------------------------------------------

void Lyrics::paste()
      {
#if defined(Q_WS_MAC) || defined(__MINGW32__)
      QClipboard::Mode mode = QClipboard::Clipboard;
#else
      QClipboard::Mode mode = QClipboard::Selection;
#endif
      QString txt = QApplication::clipboard()->text(mode);
      QStringList sl = txt.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      if (sl.isEmpty())
            return;
      cursor->insertText(sl[0]);
      layout();
      bool lo = (subtype() == TEXT_INSTRUMENT_SHORT) || (subtype() == TEXT_INSTRUMENT_LONG);
      score()->setLayoutAll(lo);
      score()->setUpdateAll();
      score()->end();
      sl.removeFirst();
      txt = sl.join(" ");
      QApplication::clipboard()->setText(txt, mode);
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

int Lyrics::endTick() const
      {
      return segment()->tick() + ticks();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Lyrics::acceptDrop(MuseScoreView*, const QPointF&, int type, int subtype) const
      {
      return (type == TEXT && subtype == TEXT_LYRICS_VERSE_NUMBER);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Lyrics::drop(const DropData& data)
      {
      Element* e = data.element;
      if (!(e->type() == TEXT && e->subtype() == TEXT_LYRICS_VERSE_NUMBER))
            return 0;
      e->setParent(this);
      score()->select(e, SELECT_SINGLE, 0);
      score()->undoAddElement(e);
      return e;
      }


