//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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
#include "xml.h"
#include "system.h"
#include "measure.h"
#include "score.h"
#include "canvas.h"
#include "sym.h"

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_LYRIC);
      _no        = 0;
      _syllabic  = SINGLE;
      _endTick   = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(Xml& xml) const
      {
      xml.stag("Lyrics");
      xml.tag("data", getText());
      if (_no)
            xml.tag("no", _no);
      static const char* sl[] = {
            "single", "begin", "end", "middle"
            };
      if (_syllabic != SINGLE)
            xml.tag("syllabic", sl[_syllabic]);
      if (_endTick)
            xml.tag("endTick", _endTick);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "data")
                  setText(val);
            else if (tag == "no")
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
                        printf("bad syllabic property\n");
                  }
            else if (tag == "endTick")
                  _endTick = i;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Lyrics::add(Element* el)
      {
      if (el->type() == LINE)
            _separator.append((Line*)el);
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
      else
            printf("Lyrics::remove: unknown element %s\n", el->name());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lyrics::draw(QPainter& p) const
      {
      Text::draw(p);
      foreach(const Line* l, _separator) {
            QPointF pt(l->pos());
            p.translate(pt);
            l->draw(p);
            p.translate(-pt);
            }
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Lyrics::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      if (debugMode && system == 0) {
            printf("Lyrics::canvasPos(): no system (%s)\n", qPrintable(getText()));
            return pos();
            }
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   lyricsTab
//---------------------------------------------------------

void Score::lyricsTab(bool /*back*/)
      {
      start();

      Lyrics* lyrics   = (Lyrics*)editObject;
      int track        = lyrics->track();
      int staffIdx     = lyrics->staffIdx();
      Segment* segment = (Segment*)(lyrics->parent());
      int verse        = lyrics->no();

      canvas()->setState(Canvas::NORMAL); // this can remove lyrics if empty
      endCmd();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1())) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == CHORD)
                  break;
            }
      if (nextSegment == 0) {
            return;
            }

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            LyricsList* nll = segment->lyricsList(staffIdx);
            if (!nll)
                  continue;
            oldLyrics = nll->value(verse);
            if (oldLyrics)
                  break;
            segment = segment->prev1();
            }

      startCmd();

      LyricsList* ll = nextSegment->lyricsList(staffIdx);
      lyrics         = ll->value(verse);
      if (!lyrics) {
            lyrics = new Lyrics(this);
            lyrics->setTick(nextSegment->tick());
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment);
            lyrics->setNo(verse);
            }

      lyrics->setSyllabic(Lyrics::SINGLE);

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::SINGLE:
                  case Lyrics::END:
                        break;
                  case Lyrics::BEGIN:
//                        oldLyrics->setEndTick(endTick);
                        oldLyrics->setSyllabic(Lyrics::END);
                        break;
                  case Lyrics::MIDDLE:
//                        if (oldLyrics->tick() < endTick) {
//                              oldLyrics->setEndTick(endTick);
//                              }
                        oldLyrics->setSyllabic(Lyrics::END);
                        break;
                  }
            }

      lyrics->setSyllabic(Lyrics::SINGLE);
      undoAddElement(lyrics);

      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);
      ((Lyrics*)editObject)->moveCursorToEnd();

      layoutAll = true;
      }

//---------------------------------------------------------
//   lyricsMinus
//---------------------------------------------------------

void Score::lyricsMinus()
      {
      start();

      Lyrics* lyrics   = (Lyrics*)editObject;
      int track        = lyrics->track();
      int staffIdx     = lyrics->staffIdx();
      Segment* segment = (Segment*)(lyrics->parent());
      int verse        = lyrics->no();

      canvas()->setState(Canvas::NORMAL); // this can remove lyrics if empty
      endCmd();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1())) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == CHORD)
                  break;
            }
      if (nextSegment == 0) {
            return;
            }

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            LyricsList* nll = segment->lyricsList(staffIdx);
            if (!nll)
                  continue;
            oldLyrics = nll->value(verse);
            if (oldLyrics)
                  break;
            segment = segment->prev1();
            }

      startCmd();

      LyricsList* ll = nextSegment->lyricsList(staffIdx);
      lyrics         = ll->value(verse);
      if (!lyrics) {
            lyrics = new Lyrics(this);
            lyrics->setTick(nextSegment->tick());
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment);
            lyrics->setNo(verse);
            }

      lyrics->setSyllabic(Lyrics::END);

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::SINGLE:
                        oldLyrics->setSyllabic(Lyrics::BEGIN);
                        lyrics->setSyllabic(Lyrics::MIDDLE);
                        break;
                  case Lyrics::BEGIN:
                        lyrics->setSyllabic(Lyrics::MIDDLE);
                        break;
                  case Lyrics::MIDDLE:
                        break;
                  case Lyrics::END:
                        oldLyrics->setSyllabic(Lyrics::MIDDLE);
                        lyrics->setSyllabic(Lyrics::MIDDLE);
                        break;
                  }
            }
      undoAddElement(lyrics);

      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);
      ((Lyrics*)editObject)->moveCursorToEnd();

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsUnderscore
//---------------------------------------------------------

void Score::lyricsUnderscore()
      {
      start();

      Lyrics* lyrics   = (Lyrics*)editObject;
      int track        = lyrics->track();
      int staffIdx     = lyrics->staffIdx();
      Segment* segment = (Segment*)(lyrics->parent());
      int verse        = lyrics->no();
      int endTick      = lyrics->tick();

      canvas()->setState(Canvas::NORMAL); // this can remove lyrics if empty
      endCmd();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1())) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == CHORD)
                  break;
            }
      if (nextSegment == 0) {
            return;
            }

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            LyricsList* nll = segment->lyricsList(staffIdx);
            if (!nll)
                  continue;
            oldLyrics = nll->value(verse);
            if (oldLyrics)
                  break;
            segment = segment->prev1();
            }

      startCmd();

      LyricsList* ll = nextSegment->lyricsList(staffIdx);
      lyrics         = ll->value(verse);
      if (!lyrics) {
            lyrics = new Lyrics(this);
            lyrics->setTick(nextSegment->tick());
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment);
            lyrics->setNo(verse);
            }

      lyrics->setSyllabic(Lyrics::SINGLE);

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::SINGLE:
                  case Lyrics::END:
                        break;
                  default:
                        oldLyrics->setSyllabic(Lyrics::END);
                        break;
                  }
            if (oldLyrics->tick() < endTick)
                  oldLyrics->setEndTick(endTick);
            }
      undoAddElement(lyrics);

      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);
      ((Lyrics*)editObject)->moveCursorToEnd();

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsReturn
//---------------------------------------------------------

void Score::lyricsReturn()
      {
      start();
      Lyrics* lyrics   = (Lyrics*)editObject;
      Segment* segment = (Segment*)(lyrics->parent());

      canvas()->setState(Canvas::NORMAL);
      endCmd();

      startCmd();

      Lyrics* oldLyrics = lyrics;

      lyrics = new Lyrics(this);
      lyrics->setTick(segment->tick());
      lyrics->setTrack(oldLyrics->track());
      lyrics->setParent(segment);
      lyrics->setNo(oldLyrics->no() + 1);
      undoAddElement(lyrics);
      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Lyrics::layout(ScoreLayout* l)
      {
      Text::layout(l);
      double lh             = lineSpacing();
      double noteHeadWidth2 = symbols[quartheadSym].width(mag()) * .5;

      Segment* seg   = segment();
      System* sys    = seg->measure()->system();
      LyricsList* ll = seg->lyricsList(staffIdx());

      int line       = ll->indexOf(this);
      double y       = lh * line + point(score()->style()->lyricsDistance)
                       + sys->staff(staffIdx())->bbox().height();
      double x       = noteHeadWidth2 - bbox().width() * .5;
      setPos(x, y);
      }


