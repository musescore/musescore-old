//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: note.cpp,v 1.63 2006/03/28 14:58:58 wschweer Exp $
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

/**
 \file
 Implementation of classes Note and ShadowNote.
*/

#include "note.h"
#include "score.h"
#include "key.h"
#include "chord.h"
#include "sym.h"
#include "xml.h"
#include "slur.h"
#include "text.h"
#include "clef.h"
#include "preferences.h"
#include "padstate.h"
#include "staff.h"
#include "viewer.h"
#include "pitchspelling.h"
#include "breath.h"
#include "arpeggio.h"
#include "tremolo.h"

int Note::noteHeads[HEAD_GROUPS][4] = {
      { wholeheadSym,         halfheadSym,         quartheadSym,    brevisheadSym},
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,  wholecrossedheadSym },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,  wholediamondheadSym},
      { wholetriangleheadSym, halftriangleheadSym, triangleheadSym, wholetriangleheadSym},
      };

int Note::smallNoteHeads[HEAD_GROUPS][4] = {
      { s_wholeheadSym,         s_halfheadSym,         s_quartheadSym,    s_brevisheadSym },
      { s_wholecrossedheadSym,  s_halfcrossedheadSym,  s_crossedheadSym,  s_wholecrossedheadSym},
      { s_wholediamondheadSym,  s_halfdiamondheadSym,  s_diamondheadSym,  s_wholediamondheadSym},
      { s_wholetriangleheadSym, s_halftriangleheadSym, s_triangleheadSym, s_wholetriangleheadSym},
      };

//---------------------------------------------------------
//   NoteHead
//---------------------------------------------------------

NoteHead::NoteHead(Score* s)
   : Symbol(s)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteHead::write(Xml& xml) const
      {
      xml.stag("NoteHead");
      xml.tag("name", symbols[_sym].name());
      Element::writeProperties(xml);
      xml.etag();
      Element::write(xml);
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Score* s)
   : Element(s)
      {
      _pitch          = 0;
      _durationType   = D_QUARTER;
      _grace          = false;
      _accidental     = 0;
      _mirror         = false;
      _line           = 0;
      _move           = 0;
      _userAccidental = 0;
      _lineOffset     = 0;
      _dots           = 0;
      _tieFor         = 0;
      _tieBack        = 0;
      _head           = 0;
      _tpc            = -1;
      _headGroup      = 0;
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void Note::setPitch(int val)
      {
      _pitch = val;
      _tpc   = pitch2tpc(_pitch);
      }

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

void Note::setTpc(int v)
      {
      _tpc = v;
      _userAccidental = 0;
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
//   headHeight
//---------------------------------------------------------

double Note::headHeight() const
      {
      return symbols[_head].height();
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
*/

void Note::changePitch(int n)
      {
      if (chord()) {
            // keep notes sorted in chord:
            Chord* c     = chord();
            NoteList* nl = c->noteList();
            iNote i;
            for (i = nl->begin(); i != nl->end(); ++i) {
                  if (i->second == this)
                        break;
                  }
            if (i == nl->end()) {
                  printf("Note::changePitch(): note not found in chord()\n");
                  return;
                  }
            nl->erase(i);
            nl->insert(std::pair<const int, Note*> (n, this));
            }
      setPitch(n);
      score()->spell(this);
      }

//---------------------------------------------------------
//   changeAccidental
//---------------------------------------------------------

/**
 Sets a "user selected" accidental.
 This recomputes _pitch and _tpc.
*/

void Note::changeAccidental(int accType)
      {
      _userAccidental = 0;
      int acc1  = Accidental::subtype2value(accType);
      int line  = tpc2line(_tpc);
      _tpc      = line2tpc(line, acc1);
      _pitch    = tpc2pitch(_tpc) + (_pitch / 12) * 12;
      chord()->measure()->layoutNoteHeads(staffIdx());    // compute actual accidental
      int acc2  = accidentalSubtype();
      if (accType != acc2)
            _userAccidental = accType;    // bracketed editorial accidental
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* el)
      {
	el->setParent(this);
      switch(el->type()) {
            case TEXT:
                  _fingering.append((Text*) el);
                  break;
            case TIE:
                  {
                  Tie* tie = (Tie*)el;
	      	tie->setStartNote(this);
                  tie->setStaff(staff());
      		setTieFor(tie);
                  }
                  break;
            case ACCIDENTAL:
                  _accidental = (Accidental*)el;
                  break;
            default:
                  printf("Note::add() not impl. %s\n", el->name());
                  break;
            }
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
      switch(el->type()) {
            case TEXT:
                  {
                  int i = _fingering.indexOf((Text*)el);
                  if (i != -1)
                        _fingering.removeAt(i);
                  else
                        printf("Note::remove: fingering not found\n");
                  }
                  break;
#if 0 // TODO
	      case TIE:
                  {
                  Tie* tie = (Tie*) el;
                  setTieFor(0);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(0);
            	QList<SlurSegment*>* el = tie->elements();
                  for (iElement i = el->begin(); i != el->end(); ++i) {
      	            score()->removeElement(*i);
                        }
                  el->clear();
                  }
                  break;
#endif
            case ACCIDENTAL:
                  _accidental = 0;
                  break;

            default:
                  printf("Note::remove() not impl. %s\n", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF Note::stemPos(bool upFlag) const
      {
      double sw = point(score()->style()->stemWidth) * .5;
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
            x += sw;
            y += _spatium * .2;
            }
      return QPointF(x + xo, y);
      }

//---------------------------------------------------------
//   setAccidentalSubtype
//---------------------------------------------------------

void Note::setAccidentalSubtype(int pre)
      {
      if (pre && !_tieBack) {
            if (!_accidental) {
                  _accidental = new Accidental(score());
                  _accidental->setParent(this);
                  _accidental->setVoice(voice());
                  }
            pre &= pre & (~ACC_SMALL);
            _accidental->setSubtype(_grace ? ACC_SMALL + pre : pre);
            }
      else if (_accidental) {
            delete _accidental;
            _accidental = 0;
            }
      }

//---------------------------------------------------------
//   setHead
//---------------------------------------------------------

/**
 Set note head and dots depending on \a ticks.
*/

void Note::setHead(int ticks)
      {
      int headType = 0;
      if (ticks < (2*division)) {
            headType = 2;
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
            headType = 0;
            if (ticks % (4 * division))
                  _dots = 1;
            }
      else {
            headType = 1;
            if (ticks % (2 * division))
                  _dots = 1;
            }
      if (_grace)
            _head = smallNoteHeads[_headGroup][headType];
      else
            _head = noteHeads[_headGroup][headType];
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void Note::setType(DurationType t)
      {
      int headType = 0;
      switch(t) {
            case D_256TH:
            case D_128TH:
            case D_64TH:
            case D_32ND:
            case D_16TH:
            case D_EIGHT:
            case D_QUARTER:
                  headType = 2;
                  break;
            case D_HALF:
                  headType = 1;
                  break;
            case D_WHOLE:
                  headType = 0;
                  break;
            case D_BREVE:
            case D_LONG:      // not impl.
                  headType = 3;
                  break;
            }
      if (_grace)
            _head = smallNoteHeads[_headGroup][headType];
      else
            _head = noteHeads[_headGroup][headType];
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(QPainter& p)
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
                  symbols[dotSym].draw(p, symbols[_head].width() + point(score()->style()->dotNoteDistance) * i, y);
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Note::bbox() const
      {
      QRectF _bbox = symbols[_head].bbox();
#if 0
      if (_tieFor)
            _bbox |= _tieFor->bbox().translated(_tieFor->pos());
      if (_accidental)
            _bbox |= _accidental->bbox().translated(_accidental->pos());
      foreach(const Text* f, _fingering)
            _bbox |= f->bbox().translated(f->pos());
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
                  double xoff = symbols[_head].width() + point(score()->style()->dotNoteDistance) * i;
                  dot.translate(xoff, y);
                  _bbox |= dot;
                  }
            }
#endif
      return _bbox;
      }

//---------------------------------------------------------
//   isSimple
//---------------------------------------------------------

bool Note::isSimple(Xml& xml) const
      {
      QList<Prop> pl = Element::properties(xml);
      if (_accidental && !_accidental->userOff().isNull())
            return false;
      return (pl.empty() && _fingering.empty() && _tieFor == 0 && _move == 0 && _headGroup == 0);
      }

//---------------------------------------------------------
//   Note::write
//---------------------------------------------------------

void Note::write(Xml& xml) const
      {
      if (isSimple(xml)) {
            xml.tagE(QString("Note pitch=\"%1\" tpc=\"%2\"").arg(pitch()).arg(tpc()));
            }
      else {
            xml.stag("Note");
            QList<Prop> pl = Element::properties(xml);
            xml.prop(pl);
            xml.tag("pitch", pitch());
            xml.tag("tpc", tpc());

            if (_userAccidental)
                  xml.tag("userAccidental", _userAccidental);
            if (_accidental && !_accidental->userOff().isNull())
                  _accidental->write(xml);

            foreach(const Text* f, _fingering)
                  f->write(xml);
            if (_tieFor)
                  _tieFor->write(xml);
            if (_move)
                  xml.tag("move", _move);
            if (_headGroup != 0)
                  xml.tag("head", _headGroup);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(QDomElement e)
      {
      int ptch = e.attribute("pitch", "-1").toInt();
      if (ptch != -1)
            _pitch = ptch;
      int tpcVal = e.attribute("tpc", "-100").toInt();

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "pitch")
                  _pitch = i;
            else if (tag == "tpc")
                  tpcVal = i;
            else if (tag == "Tie") {
                  _tieFor = new Tie(score());
                  _tieFor->setStaff(staff());
                  _tieFor->read(e);
                  _tieFor->setStartNote(this);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setStaff(staff());
                  f->read(e);
                  f->setParent(this);
                  _fingering.append(f);
                  }
            else if (tag == "head")
                  _headGroup = i;
            else if (tag == "userAccidental")
                  _userAccidental = i;
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  _accidental->setParent(this);
                  }
            else if (tag == "move")
                  _move = i;
            else if (Element::readProperties(e))
                  ;
            else
                  domError(e);
            }
      if (tpcVal != -100)
            _tpc = tpcVal;
      else {
            _tpc = pitch2tpc(_pitch);
            }
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Note::drag(const QPointF& s)
      {
      QRectF bb(chord()->bbox());
      _lineOffset = lrint(s.y() * 2 / _spatium);
      return bb.translated(chord()->canvasPos());
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
      setPitch(line2pitch(_line, clef));
      }

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
   : Element(s)
      {
      _line = 1000;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(QPainter& p)
      {
      if (!visible())
            return;

      QPointF ap(canvasPos());

      QRect r(abbox().toRect());
//      QRect c(p.clipRect());

//      if (c.intersects(r)) {
            p.translate(ap);
            qreal lw = point(score()->style()->ledgerLineWidth);
            QPen pen(preferences.selectColor[score()->padState()->voice].light(160));
            pen.setWidthF(lw);
            p.setPen(pen);

            symbols[quartheadSym].draw(p);

            double x1 = symbols[quartheadSym].width()/2 - _spatium;
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
//            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF ShadowNote::bbox() const
      {
      QRectF b = symbols[quartheadSym].bbox();
      double x  = b.width()/2 - _spatium;
      double lw = point(score()->style()->ledgerLineWidth);

      QRectF r(0, -lw/2.0, 2 * _spatium, lw);

      for (int i = -2; i >= _line; i -= 2)
            b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
      for (int i = 10; i <= _line; i += 2)
            b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
      return b;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(Viewer* viewer, const QPointF&, int type, int) const
      {
      if (type == ATTRIBUTE || type == TEXT || type == ACCIDENTAL
         || type == BREATH || type == ARPEGGIO || type == NOTEHEAD
         || type == TREMOLO || type == IMAGE) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Note::drop(const QPointF&, const QPointF&, Element* e)
      {
      switch(e->type()) {
            case ATTRIBUTE:
                  {
                  NoteAttribute* atr = (NoteAttribute*)e;
                  Chord* cr = chord();
                  NoteAttribute* oa = cr->hasAttribute(atr);
                  if (oa) {
                        delete atr;
                        atr = 0;
                        // if attribute is already there, remove
                        // score()->cmdRemove(oa); // unexpected behaviour?
                        score()->select(oa, 0, 0);
                        }
                  else {
                        atr->setParent(cr);
                        atr->setStaff(staff());
                        score()->select(atr, 0, 0);
                        score()->cmdAdd(atr);
                        }
                  return atr;
                  }
            case TEXT:
                  {
                  Text* f = (Text*)e;
                  //
                  // override palette settings for text:
                  //
                  // QString s(f->getText());
                  // f->setSubtype(TEXT_FINGERING);
                  // f->setText(s);

                  f->setParent(this);
                  score()->select(f, 0, 0);
                  score()->undoAddElement(f);
                  return f;
                  }
            case ACCIDENTAL:
                  {
                  Accidental* a = (Accidental*)e;
                  int subtype = a->subtype();
                  delete a;
                  score()->addAccidental(this, subtype);
                  }
                  break;
            case BREATH:
                  {
                  Breath* b = (Breath*)e;
                  int tick   = chord()->tick();
                  b->setStaff(staff());
                  Measure* m = chord()->segment()->measure();

                  // TODO: insert automatically in all staves?

                  Segment* seg = m->findSegment(Segment::SegBreath, tick);
                  if (seg == 0) {
                        seg = m->createSegment(Segment::SegBreath, tick);
                        score()->undoAddElement(seg);
                        }
                  b->setParent(seg);
                  score()->undoAddElement(b);
                  }
                  break;
            case ARPEGGIO:
                  {
                  Arpeggio* a = (Arpeggio*)e;
                  a->setParent(chord());
                  a->setHeight(_spatium * 5);   //DEBUG
                  score()->undoAddElement(a);
                  }
                  break;
            case NOTEHEAD:
                  {
                  Symbol* s = (Symbol*)e;
                  switch(s->sym()) {
                        case quartheadSym:    _headGroup = 0; break;
                        case crossedheadSym:  _headGroup = 1; break;
                        case diamondheadSym:  _headGroup = 2; break;
                        case triangleheadSym: _headGroup = 3; break;
                        default: printf("unknown note head\n"); break;
                        }
                  delete s;
                  }
                  break;
            case TREMOLO:
                  {
                  Tremolo* tremolo = (Tremolo*)e;
                  tremolo->setParent(chord());
                  score()->undoAddElement(tremolo);
                  }
                  break;

            default:
                  delete e;
                  break;
            }
      return 0;
      }
