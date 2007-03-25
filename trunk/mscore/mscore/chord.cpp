//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chord.cpp,v 1.6 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

/**
 \file
 Implementation of classes Chord, HelpLine, NoteList and Stem.
*/

#include "chord.h"
#include "note.h"
#include "xml.h"
#include "style.h"
#include "segment.h"
#include "text.h"
#include "measure.h"
#include "system.h"
#include "tuplet.h"
#include "hook.h"
#include "layout.h"

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(QPainter& p)
      {
      qreal lw = point(::style->stemWidth);
      QPen pen(p.pen());
      pen.setWidthF(lw);
      p.setPen(pen);

      p.drawLine(QLineF(0.0, 0.0, 0.0, point(_len)));
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Stem::setLen(const Spatium& l)
      {
      _len = l;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Stem::bbox() const
      {
      double w = point(::style->stemWidth);
      double l = point(_len);
      return QRectF(-w * .5, 0, w, l);
      }

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

void Chord::init()
      {
      _up            = true;
      _stem          = 0;
      _hook          = 0;
      _stemDirection = AUTO;
      _grace         = false;
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Chord::upLine() const
      {
      return upNote()->line();
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Chord::downLine() const
      {
      return downNote()->line();
      }

Chord::Chord(Score* s)
   : ChordRest(s)
      {
      init();
      }

Chord::Chord(Score* s, int tick)
   : ChordRest(s)
      {
      init();
      setTick(tick);
      }

//---------------------------------------------------------
//   setHook
//---------------------------------------------------------

void Chord::setHook(Hook* f)
      {
      if (_hook)
            delete _hook;
      _hook = f;
      if (_hook)
            _hook->setParent(this);
      }

//---------------------------------------------------------
//   setStem
//---------------------------------------------------------

void Chord::setStem(Stem* s)
      {
      if (_stem)
            delete _stem;
      _stem = s;
      if (_stem)
            _stem->setParent(this);
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF Chord::stemPos(bool upFlag, bool top) const
      {
      Note* note = (top ? !upFlag : upFlag) ? noteList()->front() : noteList()->back();
      return note->stemPos(upFlag);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Chord::setSelected(bool f)
      {
      Element::setSelected(f);
      foreach(Note* n, notes)
            n->setSelected(f);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Chord::add(Element* e)
      {
      e->setVoice(voice());
      e->setParent(this);
      e->setStaff(staff());
      if (e->type() == NOTE) {
            notes.append((Note*)e);
            qSort(notes.begin(), notes.end(), notesPitchCompare);
            }
      else if (e->type() == ATTRIBUTE)
            attributes.push_back((NoteAttribute*)e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(Element* e)
      {
      if (e->type() == NOTE) {
            int idx = notes.indexOf((Note*)e);
            if (idx == -1)
                  printf("Chord::remove() note %p not found!\n", e);
            else {
                  notes.removeAt(idx);
                  qSort(notes.begin(), notes.end(), notesPitchCompare);
                  }
            }
      else if (e->type() == ATTRIBUTE) {
            int idx = attributes.indexOf((NoteAttribute*)e);
            if (idx == -1)
                  printf("Chord::remove(): attribute not found\n");
            else {
                  attributes.removeAt(idx);
                  }
            }
      }

//---------------------------------------------------------
//   drawPosMark
//---------------------------------------------------------

void drawPosMark(QPainter& painter, const QPointF& p)
      {
      qreal x = p.x();
      qreal y = p.y();
      painter.setPen(Qt::blue);
      painter.drawLine(QLineF(x-10, y-10, x+10, y+10));
      painter.drawLine(QLineF(x+10, y-10, x-10, y+10));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

/**
 Draw chord and calculate bounding region.
*/

void Chord::draw(QPainter& p)
      {
      foreach(Note* n, notes)
            n->draw(p);
      for (ciHelpLine l = helpLines.begin(); l != helpLines.end(); ++l)
            (*l)->draw(p);
      for (ciAttribute l = attributes.begin(); l != attributes.end(); ++l)
            (*l)->draw(p);
      if (_hook)
            _hook->draw(p);
      if (_stem)
            _stem->draw(p);
      }

//---------------------------------------------------------
//   bbox
//    CHECK: is this function needed?
//---------------------------------------------------------

QRectF Chord::bbox() const
      {
      QRectF _bbox;
      foreach(const Note* n, notes)
            _bbox |= n->bbox().translated(n->pos());
      for (ciHelpLine i = helpLines.begin(); i != helpLines.end(); ++i)
            _bbox |= (*i)->bbox().translated((*i)->pos());
      for (ciAttribute i = attributes.begin(); i != attributes.end(); ++i)
            _bbox |= (*i)->bbox().translated((*i)->pos());
      if (_hook)
            _bbox |= _hook->bbox().translated(_hook->pos());
      if (_stem)
            _bbox |= _stem->bbox().translated(_stem->pos());
      return _bbox;
      }

//---------------------------------------------------------
//   layoutStem
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

void Chord::layoutStem(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();
      System* s      = segment()->measure()->system();
      double sy      = s->staff(staffIdx())->bbox().y();
      Note* upnote   = notes.back();
      Note* downnote = notes.front();

      double uppos   = s->staff(staffIdx() + upnote->move())->bbox().y();
            uppos    = (uppos - sy)/_spatium * 2.0 + upnote->line();

      double downpos = s->staff(staffIdx() + downnote->move())->bbox().y();
            downpos  = (downpos - sy)/_spatium * 2.0 + downnote->line();

      //-----------------------------------------
      //  process stem
      //-----------------------------------------

      bool up = isUp();

      int ticks = tuplet() ? tuplet()->baseLen() : tickLen();
      int hookIdx;
      Spatium stemLen;

      if (_grace) {
            stemLen = Spatium(2.5);
            hookIdx = 1;
            }
      else {
            stemLen = Spatium((up ? uppos - 3 : 3 - downpos) / 2.0);
            if (stemLen < Spatium(3.5))
                  stemLen = Spatium(3.5);
            if (ticks < division/16)
                  hookIdx = 5;
            else if (ticks < division/8)
                  hookIdx = 4;
            else if (ticks < division/4)
                  hookIdx = 3;
            else if (ticks < division/2)
                  hookIdx = 2;
            else if (ticks < division)
                  hookIdx = 1;
            else if (ticks < division*2)
                  hookIdx = 0;
            else if (ticks < division*4)  // < 1/1
                  hookIdx = 0;
            else {
                  setStem(0);
                  return;
                  }
            }

      double headCorrection = 0.2;

      stemLen += Spatium((downpos - uppos) * .5 - headCorrection);
      if (!_stem) {
            _stem = new Stem(score());
            _stem->setParent(this);
            }
      _stem->setLen(stemLen);

      double pstemLen = point(stemLen);
      QPointF npos = stemPos(up, false);
      if (up)
            npos += QPointF(0, -pstemLen);
      _stem->setPos(npos);

      //-----------------------------------------
      //  process hook
      //-----------------------------------------

      if (hookIdx) {
            if (!up)
                  hookIdx = -hookIdx;
            if (!_hook) {
                  _hook = new Hook(score());
                  _hook->setParent(this);
                  }
            _hook->setIdx(hookIdx, _grace);
            qreal lw = point(::style->stemWidth) * .5;
            _hook->setPos(npos + QPointF(lw, up ? -lw : pstemLen));
            }
      else
            setHook(0);
      }

//---------------------------------------------------------
//   addHelpLine
//---------------------------------------------------------

void Chord::addHelpLine(double x, double y, int i)
      {
      HelpLine* h = new HelpLine(score());
      h->setParent(this);
      double ho = 0.0;
      //
      // Experimental:
      //
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* n = *in;
            if (n->line() >= (i-1) && n->line() <= (i+1) && n->accidental()) {
                  ho = _spatium * .25;
                  h->setLen(h->len() - Spatium(.25));
                  }
            }
      h->setPos(x + ho, y + _spatium * .5 * i);
      helpLines.push_back(h);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Chord::layout(ScoreLayout* layout)
      {
      if (notes.empty())
            return;
      double _spatium = layout->spatium();
      Note* upnote     = notes.back();
      double headWidth = upnote->headWidth();

      computeUp();
      bool up = isUp();

      //-----------------------------------------
      //  process notes
      //    - position
      //-----------------------------------------

      int minMove = 1;
      int maxMove = -1;

      System* s = segment()->measure()->system();
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* note = *in;
            double x = 0;

            int move = note->move();
            if (move < minMove)
                  minMove = move;
            if (move > maxMove)
                  maxMove = move;
            double y = s->staff(staffIdx() + move)->bbox().y();
            y        -= s->staff(staffIdx())->bbox().y();
            y        += (*in)->line() * _spatium * .5;

            if (note->mirror())
                  x += up ? headWidth : - headWidth;
            note->setPos(x, y);
            Accidental* accidental = note->accidental();
            if (accidental) {
                  double x = point(style->prefixNoteDistance);
                  if (_grace)
                        x /= 2;
                  // accidental->setPos(- x - accidental->width(), 0);
                  accidental->setPos(- x - accidental->width() - accidental->bbox().x(), 0);
                  }
            }

      //-----------------------------------------
      //  process help lines
      //-----------------------------------------

      for (iHelpLine l = helpLines.begin(); l != helpLines.end(); ++l)
            delete *l;
      helpLines.clear();

      //---------------------------------------------------
      //    process help lines for notes
      //    moved to upper staff
      //---------------------------------------------------

      int uppos;
      int downpos;
      if (minMove == -1) {
            uppos   = 1000;
            downpos = -1000;
            for (iNote in = notes.begin(); in != notes.end(); ++in) {
                  if ((*in)->move() == -1) {
                        if ((*in)->line() < uppos)
                              uppos = (*in)->line();
                        if ((*in)->line() > downpos)
                              downpos = (*in)->line();
                        }
                  }
            if (uppos < 0 || downpos >= 10) {
                  double x = upnote->pos().x();
                  x += headWidth/2 - _spatium;

                  double y = s->staff(staffIdx() - 1)->bbox().y();
                  y        -= s->staff(staffIdx())->bbox().y();

                  for (int i = -2; i >= uppos; i -= 2)
                        addHelpLine(x, y, i);
                  for (int i = 10; i <= downpos; i += 2)
                        addHelpLine(x, y, i);
                  }
            }

      uppos   = 1000;
      downpos = -1000;
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            if ((*in)->move() == 0) {
                  if ((*in)->line() < uppos)
                        uppos = (*in)->line();
                  if ((*in)->line() > downpos)
                        downpos = (*in)->line();
                  }
            }
      if (uppos < 0 || downpos >= 10) {
            double x = upnote->pos().x();
            x += headWidth/2 - _spatium;

            for (int i = -2; i >= uppos; i -= 2) {
                  addHelpLine(x, 0.0, i);
                  }
            for (int i = 10; i <= downpos; i += 2) {
                  addHelpLine(x, 0.0, i);
                  }
            }

      //---------------------------------------------------
      //    process help lines for notes
      //    moved to lower staff
      //---------------------------------------------------

      if (maxMove == 1) {
            uppos   = 1000;
            downpos = -1000;
            for (iNote in = notes.begin(); in != notes.end(); ++in) {
                  if ((*in)->move() == 1) {
                        if ((*in)->line() < uppos)
                              uppos = (*in)->line();
                        if ((*in)->line() > downpos)
                              downpos = (*in)->line();
                        }
                  }
            if (uppos < 0 || downpos >= 10) {
                  double x = upnote->pos().x();
                  x += headWidth/2 - _spatium;

                  double y = s->staff(staffIdx() + 1)->bbox().y();
                  y        -= s->staff(staffIdx())->bbox().y();

                  for (int i = -2; i >= uppos; i -= 2)
                        addHelpLine(x, y, i);
                  for (int i = 10; i <= downpos; i += 2)
                        addHelpLine(x, 0.0, i);
                  }
            }

      //-----------------------------------------
      //  Note Attributes
      //-----------------------------------------

      layoutAttributes(layout);
#if 0
      double x = upnote->pos().x();
      double y = upnote->pos().y();
      double headHeight = upnote->height();
      if (up) {
            y -= point(style->propertyDistanceStem + Spatium(3.5));
            x += headWidth;
            }
      else {
            y -= (point(style->propertyDistanceHead) + headHeight/2);
            x += headWidth * .5;
            }

      for (iAttribute ia = attributes.begin(); ia != attributes.end(); ++ia) {
            (*ia)->setPos(x, y);
            y -= (point(style->propertyDistance) + (*ia)->bbox().height());
            }
#endif

      //-----------------------------------------
      //  Fingering
      //-----------------------------------------

      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* note = *in;
            QList<Text*>& fingering = note->fingering();
            double x = _spatium * 0.8 + note->headWidth();
            foreach(Text* f, fingering) {
                  f->setPos(x, 0.0);
                  // TODO: x += _spatium;
                  // if we have two fingerings and move the first,
                  // the second will also change position because their
                  // position in this list changes
                  }
            }
      }

//-----------------------------------------------------------------------------
//   computeUp
//    rules:
//      single note:
//          All notes beneath the middle line: upward stems
//          All notes on or above the middle line: downward stems
//      two notes:
//          If the interval above the middle line is greater than the interval
//             below the middle line: downward stems
//          If the interval below the middle line is greater than the interval
//             above the middle line: upward stems
//          If the two notes are the same distance from the middle line:
//             stem can go in either direction. but most engravers prefer
//             downward stems
//       > two notes:
//          If the interval of the highest note above the middle line is greater
//             than the interval of the lowest note below the middle line:
//             downward stems
//          If the interval of the lowest note below the middle line is greater
//             than the interval of the highest note above the middle line:
//             upward stem
//          If the highest and the lowest notes are the same distance from
//          the middle line:, use these rules to determine stem direction:
//             - If the majority of the notes are above the middle:
//               downward stems
//             - If the majority of the notes are below the middle:
//               upward stems
//-----------------------------------------------------------------------------

void Chord::computeUp()
      {
      if (_stemDirection != AUTO) {
            _up = _stemDirection == UP;
            return;
            }
      if (_grace) {
            _up = true;
            return;
            }
      if (tickLen() >= division*4) {
            _up = true;
            return;
            }

      Note* upnote = upNote();
      if (notes.size() == 1) {
            if (upnote->move() > 0)
                  _up = true;
            else if (upnote->move() < 0)
                  _up = false;
            else
                  _up = upnote->line() > 4;
            return;
            }
      Note* downnote = downNote();
      int ud = upnote->line() - 4;
      int dd = downnote->line() - 4;
      if (-ud == dd) {
            int up = 0;
            for (ciNote in = notes.begin(); in != notes.end(); ++in) {
                  int l = (*in)->line();
                  if (l <= 4)
                        --up;
                  else
                        ++up;
                  }
            _up = up > 0;
            }
      _up = dd > -ud;
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

int Chord::move() const
      {
      int move = notes.front()->move();
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            if ((*in)->move() != move)
                  return 0;
            }
      return move;
      }

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
      {
      Note* note = 0;
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            if ((*in)->selected()) {
                  if (note)
                        return 0;
                  note = *in;
                  }
            }
      return note;
      }

//---------------------------------------------------------
//   Chord::write
//---------------------------------------------------------

void Chord::write(Xml& xml) const
      {
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
      if (_grace)
            xml.tag("GraceNote", _grace);
      switch(_stemDirection) {
            case UP:   xml.tag("StemDirection", "up"); break;
            case DOWN: xml.tag("StemDirection", "down"); break;
            case AUTO: break;
            }
      for (ciNote in = notes.begin(); in != notes.end(); ++in)
            (*in)->write(xml);
      xml.etag("Chord");
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(QDomNode node, int staffIdx)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "Note") {
                  Note* note = new Note(score());
                  note->setParent(this);
                  note->setGrace(_grace);
                  note->setStaff(staff());
                  note->setVoice(voice());
                  note->setHead(tickLen());
                  note->read(node);
                  notes.append(note);
                  }
            else if (tag == "GraceNote")
                  _grace = i;
            else if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (ChordRest::readProperties(node))
                  ;
            else if (tag == "Slur") {
                  readSlur(node, staffIdx);
                  }
            else
                  domError(node);
            }
      qSort(notes.begin(), notes.end(), notesPitchCompare);
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Chord::dump() const
      {
      printf("Chord tick %d  len %d\n", tick(), tickLen());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Chord::space(double& min, double& extra) const
      {
      extra = 0.0; // point(::style->minNoteDistance);
      double mirror = 0.0;
      double hw     = 0.0;

      for (ciNote i = notes.begin(); i != notes.end(); ++i) {
            Note* note = *i;
            double lhw = note->headWidth();
            if (lhw > hw)
                  hw = lhw;
            double prefixWidth  = 0.0;
            if (note->accidental()) {
                  prefixWidth = note->accidental()->width();
                  if (_grace)
                       prefixWidth += point(::style->prefixNoteDistance)/2.0;
                  else
                       prefixWidth += point(::style->prefixNoteDistance);
                  if (prefixWidth > extra)
                        extra = prefixWidth;
                  }
            if (note->mirror()) {
                  if (isUp()) {
                        // note head on the right side of stem
                        mirror = lhw;
                        }
                  else {
                        // note head on left side of stem
                        if ((lhw + prefixWidth) > extra)
                              extra = lhw + prefixWidth;
                        }
                  }
            }
      min = mirror + hw;
      if (isUp() && _hook)
            min += _hook->width();
      }

//---------------------------------------------------------
//   find
//---------------------------------------------------------

#if 0
Note* NoteList::find(int pitch) const
      {
      foreach(Note* n, notes) {
            if (n->pitch() == pitch)
                  return n;
            }
      return 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void NoteList::add(Note* n)
      {
      notes.append(n);
      qSort(notes.begin(), notes.end(), notesPitchCompare);
      }
#endif

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Chord::upPos() const
      {
      Note* upnote = notes.back();
      return upnote->pos().y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Chord::downPos() const
      {
      Note* downnote = notes.front();
      return downnote->pos().y();
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Chord::centerX() const
      {
      qreal x;
      if (_up) {
            Note* upnote = notes.back();
            x  = upnote->pos().x();
            x += upnote->headWidth() * .5;
            }
      else {
            Note* downnote = notes.front();
            x = downnote->pos().x();
            x += downnote->headWidth() * .5;
            }
      return x;
      }

//---------------------------------------------------------
//   sortNotes
//---------------------------------------------------------

void Chord::sortNotes()
      {
      qSort(notes.begin(), notes.end(), notesPitchCompare);
      }

