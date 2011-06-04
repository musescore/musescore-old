//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: note.cpp 3662 2010-11-02 10:35:45Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include <math.h>

#include "note.h"
#include "score.h"
#include "key.h"
#include "chord.h"
#include "sym.h"
#include "al/xml.h"
#include "slur.h"
#include "text.h"
#include "clef.h"
#include "preferences.h"
#include "staff.h"
#include "pitchspelling.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "utils.h"
#include "image.h"
#include "system.h"
#include "tuplet.h"
#include "articulation.h"
#include "drumset.h"
#include "segment.h"
#include "measure.h"
#include "part.h"
#include "tablature.h"
#include "fret.h"
#include "harmony.h"
#include "fingering.h"
#include "bend.h"
#include "painter.h"

//---------------------------------------------------------
//   noteHeads
//    note head groups
//---------------------------------------------------------

const int noteHeads[2][HEAD_GROUPS][4] = {
      {     // down stem
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadSym        },
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,    wholecrossedheadSym  },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,    wholediamondheadSym  },
      { s0triangleHeadSym,    d1triangleHeadSym,   d2triangleHeadSym, s0triangleHeadSym    },
      { s0miHeadSym,          s1miHeadSym,         s2miHeadSym,       -1                   },
      { wholeslashheadSym,    halfslashheadSym,    quartslashheadSym, wholeslashheadSym    },
      { xcircledheadSym,      xcircledheadSym,     xcircledheadSym,   xcircledheadSym      },
      { s0doHeadSym,          d1doHeadSym,         d2doHeadSym,       -1                   },
      { s0reHeadSym,          d1reHeadSym,         d2reHeadSym,       -1                   },
      { d0faHeadSym,          d1faHeadSym,         d2faHeadSym,       -1                   },
      { s0laHeadSym,          s1laHeadSym,         s2laHeadSym,       -1                   },
      { s0tiHeadSym,          d1tiHeadSym,         d2tiHeadSym,       -1                   },
      },
      {     // up stem
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadSym        },
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,    wholecrossedheadSym  },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,    wholediamondheadSym  },
      { s0triangleHeadSym,    u1triangleHeadSym,   u2triangleHeadSym, s0triangleHeadSym    },
      { s0miHeadSym,          s1miHeadSym,         s2miHeadSym,       -1                   },
      { wholeslashheadSym,    halfslashheadSym,    quartslashheadSym, wholeslashheadSym    },
      { xcircledheadSym,      xcircledheadSym,     xcircledheadSym,   xcircledheadSym      },
      { s0doHeadSym,          u1doHeadSym,         u2doHeadSym,       -1                   },
      { s0reHeadSym,          u1reHeadSym,         u2reHeadSym,       -1                   },
      { u0faHeadSym,          u1faHeadSym,         u2faHeadSym,       -1                   },
      { s0laHeadSym,          s1laHeadSym,         s2laHeadSym,       -1                   },
      { s0tiHeadSym,          u1tiHeadSym,         u2tiHeadSym,       -1                   },
      }
      };

//---------------------------------------------------------
//   noteHeadSym
//---------------------------------------------------------

Sym* noteHeadSym(bool up, int group, int n)
      {
      return &symbols[0][noteHeads[up][group][n]];
      }

//---------------------------------------------------------
//   NoteDot
//---------------------------------------------------------

NoteDot::NoteDot(Score* s)
   : Symbol(s, dotSym)
      {
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      dragMode           = false;
      _pitch             = 0;
      _ppitch            = 0;
      _tuning            = 0.0;
      _accidental        = 0;
      _mirror            = false;
      _userMirror        = DH_AUTO;
      _line              = 0;
      _fret              = -1;
      _string            = -1;
      _ghost             = false;
      _lineOffset        = 0;
      _tieFor            = 0;
      _tieBack           = 0;
      _tpc               = -1;
      _headGroup         = 0;
      _headType          = HEAD_AUTO;

      _hidden            = false;
      _subchannel        = 0;

      _veloType          = AUTO_VAL;
      _veloOffset        = 0;

      _onTimeOffset      = 0;
      _onTimeUserOffset  = 0;

      _offTimeOffset     = 0;
      _offTimeUserOffset = 0;
      _bend              = 0;
      _dots[0]           = 0;
      _dots[1]           = 0;
      _dots[2]           = 0;
      }

Note::~Note()
      {
      delete _accidental;
      foreach(Element* e, _el)
            delete e;
      delete _tieFor;
      delete _bend;
      delete _dots[0];
      delete _dots[1];
      delete _dots[2];
      }

Note::Note(const Note& n)
   : Element(n)
      {
      _subchannel        = n._subchannel;
      _line              = n._line;
      _fret              = n._fret;
      _string            = n._string;
      _ghost             = n._ghost;
      dragMode           = n.dragMode;
      _pitch             = n._pitch;
      _tpc               = n._tpc;
      _ppitch            = n._ppitch;
      _hidden            = n._hidden;
      _tuning            = n._tuning;
      _veloType          = n._veloType;
      _veloOffset        = n._veloOffset;
      _onTimeOffset      = n._onTimeOffset;
      _onTimeUserOffset  = n._onTimeUserOffset;
      _offTimeOffset     = n._offTimeOffset;
      _offTimeUserOffset = n._offTimeUserOffset;
      _headGroup         = n._headGroup;
      _headType          = n._headType;
      _mirror            = n._mirror;
      _userMirror        = n._userMirror;
      _accidental        = 0;
      if (n._accidental)
            add(new Accidental(*(n._accidental)));

      foreach(Element* e, n._el)
            add(e->clone());

      _tieFor            = 0;
      _tieBack           = 0;
      _bend              = 0;
      if (n._bend)
            add(new Bend(*n._bend));
      for (int i = 0; i < 3; ++i) {
            _dots[i] = 0;
            if (n._dots[i]) {
                  _dots[i] = new NoteDot(*n._dots[i]);
                  _dots[i]->setIdx(i);
                  }
            }

      _lineOffset = n._lineOffset;
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void Note::setPitch(int val)
      {
      if (val > 127)
            val = 127;
      else if (val < 0)
            val = 0;
      _pitch = val;
      int pitchOffset = 0;
      if (score()) {
            Part* part = score()->part(staffIdx());
            if (part)
                  pitchOffset = score()->styleB(ST_concertPitch) ? 0 : part->instr()->transpose().chromatic;
            }
      _ppitch = _pitch + pitchOffset;
      if (chord())
            chord()->pitchChanged();
      }

void Note::setPitch(int a, int b)
      {
      setPitch(a);
      _tpc = b;
      }

//---------------------------------------------------------
//   setTpcFromPitch
//---------------------------------------------------------

void Note::setTpcFromPitch()
      {
      KeySigEvent key = (staff() && chord()) ? staff()->key(chord()->tick()) : KeySigEvent();
      _tpc    = pitch2tpc(_pitch, key.accidentalType());
// printf("setTpcFromPitch pitch %d tick %d key %d tpc %d\n", pitch(), chord()->tick(), key.accidentalType(), _tpc);
      }

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

void Note::setTpc(int v)
      {
      if (v < -1 || v > 33) {
            return;
            }
      _tpc = v;
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

int Note::noteHead() const
      {
      switch(_headType) {
            default:
            case HEAD_AUTO:
                  return noteHeads[chord()->up()][int(_headGroup)][chord()->durationType().headType()];
            case HEAD_WHOLE:
                  return noteHeads[chord()->up()][int(_headGroup)][0];
            case HEAD_HALF:
                  return noteHeads[chord()->up()][int(_headGroup)][1];
            case HEAD_QUARTER:
                  return noteHeads[chord()->up()][int(_headGroup)][2];
            case HEAD_BREVIS:
                  return noteHeads[chord()->up()][int(_headGroup)][3];
            }
      }

//---------------------------------------------------------
//   headWidth
//---------------------------------------------------------

qreal Note::headWidth() const
      {
      return symbols[score()->symIdx()][noteHead()].width(magS());
      }

//---------------------------------------------------------
//   headHeight
//---------------------------------------------------------

qreal Note::headHeight() const
      {
      return symbols[score()->symIdx()][noteHead()].height(magS());
      }

//---------------------------------------------------------
//   playTicks
//---------------------------------------------------------

/**
 Return total tick len of tied notes
*/

int Note::playTicks() const
      {
      const Note* note = this;
      while (note->tieBack())
            note = note->tieBack()->startNote();
      int len = 0;
      while (note->tieFor() && note->tieFor()->endNote()) {
            len += note->chord()->ticks();
            note = note->tieFor()->endNote();
            }
      len += note->chord()->ticks();
      return len;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* e)
      {
	e->setParent(this);
      switch(e->type()) {
            case NOTEDOT:
                  {
                  NoteDot* dot = static_cast<NoteDot*>(e);
                  _dots[dot->idx()] = dot;
                  }
                  break;
            case SYMBOL:
            case IMAGE:
            case FINGERING:
            case TEXT:
                  _el.append(e);
                  break;
            case TIE:
                  {
                  Tie* tie = static_cast<Tie*>(e);
	      	tie->setStartNote(this);
                  tie->setTrack(track());
      		setTieFor(tie);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(tie);
                  }
                  break;
            case ACCIDENTAL:
                  _accidental = static_cast<Accidental*>(e);
                  break;
            case BEND:
                  _bend = static_cast<Bend*>(e);
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Note::remove(Element* e)
      {
      switch(e->type()) {
            case NOTEDOT:
                  for (int i = 0; i < 3; ++i) {
                        if (_dots[i] == e) {
                              _dots[i] = 0;
                              break;
                              }
                        }
                  break;

            case TEXT:
            case SYMBOL:
            case IMAGE:
            case FINGERING:
                  if (!_el.remove(e)) {
                        ; // printf("Note::remove(): cannot find %s\n", e->name());
                        }
                  break;
	      case TIE:
                  {
                  Tie* tie = static_cast<Tie*>(e);
                  setTieFor(0);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(0);
                  }
                  break;

            case ACCIDENTAL:
                  _accidental = 0;
                  break;

            case BEND:
                  _bend = 0;
                  break;

            default:
                  printf("Note::remove() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   stemPos
//    return in canvas coordinates
//---------------------------------------------------------

QPointF Note::stemPos(bool upFlag) const
      {
      QPointF pt(pos());
      if (chord()->staffMove()) {
            System* system = chord()->measure()->system();
            pt.ry() += system->staff(staffIdx() + chord()->staffMove())->y() - system->staff(staffIdx())->y();
            }
      if (_mirror)
            upFlag = !upFlag;

      qreal sw   = point(score()->styleS(ST_stemWidth)) * .5;
      if (chord() && chord()->staff() && chord()->staff()->useTablature()) {
            qreal xoffset = (sw + _bbox.width() + _bbox.x()) * .5;
            pt += QPointF(xoffset, (_bbox.height() * .5 + spatium() * .5) * (upFlag ? -1.0 : 1.0));
            }
      else {
            QPointF off = symbols[score()->symIdx()][noteHead()].attach(magS());
            if (upFlag) {
                  pt.rx() += off.x() - sw;
                  pt.ry() += off.y();
                  }
            else {
                  pt.rx() += symbols[score()->symIdx()][noteHead()].width(magS()) - off.x() + sw;
                  pt.ry() -= off.y();
                  }
            }
      return pt + chord()->canvasPos();
      }

//---------------------------------------------------------
//   stemYoff
//---------------------------------------------------------

qreal Note::stemYoff(bool upFlag) const
      {
      if (_mirror)
            upFlag = !upFlag;
      //
      // TODO: implement table for all note heads
      //
      qreal yo = .2 * mag();
      if (_headGroup == 5)
            yo = 1.0 * mag();
      return upFlag ? -yo : yo;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(Painter* p) const
      {
      bool tablature = staff() && staff()->useTablature();
      if (!_hidden || !userOff().isNull()) {
            if (tablature) {
#if 0
                  if (tieBack())
                        return;
                  qreal mag = magS();
                  Font f("FreeSans");
                  int size = lrint(9.0 * DPI / PPI);
                  f.setPixelSize(size);
                  qreal imag = 1.0 / mag;
                  p->scale(mag, mag);
                  p->setFont(f);

                  QString s = _ghost ? "X" : QString::number(_fret);
                  qreal d  = spatium() * .2;
                  QRectF bb = bbox().adjusted(-d, 0.0, d, 0.0);
                  if (v)
                        v->drawBackground(p, bb);
                  else
                        p.eraseRect(bb);
                  p->drawText(0.0, bbox().height() * .5, s);
                  p->scale(imag, imag);
#endif
                  }
            else {
                  //
                  // warn if pitch extends usable range of instrument
                  // by coloring the note head
                  //
#if 0
                  if (chord() && chord()->segment() && staff() && !selected() && !score()->printing() && preferences.warnPitchRange) {
                        const Instrument* in = staff()->part()->instr();
                        int i = ppitch();
                        if (i < in->minPitchP() || i > in->maxPitchP())
                              p->setPen(Qt::red);
                        else if (i < in->minPitchA() || i > in->maxPitchA())
                              p->setPen(Qt::darkYellow);
                        }
#endif
                  symbols[score()->symIdx()][noteHead()].draw(p, magS());
                  }
            }
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(XmlReader* r)
      {
      char tpcVal = -100;
      while (r->readAttribute()) {
            if (r->tag() == "pitch")
                  _ppitch = _pitch  = r->intValue();
            else if (r->tag() == "tcp")
                  tpcVal = r->intValue();
            }
      QString val;
      int i;
      while (r->readElement()) {
            MString8 tag(r->tag());
            if (r->readInt("pitch", &i)) {
                  if (i > 127)
                        i = 127;
                  else if (i < 0)
                        i = 0;
                  _pitch  = i;
                  _ppitch = i;
                  }
            else if (r->readChar("fret", &_fret))
                  ;
            else if (r->readChar("string", &_string))
                  ;
            else if (r->readBool("ghost", &_ghost))
                  ;
            else if (r->readChar("line", &_line))
                  ;
            else if (r->readReal("tuning", &_tuning))
                  ;
            else if (r->readChar("tpc", &tpcVal))
                  ;
            else if (tag == "Tie") {
                  _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(r);
                  _tieFor->setStartNote(this);
                  }
            else if (tag == "Fingering") {
                  Fingering* f = new Fingering(score());
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(r);
                  add(f);
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTrack(track());
                  s->read(r);
                  add(s);
                  }
#if 0
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  XmlReader* ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        printf("unknown image format <%s>\n", qPrintable(path));
                        }
                  if (image) {
                        image->setTrack(track());
                        image->read(e);
                        add(image);
                        }
                  }
#endif
            else if (r->readChar("head", &_headGroup))
                  ;
            else if (r->readInt("headType", &i))
                  _headType = NoteHeadType(i);
            else if (tag == "Accidental") {
                  Accidental* a = new Accidental(score());
                  a->read(r);
                  add(a);
                  }
            else if (r->readInt("mirror", &i))
                  _userMirror = DirectionH(i);
//            else if (tag == "veloType")
//                  ;     // _veloType = readValueType(e);
            else if (r->readInt("velocity", &_veloOffset))
                  ;
            else if (tag == "Bend") {
                  _bend = new Bend(score());
                  _bend->setTrack(track());
                  _bend->read(r);
                  _bend->setParent(this);
                  }
            else if (tag == "NoteDot") {
                  NoteDot* dot = new NoteDot(score());
                  dot->setParent(this);
                  dot->read(r);
                  for (int i = 0; i < 3; ++i) {
                        if (_dots[i] == 0) {
                              _dots[i] = dot;
                              dot->setIdx(i);
                              dot = 0;
                              break;
                              }
                        }
                  if (dot) {
                        printf("too many dots\n");
                        delete dot;
                        }
                  }
            else if (r->readInt("onTimeOffset", &_onTimeUserOffset))
                  ;
            else if (r->readInt("offTimeOffset", &_offTimeUserOffset))
                  ;
            else if (Element::readProperties(r))
                  ;
            else
                  r->unknown();
            }
      if (tpcVal != -100)
            _tpc = tpcVal;
      else {
            _tpc = pitch2tpc(_pitch);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Note::layout()
      {
      if (staff() && staff()->useTablature()) {
            Font f("FreeSerif");
            f.setSize(9.0 * DPI / PPI);
            FontMetricsF fm(f);
            qreal mag = magS();
            QString s  = QString::number(_fret);
            QRectF bb(fm.tightBoundingRect(s));
            bb = bb.translated(0.0, bb.height() * .5);
            _bbox = QRectF(bb.x() * mag, bb.y() * mag, bb.width() * mag, bb.height() * mag);
            }
      else
            _bbox = symbols[score()->symIdx()][noteHead()].bbox(magS());
      if (parent() == 0)
            return;
      foreach (Element* e, _el) {
            e->setMag(mag());
            e->layout();
            if (e->type() == SYMBOL && static_cast<Symbol*>(e)->sym() == rightparenSym)
                  e->setPos(headWidth(), 0.0);
            }
      if (_bend)
            _bend->layout();
      int dots = chord()->dots();
      for (int i = 0; i < 3; ++i) {
            if (i < dots) {
                  if (_dots[i] == 0) {
                        _dots[i] = new NoteDot(score());
                        _dots[i]->setIdx(i);
                        _dots[i]->setParent(this);
                        }
                  _dots[i]->layout();
                  }
            else {
                  delete _dots[i];
                  _dots[i] = 0;
                  }
            }
      if (dots) {
            qreal _spatium = spatium();
            qreal d  = point(score()->styleS(ST_dotNoteDistance));
            qreal dd = point(score()->styleS(ST_dotDotDistance));
            qreal y  = 0;
            qreal x  = chord()->dotPosX() - pos().x();

            // do not draw dots on staff line
            if ((_line & 1) == 0) {
                  Measure* m = chord()->measure();
                  if (m->mstaff(staffIdx())->hasVoices) {
                        if (voice() == 0 || voice() == 2)
                              y = -_spatium *.5;
                        else
                              y = _spatium *.5;
                        }
                  else
                        y = -_spatium *.5;
                  }
            for (int i = 0; i < dots; ++i)
                  _dots[i]->setPos(x + d + dd * i, y);
            }
      }

//---------------------------------------------------------
//   layout10
//    compute actual accidental and line
//---------------------------------------------------------

void Note::layout10(char* tversatz)
      {
      if (staff()->useTablature()) {
            if (_accidental) {
                  delete _accidental;
                  _accidental = 0;
                  }
            if (_fret < 0) {
                  int string, fret;
                  Tablature* tab = staff()->part()->instr()->tablature();
                  if (tab->convertPitch(_pitch, &string, &fret)) {
                        _fret   = fret;
                        _string = string;
                        }
                  }
            }
      else {
            _line          = tpc2step(_tpc) + (_pitch/12) * 7;
            int tpcPitch   = tpc2pitch(_tpc);
            if (tpcPitch < 0)
                  _line += 7;
            else
                  _line -= (tpcPitch/12)*7;

            // calculate accidental

            AccidentalType acci = ACC_NONE;
            if (_accidental && _accidental->role() == ACC_USER)
                  acci = _accidental->accidentalType();
            else  {
                  int accVal = tpc2alter(_tpc);
                  if ((accVal != tversatz[int(_line)]) || hidden()) {
                        if (_tieBack == 0)
                              tversatz[int(_line)] = accVal;
                        acci = Accidental::value2subtype(accVal);
                        if (acci == ACC_NONE)
                              acci = ACC_NATURAL;
                        }
                  }
            if (acci != ACC_NONE && !_tieBack && !_hidden) {
                  if (_accidental == 0) {
                        _accidental = new Accidental(score());
                        _accidental->setGenerated(true);
                        _accidental->setParent(this);
                        }
                  _accidental->setSubtype(acci);
                  }
            else {
                  if (_accidental) {
                        if (_accidental->selected()) {
                              score()->deselect(_accidental);
                              }
                        delete _accidental;
                        _accidental = 0;
                        }
                  }
            //
            // calculate the real note line depending on clef
            //
            Staff* s = score()->staff(staffIdx() + chord()->staffMove());
            int tick = chord()->tick();
            int clef = s->clefList()->clef(tick);
            _line    = 127 - _line - 82 + clefTable[clef].yOffset;
            }
      }

//---------------------------------------------------------
//   noteType
//---------------------------------------------------------

NoteType Note::noteType() const
      {
      return chord()->noteType();
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Note::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      Chord* ch = chord();
      if (ch == 0 || ch->parent() == 0)
            return pos();
      Measure* m = ch->measure();
      if (m == 0)
            return pos();
      System* system = m->system();
      if (system == 0)
            return pos();
      qreal yp = y() + system->staff(staffIdx() + chord()->staffMove())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Note::scanElements(void* data, void (*func)(void*, Element*))
      {
      func(data, this);
      if (_tieFor && !staff()->useTablature())  // no ties in tablature
            _tieFor->scanElements(data, func);
      foreach(Element* e, _el)
            e->scanElements(data, func);
      if (!dragMode && _accidental)
            func(data, _accidental);
      if (_bend)
            func(data, _bend);
      for (int i = 0; i < chord()->dots(); ++i) {
            if (_dots[i])
                  func(data, _dots[i]);
            }
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Note::setTrack(int val)
      {
      Element::setTrack(val);
      if (_tieFor) {
            _tieFor->setTrack(val);
            foreach(SpannerSegment* seg, _tieFor->spannerSegments())
                  seg->setTrack(val);
            }
      foreach(Element* e, _el)
            e->setTrack(val);
      if (_accidental)
            _accidental->setTrack(val);
      if (_bend)
            _bend->setTrack(val);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Note::setMag(qreal val)
      {
      Element::setMag(val);
      if (_accidental)
            _accidental->setMag(val);
      if (_bend)
            _bend->setMag(val);
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void Note::setLine(int n)
      {
      _line = n;
      rypos() = _line * spatium() * .5;
      }

//---------------------------------------------------------
//   setString
//---------------------------------------------------------

void Note::setString(int val)
      {
      _string = val;
      rypos() = _string * spatium() * 1.5;
      }

//---------------------------------------------------------
//   setHeadGroup
//---------------------------------------------------------

void Note::setHeadGroup(int val)
      {
      _headGroup = val;
      }

//---------------------------------------------------------
//   ppitch
//    playback pitch
//    honours ottava and transposing instruments
//---------------------------------------------------------

int Note::ppitch() const
      {
      int tick        = chord()->segment()->tick();
      int pitchOffset = score()->styleB(ST_concertPitch) ? 0 : staff()->part()->instr()->transpose().chromatic;
      return _pitch + staff()->pitchOffsets().pitchOffset(tick) + pitchOffset;
      }

//---------------------------------------------------------
//   customizeVelocity
//    Input is the global velocity determined by dynamic
//    signs and crescende/decrescendo etc.
//    Returns the actual play velocity for this note
//    modified by veloOffset
//---------------------------------------------------------

int Note::customizeVelocity(int velo) const
      {
      if (veloType() == OFFSET_VAL)
            velo = velo + (velo * veloOffset()) / 100;
      else if (veloType() == USER_VAL)
            velo = veloOffset();
      if (velo < 1)
            velo = 1;
      else if (velo > 127)
            velo = 127;
      return velo;
      }

//---------------------------------------------------------
//   updateLine
//---------------------------------------------------------

void Note::updateLine()
      {
      _line          = tpc2step(_tpc) + (_pitch/12) * 7;
      int tpcPitch   = tpc2pitch(_tpc);
      if (tpcPitch < 0)
            _line += 7;
      else
            _line -= (tpcPitch/12)*7;
      Staff* s = score()->staff(staffIdx() + chord()->staffMove());
      int tick = chord()->tick();
      int clef = s->clefList()->clef(tick);
      _line    = 127 - _line - 82 + clefTable[clef].yOffset;
      }


//---------------------------------------------------------
//   setNval
//---------------------------------------------------------

void Note::setNval(NoteVal nval)
      {
      setPitch(nval.pitch);
      _fret      = nval.fret;
      _string    = nval.string;
      _headGroup = nval.headGroup;
      }
