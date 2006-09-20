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

#include "score.h"
#include "key.h"
#include "note.h"
#include "chord.h"
#include "symbols.h"
#include "xml.h"
#include "slur.h"
#include "navigate.h"
#include "measure.h"
#include "textelement.h"
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
//    pitch - midi pitch 0 - 127
//    len   - midi ticks (1/division) quarternote
//    stem/flag
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
      _fingering      = 0;
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
      _fingering      = 0;
      _pitch          = p;
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::~Note()
      {
      if (_fingering)
            delete _fingering;
      if (_accidental)
            delete _accidental;
      }

//---------------------------------------------------------
//   totalTicks
//    give total tick len of tied notes
//---------------------------------------------------------

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

void Note::changePitch(int n)
      {
      setPitch(n);
      _userAccidental = -1;
      chord()->measure()->layoutNoteHeads(staffIdx());
      }

//---------------------------------------------------------
//   changeAccidental
//    sets a "user selected" accidental
//    this recomputes _pitch
//---------------------------------------------------------

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
                  pitchOffset = preTab[keyPre];
            else
                  pitchOffset = preTab[curPre];
            }
      else if (pre == ACC_NATURAL)
            pitchOffset = 0;
      else {
            pitchOffset = preTab[pre]; //  + preTab[keyPre];
            }

      int clef = staff()->clef()->clef(tick);
      int newPitch = line2pitch(_line, clef) + pitchOffset;

      setPitch(newPitch);
      chord()->measure()->layoutNoteHeads(staffIdx());
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* el)
      {
	el->setParent(this);
      if (el->type() == FINGERING) {
            _fingering = (Fingering*) el;
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
      if (el->type() == FINGERING)
            _fingering = 0;
	else if (el->type() == TIE) {
            Tie* tie = (Tie*) el;
            setTieFor(0);
            if (tie->endNote())
                  tie->endNote()->setTieBack(0);
      	ElementList* el = tie->elements();
            for (iElement i = el->begin(); i != el->end(); ++i) {
	            score()->removeObject(*i);
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
      double sw2 = point(style->stemWidth) * .5;
      double x = pos().x();
      double y = pos().y();

      if (_mirror)
            upFlag = !upFlag;
      qreal xo = _sym.bbox().x();
      if (upFlag) {
            x += _sym.width() - sw2;
            y -= _spatium * .2;
            }
      else {
            x += sw2;
            y += _spatium * .2;
            }
      return QPointF(x + xo, y);
      }

//---------------------------------------------------------
//   setAccidental
//    called from
//       Measure::layoutNoteHeads()
//       Note::changeAccidental()
//---------------------------------------------------------

void Note::setAccidental(int pre)
      {
      if (pre && !_tieBack) {
            if (_accidental) {
                  _accidental->setIdx(pre);
                  }
            else {
                  _accidental = new Accidental(score(), pre, _grace);
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
//    set note head + dots depending on tick len
//---------------------------------------------------------

void Note::setHead(int ticks)
      {
      switch (ticks) {
            case 0:     // grace note?
                  _sym = _grace ? s_quartheadSym : quartheadSym;
                  break;
            case 6*division:        // 1/1 + 1/2  (gibt's das???)
                  _dots  = 1;
            case 4*division:        // 1/1
                  _sym = _grace ? s_wholeheadSym : wholeheadSym;
                  break;
            case 3*division:        // 1/2
                  _dots = 1;
            case 2*division:        // 1/2
                  _sym = _grace ? s_halfheadSym : halfheadSym;
                  break;
            case division + division/2:         // dotted values
            case division/2 + division/4:
            case division/4 + division/8:
            case division/8 + division/16:
            case division/16 + division/32:
            case division/32 + division/64:
                  _dots  = 1;
            case division:          // quarternote
            case division/2:        // 1/8
            case division/4:        // 1/16
            case division/8:        // 1/32
            case division/16:       // 1/64
            case division/32:       // 1/128
                  _sym = _grace ? s_quartheadSym : quartheadSym;
                  break;
            default:
//                  fprintf(stderr, "invalid note len %d\n", l);
                  _sym = _grace ? s_quartheadSym : quartheadSym;
                  return;
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
                  _sym = _grace ? s_quartheadSym : quartheadSym;
                  break;
            case D_HALF:
                  _sym = _grace ? s_halfheadSym : halfheadSym;
                  break;
            case D_WHOLE:
                  _sym = _grace ? s_wholeheadSym : wholeheadSym;
                  break;
            case D_BREVE:
            case D_LONG:      // not impl.
                  _sym = _grace ? s_brevisheadSym : brevisheadSym;
                  break;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw1(Painter& p) const
      {
      _sym.draw(p);

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
                  dotSym.draw(p, _sym.width() + point(style->dotNoteDistance) * i, y);
            }
      if (_tieFor)
            _tieFor->draw(p);
      if (_accidental)
            _accidental->draw(p);
      if (_fingering)
            _fingering->draw(p);
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void Note::bboxUpdate()
      {
      setbbox(_sym.bbox());

      if (_tieFor)
            orBbox(_tieFor->bbox().translated(_tieFor->pos()));
      if (_accidental)
            orBbox(_accidental->bbox().translated(_accidental->pos()));
      if (_fingering)
            orBbox(_fingering->bbox().translated(_fingering->pos()));
      if (_dots) {
            double y = 0;
            if ((_line & 1) == 0) {
                  if (chord()->isUp())
                        y = -_spatium *.5;
                  else
                        y = _spatium * .5;
                  }
            for (int i = 1; i <= _dots; ++i) {
                  QRectF dot = dotSym.bbox();
                  double xoff = _sym.width() + point(style->dotNoteDistance) * i;
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
      if (_fingering)
            _fingering->write(xml);
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
                  _fingering = new Fingering(score());
                  _fingering->setStaff(staff());
                  _fingering->read(node);
                  _fingering->setParent(this);
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
//    p is Chord relative
//---------------------------------------------------------

Element* Note::findSelectableElement(QPointF p) const
      {
      p -= pos();       // note relative
      if (tieFor() && tieFor()->contains(p))
            return tieFor();
      if (accidental() && accidental()->contains(p))
            return accidental();
      if (fingering() && fingering()->contains(p))
            return fingering();
      if (_sym.bbox().contains(p))
            return (Element*)this;
      return 0;
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

bool Note::startDrag(const QPointF&)
      {
      return true;
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
      _sym  = quartheadSym;
      setbbox(_sym.bbox());
      _line = 1000;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(Painter& p) const
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

            _sym.draw(p);

            double x1 = _sym.width()/2 - _spatium;
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
      setbbox(_sym.bbox());
      double x  = _sym.width()/2 - _spatium;
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

bool Note::acceptDrop(int type, int) const
      {
      return (type == ATTRIBUTE
        || type == FINGERING);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void Note::drop(const QPointF&, int t, int st)
      {
      switch(t) {
            case ATTRIBUTE:
                  {
                  NoteAttribute* atr = new NoteAttribute(score());
                  atr->setSubtype(st);
                  score()->addAttribute(this, atr);
                  }
                  break;
            case FINGERING:
                  {
                  Fingering* f = new Fingering(score());
                  f->setSubtype(st);
                  add(f);
                  score()->select(f, 0, 0);
                  score()->undoOp(UndoOp::AddObject, f);
                  chord()->layout();
                  }
                  break;

            default:
                  break;
            }
      }

