//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: note.cpp,v 1.63 2006/03/28 14:58:58 wschweer Exp $
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
 Implementation of classes Note and ShadowNote.
*/

#include "score.h"
#include "key.h"
#include "note.h"
#include "chord.h"
#include "sym.h"
#include "xml.h"
#include "slur.h"
#include "navigate.h"
#include "measure.h"
#include "text.h"
#include "sig.h"
#include "clef.h"
#include "globals.h"
#include "segment.h"
#include "preferences.h"
#include "padstate.h"
#include "utils.h"
#include "painter.h"
#include "style.h"
#include "staff.h"

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Score* s)
   : Element(s)
      {
      _pitch          = 0;
      _userAccidental = -1;
      _durationType   = D_QUARTER;
      _grace          = false;
      _accidental     = 0;
      _mirror         = false;
      _line           = 0;
      _move           = 0;
      _lineOffset     = 0;
      _dots           = 0;
      _tieFor         = 0;
      _tieBack        = 0;
      _head           = 0;
      }

Note::Note(Score* s, int p, bool g)
   : Element(s)
      {
      _durationType   = D_QUARTER;
      _grace          = g;
      _accidental     = 0;
      _userAccidental = -1;
      _mirror         = false;
      _line           = 0;
      _move           = 0;
      _lineOffset     = 0;
      _dots           = 0;
      _tieFor         = 0;
      _tieBack        = 0;
      _pitch          = p;
      _head           = 0;
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::~Note()
      {
      if (_accidental)
            delete _accidental;
      }

//---------------------------------------------------------
//   headWidth
//---------------------------------------------------------

double Note::headWidth() const
      {
      return symbols[_head].width();
      }

//---------------------------------------------------------
//   totalTicks
//---------------------------------------------------------

/**
 Return total tick len of tied notes
*/

int Note::totalTicks() const
      {
      const Note* note = this;
      while (note->tieBack())
            note = note->tieBack()->startNote();
      int len = 0;
      while (note->tieFor() && note->tieFor()->endNote()) {
            len += note->chord()->tickLen();
            note = note->tieFor()->endNote();
            }
      len += note->chord()->tickLen();
      return len;
      }

//---------------------------------------------------------
//   changePitch
//---------------------------------------------------------

/**
 Computes _line and _accidental,
 resets _userAccidental.
*/

void Note::changePitch(int n)
      {
      setPitch(n);
      _userAccidental = -1;
      chord()->measure()->layoutNoteHeads(staffIdx());
      }

//---------------------------------------------------------
//   changeAccidental
//---------------------------------------------------------

/**
 Sets a "user selected" accidental.
 This recomputes _pitch.
*/

void Note::changeAccidental(int pre)
      {
      _userAccidental = pre;

      Measure* m = chord()->measure();
      char tversatz[128];
      memset(tversatz, 0, sizeof(tversatz));
      if (pre == ACC_NONE) {
            // compute current state:
            bool found = false;
            Segment* segment;
            for (segment = m->first(); segment; segment = segment->next()) {
                  int startTrack = staffIdx() * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = segment->element(track);
                        if (e == 0 || e->type() != CHORD)
                              continue;
                        if (chord() == (Chord*) e) {
                              found = true;
                              break;
                              }
                        NoteList* nl = ((Chord*)e)->noteList();
                        int tick     = ((Chord*)e)->tick();
                        for (iNote in = nl->begin(); in != nl->end(); ++in) {
                              Note* note  = in->second;
                              int nPrefix = note->accidentalIdx();
                              if (nPrefix) {
                                    int offset   = score()->clefOffset(tick, staffIdx());
                                    int line     = note->line();
                                    int l1       = 45 - line + offset;
                                    tversatz[l1] = nPrefix;
                                    }
                              }
                        }
                  if (found)
                        break;
                  }
            if (segment == 0)
                  printf("Note not found in measure\n");
            }

      static int preTab[] = {
            0,  // ACC_NONE
            1,  // ACC_SHARP
            -1, // ACC_FLAT
            2,  // ACC_SHARP2
            -2, // ACC_FLAT2
            0,  // ACC_NAT
            1, -1, 2, -2, 0,  // () brackets
            1, -1, 2, -2, 0,  // [] brackets
            };

     static int tab3[15][8] = {
          //c  d  e  f  g  a  b  c
          //------------------------
          { 2, 2, 2, 2, 2, 2, 2, 2 },    // ces
          { 2, 2, 2, 0, 2, 2, 2, 2 },    // ges
          { 0, 2, 2, 0, 2, 2, 2, 0 },    // des
          { 0, 2, 2, 0, 0, 2, 2, 0 },    // as
          { 0, 0, 2, 0, 0, 2, 2, 0 },    // es
          { 0, 0, 2, 0, 0, 0, 2, 0 },    // B
          { 0, 0, 0, 0, 0, 0, 2, 0 },    // F

          { 0, 0, 0, 0, 0, 0, 0, 0 },    // C

          { 0, 0, 0, 1, 0, 0, 0, 0 },    // G
          { 1, 0, 0, 1, 0, 0, 0, 1 },    // D
          { 1, 0, 0, 1, 1, 0, 0, 1 },    // A
          { 1, 1, 0, 1, 1, 0, 0, 1 },    // E
          { 1, 1, 0, 1, 1, 1, 0, 1 },    // H
          { 1, 1, 1, 1, 1, 1, 0, 1 },    // fis
          { 1, 1, 1, 1, 1, 1, 1, 1 }     // cis
          };

      int tick   = chord()->tick();
      int offset = score()->clefOffset(tick, staffIdx());
      int l1     = 45 - _line + offset;

      int curPre = tversatz[l1];

      int key    = score()->keymap->key(tick);
      int keyPre = tab3[key+7][l1 % 7];

      int pitchOffset;

      if (pre == ACC_NONE) {
            if (curPre == ACC_NONE)
                  pitchOffset = preTab[keyPre % 100];
            else
                  pitchOffset = preTab[curPre % 100];
            }
      else if (pre == ACC_NATURAL)
            pitchOffset = 0;
      else {
            pitchOffset = preTab[pre % 100]; //  + preTab[keyPre % 100];
            }

      int clef = staff()->clef()->clef(tick);
      int newPitch = line2pitch(_line, clef) + pitchOffset;

      setPitch(newPitch);
//DEBUG1      chord()->measure()->layoutNoteHeads(staffIdx());
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* el)
      {
	el->setParent(this);
      if (el->type() == FINGERING) {
            _fingering.append((Fingering*) el);
            }
      else if (el->type() == TIE) {
            Tie* tie = (Tie*)el;
		tie->setStartNote(this);
            tie->setStaff(staff());
		setTieFor(tie);
            }
      else
            printf("Note::add() not impl. %s\n", el->name());
      }

//---------------------------------------------------------
//   setTieBack
//---------------------------------------------------------

void Note::setTieBack(Tie* t)
      {
      _tieBack = t;
      if (t && _accidental) {          // dont show prefix of second tied note
            delete _accidental;
            _accidental = 0;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Note::remove(Element* el)
      {
      if (el->type() == FINGERING) {
            int i = _fingering.indexOf((Fingering*)el);
            if (i != -1)
                  _fingering.removeAt(i);
            else
                  printf("Note::remove: fingering not found\n");
            }
	else if (el->type() == TIE) {
            Tie* tie = (Tie*) el;
            setTieFor(0);
            if (tie->endNote())
                  tie->endNote()->setTieBack(0);
      	ElementList* el = tie->elements();
            for (iElement i = el->begin(); i != el->end(); ++i) {
	            score()->removeElement(*i);
                  }
            el->clear();
            }
      else
            printf("Note::remove() not impl. %s\n", el->name());
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF Note::stemPos(bool upFlag) const
      {
      double sw = point(style->stemWidth);
      double x = pos().x();
      double y = pos().y();

      if (_mirror)
            upFlag = !upFlag;
      qreal xo = symbols[_head].bbox().x();
      if (upFlag) {
            x += symbols[_head].width() - sw;
            y -= _spatium * .2;
            }
      else {
            x += sw * .5;
            y += _spatium * .2;
            }
      return QPointF(x + xo, y);
      }

//---------------------------------------------------------
//   setAccidental
//---------------------------------------------------------

void Note::setAccidental(int pre)
      {
      if (pre && !_tieBack) {
            if (_accidental) {
                  _accidental->setSubtype(pre);
                  }
            else {
                  _accidental = new Accidental(score());
                  _accidental->setSubtype(_grace ? 100 + pre : pre);
                  _accidental->setParent(this);
                  _accidental->setVoice(voice());
                  }
            }
      else if (_accidental) {
            delete _accidental;
            _accidental = 0;
            }
      bboxUpdate();
      }

//---------------------------------------------------------
//   setHead
//---------------------------------------------------------

/**
 Set note head and dots depending on \a ticks.
*/

void Note::setHead(int ticks)
      {
      if (ticks < (2*division)) {
            _head = _grace ? s_quartheadSym : quartheadSym;
            int nt = division;
            for (int i = 0; i < 4; ++i) {
                  if (ticks / nt) {
                        int rest = ticks % nt;
                        if (rest) {
                              _dots = 1;
                              nt /= 2;
                              if (rest % nt)
                                    ++_dots;
                              }
                        break;
                        }
                  nt /= 2;
                  }
            }
      else if (ticks >= (4 * division)) {
            _head = _grace ? s_wholeheadSym : wholeheadSym;
            if (ticks % (4 * division))
                  _dots = 1;
            }
      else {
            _head = _grace ? s_halfheadSym : halfheadSym;
            if (ticks % (2 * division))
                  _dots = 1;
            }
      bboxUpdate();
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void Note::setType(DurationType t)
      {
      switch(t) {
            case D_256TH:
            case D_128TH:
            case D_64TH:
            case D_32ND:
            case D_16TH:
            case D_EIGHT:
            case D_QUARTER:
                  _head = _grace ? s_quartheadSym : quartheadSym;
                  break;
            case D_HALF:
                  _head = _grace ? s_halfheadSym : halfheadSym;
                  break;
            case D_WHOLE:
                  _head = _grace ? s_wholeheadSym : wholeheadSym;
                  break;
            case D_BREVE:
            case D_LONG:      // not impl.
                  _head = _grace ? s_brevisheadSym : brevisheadSym;
                  break;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw1(Painter& p)
      {
      symbols[_head].draw(p);

      if (_dots) {
            double y = 0;
            // do not draw dots on line
            if (_line >= 0 && (_line & 1) == 0) {
                  if (chord()->isUp())
                        y = -_spatium *.5;
                  else
                        y = _spatium * .5;
                  }
            for (int i = 1; i <= _dots; ++i)
                  symbols[dotSym].draw(p, symbols[_head].width() + point(style->dotNoteDistance) * i, y);
            }
      if (_tieFor)
            _tieFor->draw(p);
      if (_accidental)
            _accidental->draw(p);
      foreach(Fingering* f, _fingering)
            f->draw(p);
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void Note::bboxUpdate()
      {
      setbbox(symbols[_head].bbox());

      if (_tieFor)
            orBbox(_tieFor->bbox().translated(_tieFor->pos()));
      if (_accidental)
            orBbox(_accidental->bbox().translated(_accidental->pos()));
      foreach(const Fingering* f, _fingering)
            orBbox(f->bbox().translated(f->pos()));
      if (_dots) {
            double y = 0;
            if ((_line & 1) == 0) {
                  if (chord()->isUp())
                        y = -_spatium *.5;
                  else
                        y = _spatium * .5;
                  }
            for (int i = 1; i <= _dots; ++i) {
                  QRectF dot = symbols[dotSym].bbox();
                  double xoff = symbols[_head].width() + point(style->dotNoteDistance) * i;
                  dot.translate(xoff, y);
                  orBbox(dot);
                  }
            }
      }

//---------------------------------------------------------
//   Note::write
//---------------------------------------------------------

void Note::write(Xml& xml) const
      {
      xml.stag("Note");
      Element::writeProperties(xml);
      xml.tag("pitch", pitch());
      if (_userAccidental != -1) {
            xml.tag("prefix", _userAccidental);
            xml.tag("line", _line);
            }
      foreach(const Fingering* f, _fingering)
            f->write(xml);
      if (_tieFor)
            _tieFor->write(xml);
      if (_move)
            xml.tag("move", _move);
      xml.etag("Note");
      }

//---------------------------------------------------------
//   readSlur
//---------------------------------------------------------

void Chord::readSlur(QDomNode node, int /*staff*/)
      {
      int type = 0;         // 0 - begin, 1 - end
      Placement placement = PLACE_AUTO;

      QDomElement e = node.toElement();
      QString s = e.attribute("type", "");
      if (s == "begin")
            type = 0;
      else if (s == "end")
            type = 1;
      else
            printf("Chord::readSlur: unknown type <%s>\n", s.toLatin1().data());

//      int number = e.attribute("number", "0").toInt();
      s = e.attribute("placement", "");
      if (s == "auto")
            placement = PLACE_AUTO;
      else if (s == "above")
            placement = PLACE_ABOVE;
      else if (s == "below")
            placement = PLACE_BELOW;
#if 0 //TODO
      XmlSlur* xs = &(xml.slurs[number]);
      if (type == 0) {
            xs->tick = tick();
            xs->placement = placement;
            xs->voice = voice();
            }
      else {
            Slur* slur = new Slur(score());
            slur->setUpMode(xs->placement);

            slur->setStart(xs->tick, staff);
            slur->setEnd(tick(), staff);
            measure()->add(slur);
            }
#endif
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "pitch")
                  setPitch(i);
            else if (tag == "prefix")
                  _userAccidental = i;
            else if (tag == "line")
                  _line = i;
            else if (tag == "Tie") {
                  _tieFor = new Tie(score());
                  _tieFor->setStaff(staff());
                  _tieFor->read(node);
                  _tieFor->setStartNote(this);
                  }
            else if (tag == "Fingering") {
                  Fingering* f = new Fingering(score());
                  f->setStaff(staff());
                  f->read(node);
                  f->setParent(this);
                  _fingering.append(f);
                  }
            else if (tag == "move")
                  _move = i;
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   findSelectableElement
//---------------------------------------------------------

/**
 Find selectable element nearest to \a p.

 Note: \a p is Chord relative.
*/

Element* Note::findSelectableElement(QPointF p) const
      {
      p -= pos();       // note relative
      if (tieFor() && tieFor()->contains(p))
            return tieFor();
      if (accidental() && accidental()->contains(p))
            return accidental();
      foreach(Fingering* f, _fingering) {
            if (f->contains(p))
                  return f;
            }
      if (symbols[_head].bbox().contains(p))
            return (Element*)this;
      return 0;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Note::drag(const QPointF& s)
      {
      QRectF bb(chord()->bbox());
      _lineOffset = lrint(s.y() * 2 / _spatium);
      bboxUpdate();
      chord()->layout();
      chord()->measure()->layoutBeams();
      bb |= chord()->bbox();
      return bb.translated(chord()->aref());
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Note::endDrag()
      {
      if (_lineOffset == 0)
            return;

      _line      += _lineOffset;
      _lineOffset = 0;
      int clef    = chord()->staff()->clef()->clef(chord()->tick());
      _pitch      = line2pitch(_line, clef);
      }

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
   : Element(s)
      {
      _sym  = new Sym(symbols[quartheadSym]);
      _line = 1000;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF& ShadowNote::bbox() const
      {
      return _sym->bbox();
      }

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::~ShadowNote()
      {
      delete _sym;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(Painter& p)
      {
      if (!visible())
            return;

      QPointF ap(apos());

      QRect r(abbox().toRect());
      QRect c(p.clipRect());

      if (c.intersects(r)) {
            QPointF ap(apos());
            p.translate(ap);
            qreal lw = point(style->helpLineWidth);
            QPen pen(preferences.selectColor[padState.voice].light(160));
            pen.setWidthF(lw);
            p.setPen(pen);

            _sym->draw(p);

            double x1 = _sym->width()/2 - _spatium;
            double x2 = x1 + 2 * _spatium;

            for (int i = -2; i >= _line; i -= 2) {
                  double y = _spatium * .5 * (i - _line);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            for (int i = 10; i <= _line; i += 2) {
                  double y = _spatium * .5 * (i - _line);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            p.translate(-ap);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ShadowNote::layout()
      {
      setbbox(_sym->bbox());
      double x  = _sym->width()/2 - _spatium;
      double lw = point(style->helpLineWidth);

      QRectF r(0, -lw/2.0, 2 * _spatium, lw);

      for (int i = -2; i >= _line; i -= 2)
            orBbox(r.translated(QPointF(x, _spatium * .5 * (i - _line))));
      for (int i = 10; i <= _line; i += 2)
            orBbox(r.translated(QPointF(x, _spatium * .5 * (i - _line))));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(const QPointF&, int type, const QDomNode&) const
      {
      return (type == ATTRIBUTE || type == FINGERING || type == ACCIDENTAL);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void Note::drop(const QPointF&, int t, const QDomNode& node)
      {
      switch(t) {
            case ATTRIBUTE:
                  {
                  NoteAttribute* atr = new NoteAttribute(score());
                  atr->read(node);
                  score()->addAttribute(this, atr);
                  }
                  break;
            case FINGERING:
                  {
                  Fingering* f = new Fingering(score());
                  f->read(node);
                  f->setParent(this);
                  score()->select(f, 0, 0);
                  score()->undoOp(UndoOp::AddElement, f);
                  chord()->layout();
                  }
                  break;
            case ACCIDENTAL:
                  {
                  Accidental* a = new Accidental(0);
                  a->read(node);
                  int subtype = a->subtype();
                  delete a;
                  score()->addAccidental(this, subtype);
                  }
                  break;

            default:
                  break;
            }
      }

