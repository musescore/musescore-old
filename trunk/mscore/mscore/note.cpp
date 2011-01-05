//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include <assert.h>

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
#include "staff.h"
#include "pitchspelling.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "chordproperties.h"
#include "icons.h"
#include "utils.h"
#include "image.h"
#include "system.h"
#include "tuplet.h"
#include "articulation.h"
#include "drumset.h"
#include "segment.h"
#include "measure.h"
#include "undo.h"
#include "part.h"
#include "stafftype.h"
#include "tablature.h"
#include "fret.h"
#include "harmony.h"
#include "fingering.h"
#include "bend.h"
#include "bend.h"
#include "scoreview.h"

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
//   write
//---------------------------------------------------------

void NoteHead::write(Xml& xml) const
      {
      xml.stag("NoteHead");
      xml.tag("name", symbols[0][_sym].name());
      Element::writeProperties(xml);
      xml.etag();
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
      _fretConflict      = false;
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
      _fretConflict      = n._fretConflict;
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
            printf("Note::setTpc: bad tpc %d\n", v);
            abort();
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

double Note::headWidth() const
      {
      return symbols[score()->symIdx()][noteHead()].width(magS());
      }

//---------------------------------------------------------
//   headHeight
//---------------------------------------------------------

double Note::headHeight() const
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
                  printf("Note::add() not impl. %s\n", e->name());
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
                  if (!_el.remove(e))
                        printf("Note::remove(): cannot find %s\n", e->name());
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

      double sw   = point(score()->styleS(ST_stemWidth)) * .5;
      if (chord() && chord()->staff() && chord()->staff()->useTablature()) {
            double xoffset = (sw + _bbox.width() + _bbox.x()) * .5;
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

double Note::stemYoff(bool upFlag) const
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

void Note::draw(QPainter& p, ScoreView* v) const
      {
      bool tablature = staff() && staff()->useTablature();
      if (!_hidden || !userOff().isNull()) {
            if (tablature) {
                  if (tieBack())
                        return;
                  StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
                  double mag = magS();
                  double imag = 1.0 / mag;
                  double currSpatium = spatium();

                  p.scale(mag, mag);
                  p.setFont(tab->fretFont());

                  // when using letters, "+(_fret > 8)" skips 'j'
                  QString s = _ghost ? "X" :
                          ( tab->useNumbers() ? QString::number(_fret) : QString('a' + _fret + (_fret > 8)) );
                  double d  = currSpatium * .2;
                  // draw background, if required
                  if(!tab->linesThrough() || !tab->slashStyle() || fretConflict()) {
                        QRectF bb = bbox().adjusted(-d, 0.0, d, 0.0);
                        if (v) {
                              v->drawBackground(p, bb);
                              }
                        else
                              p.eraseRect(bb);
                        if(fretConflict()) {          //on fret conflict, draw on red background
                              QPen oldPen = p.pen();
                              p.setPen(Qt::red);
                              p.setBrush(Qt::red);
                              p.drawRect(bb);
                              p.setPen(oldPen);
                              }
                        }
                  p.drawText(_bbox.x(), tab->fretFontYOffset() * mag, s);
                  p.scale(imag, imag);
                  }
            else {                        // if not tablature
                  //
                  // warn if pitch extends usable range of instrument
                  // by coloring the note head
                  //
                  if (chord() && chord()->segment() && staff() && !selected() && !score()->printing() && preferences.warnPitchRange) {
                        const Instrument* in = staff()->part()->instr();
                        int i = ppitch();
                        if (i < in->minPitchP() || i > in->maxPitchP())
                              p.setPen(Qt::red);
                        else if (i < in->minPitchA() || i > in->maxPitchA())
                              p.setPen(Qt::darkYellow);
                        }
                  symbols[score()->symIdx()][noteHead()].draw(p, magS());
                  }
            }

/*      if (chord() && !tablature) {
            int dots = chord()->dots();
            for (int i = 0; i < dots; ++i)
                  _dots[i]->draw(p, v);
            }
      */
      }

//--------------------------------------------------
//   Note::write
//---------------------------------------------------------

void Note::write(Xml& xml, int /*startTick*/, int endTick) const
      {
      xml.stag("Note");
      QList<Prop> pl = Element::properties(xml);
      xml.prop(pl);
      //
      // get real pitch for clipboard (copy/paste)
      //
      int rpitch = pitch();
      int rtpc   = tpc();

      if (xml.clipboardmode && !score()->styleB(ST_concertPitch)) {
            Part* part = staff()->part();
            if (part->instr()->transpose().chromatic)
                  transposeInterval(pitch(), tpc(), &rpitch, &rtpc, part->instr()->transpose(), true);
            }

      xml.tag("pitch", rpitch);
      xml.tag("tpc", rtpc);
      if (_fret >= 0) {
            xml.tag("fret", _fret);
            xml.tag("string", _string);
            }
      if (_ghost)
            xml.tag("ghost", _ghost);

      if (_tuning != 0.0)
            xml.tag("tuning", _tuning);

      if (_accidental)
            _accidental->write(xml);
      _el.write(xml);
      int dots = chord()->dots();
      bool hasUserModifiedDots = false;
      for (int i = 0; i < dots; ++i) {
            if (!_dots[i]->userOff().isNull()) {
                  hasUserModifiedDots = true;
                  break;
                  }
            }
      if (hasUserModifiedDots) {
            for (int i = 0; i < dots; ++i)
                  _dots[i]->write(xml);
            }

      if (_tieFor) {
            // in clipboardmode write tie only if the next note is < endTick
            if (!xml.clipboardmode || _tieFor->endNote()->chord()->tick() < endTick)
                  _tieFor->write(xml);
            }
      if (_headGroup != 0)
            xml.tag("head", _headGroup);
      if (_headType != HEAD_AUTO)
            xml.tag("headType", _headType);
      if (_userMirror != DH_AUTO)
            xml.tag("mirror", _userMirror);
      if (_veloType != AUTO_VAL) {
            xml.valueTypeTag("veloType", _veloType);
            xml.tag("velocity", _veloOffset);
            }
      if (_onTimeUserOffset)
            xml.tag("onTimeOffset", _onTimeUserOffset);
      if (_offTimeUserOffset)
            xml.tag("offTimeOffset", _offTimeUserOffset);
      if (_bend)
            _bend->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(QDomElement e)
      {
      int ptch = e.attribute("pitch", "-1").toInt();
      if (ptch != -1) {
            _pitch = ptch;
            _ppitch = ptch;
            }
      int tpcVal = e.attribute("tpc", "-100").toInt();

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "pitch") {
                  if (i > 127)
                        i = 127;
                  else if (i < 0)
                        i = 0;
                  _pitch  = i;
                  _ppitch = i;
                  }
            else if (tag == "fret")
                  _fret = i;
            else if (tag == "string")
                  _string = i;
            else if (tag == "ghost")
                  _ghost = i;
            else if (tag == "line")
                  _line = i;
            else if (tag == "tuning")
                  _tuning = val.toDouble();
            else if (tag == "tpc")
                  tpcVal = i;
            else if (tag == "Tie") {
                  _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(this);
                  }
            else if (tag == "Text") {                       // obsolete
                  Fingering* f = new Fingering(score());
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(e);
                  add(f);
                  }
            else if (tag == "Fingering") {
                  Fingering* f = new Fingering(score());
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(e);
                  add(f);
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTrack(track());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
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
            else if (tag == "head")
                  _headGroup = i;
            else if (tag == "headType")
                  _headType = NoteHeadType(i);
            else if (tag == "userAccidental") {
                  bool ok;
                  int k = val.toInt(&ok);
                  if (ok) {
                        _accidental = new Accidental(score());
                        _accidental->setParent(this);
                        // TODO: for backward compatibility
                        bool bracket = k & 0x8000;
                        k &= 0xfff;
                        AccidentalType at = ACC_NONE;
                        switch(k) {
                              case 0: at = ACC_NONE; break;
                              case 1:
                              case 11: at = ACC_SHARP; break;
                              case 2:
                              case 12: at = ACC_FLAT; break;
                              case 3:
                              case 13: at = ACC_SHARP2; break;
                              case 4:
                              case 14: at = ACC_FLAT2; break;
                              case 5:
                              case 15: at = ACC_NATURAL; break;

                              case 6:  at = ACC_SHARP; bracket = true; break;
                              case 7:  at = ACC_FLAT; bracket = true; break;
                              case 8:  at = ACC_SHARP2; bracket = true; break;
                              case 9:  at = ACC_FLAT2; bracket = true; break;
                              case 10: at = ACC_NATURAL; bracket = true; break;

                              case 16: at = ACC_FLAT_SLASH; break;
                              case 17: at = ACC_FLAT_SLASH2; break;
                              case 18: at = ACC_MIRRORED_FLAT2; break;
                              case 19: at = ACC_MIRRORED_FLAT; break;
                              case 20: at = ACC_MIRRIRED_FLAT_SLASH; break;
                              case 21: at = ACC_FLAT_FLAT_SLASH; break;

                              case 22: at = ACC_SHARP_SLASH; break;
                              case 23: at = ACC_SHARP_SLASH2; break;
                              case 24: at = ACC_SHARP_SLASH3; break;
                              case 25: at = ACC_SHARP_SLASH4; break;

                              case 26: at = ACC_SHARP_ARROW_UP; break;
                              case 27: at = ACC_SHARP_ARROW_DOWN; break;
                              case 28: at = ACC_SHARP_ARROW_BOTH; break;
                              case 29: at = ACC_FLAT_ARROW_UP; break;
                              case 30: at = ACC_FLAT_ARROW_DOWN; break;
                              case 31: at = ACC_FLAT_ARROW_BOTH; break;
                              case 32: at = ACC_NATURAL_ARROW_UP; break;
                              case 33: at = ACC_NATURAL_ARROW_DOWN; break;
                              case 34: at = ACC_NATURAL_ARROW_BOTH; break;
                              }
                        _accidental->setSubtype(at);
                        _accidental->setHasBracket(bracket);
                        _accidental->setRole(ACC_USER);
                        }
                  }
            else if (tag == "Accidental") {
                  Accidental* a = new Accidental(score());
                  a->read(e);
                  add(a);
                  }
            else if (tag == "move")             // obsolete
                  chord()->setStaffMove(i);
            else if (tag == "mirror")
                  _userMirror = DirectionH(i);
            else if (tag == "veloType")
                  _veloType = readValueType(e);
            else if (tag == "velocity")
                  _veloOffset = i;
            else if (tag == "Bend") {
                  _bend = new Bend(score());
                  _bend->setTrack(track());
                  _bend->read(e);
                  _bend->setParent(this);
                  }
            else if (tag == "NoteDot") {
                  NoteDot* dot = new NoteDot(score());
                  dot->setParent(this);
                  dot->read(e);
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
            else if (tag == "onTimeType")                   // obsolete
                  ; // _onTimeType = readValueType(e);
            else if (tag == "onTimeOffset")
                  _onTimeUserOffset = i;
            else if (tag == "offTimeType")                  // obsolete
                  ; // _offTimeType = readValueType(e);
            else if (tag == "offTimeOffset")
                  _offTimeUserOffset = i;
            else if (tag == "tick")                   // bad input file
                  ;
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
      dragMode = true;
      QRectF bb(chord()->bbox());

      double _spatium = spatium();
      bool tab = staff()->useTablature();
      double step = tab ? _spatium * 1.5 : _spatium * .5;
      _lineOffset = lrint(s.y() / step);
      if (tab) {
            int strings = staff()->lines();
            if (_line + _lineOffset < 0)
                  _lineOffset = -_line;
            else if (_line + _lineOffset >= strings)
                  _lineOffset = strings - _line - 1;
            }
      score()->setLayout(chord()->measure());
      return bb.translated(chord()->canvasPos());
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Note::endDrag()
      {
      if (_lineOffset == 0)
            return;
      int nLine    = _line + _lineOffset;
      _lineOffset  = 0;
      dragMode     = false;
      int staffIdx = chord()->staffIdx() + chord()->staffMove();
      Staff* staff = score()->staff(staffIdx);
      int npitch;
      int tpc;
      int nString = _string;
      int nFret   = _fret;
      if (staff->useTablature()) {
            nString = nLine;
            npitch = staff->part()->instr()->tablature()->getPitch(_string, _fret);
            tpc    = pitch2tpc(npitch, 0);
            }
      else {
            int tick = chord()->tick();
            int clef = staff->clef(tick);
            int key  = staff->key(tick).accidentalType();
            npitch   = line2pitch(nLine, clef, key);
            tpc      = pitch2tpc(npitch, key);
            nString  = -1;
            nFret    = -1;
            }
      Note* n = this;
      while (n->tieBack())
            n = n->tieBack()->startNote();
      for (; n; n = n->tieFor() ? n->tieFor()->endNote() : 0)
            score()->undoChangePitch(n, npitch, tpc, nLine, nFret, nString);
      score()->select(this, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(ScoreView*, const QPointF&, int type, int subtype) const
      {
      return (type == ARTICULATION
         || type == CHORDLINE
         || type == TEXT
         || type == FINGERING
         || type == ACCIDENTAL
         || type == BREATH
         || type == ARPEGGIO
         || type == NOTEHEAD
         || type == NOTE
         || type == TREMOLO
         || type == STAFF_STATE
         || type == INSTRUMENT_CHANGE
         || type == IMAGE
         || type == CHORD
         || type == HARMONY
         || type == DYNAMIC
         || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_ACCIACCATURA)
         || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_APPOGGIATURA)
	   || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_GRACE4)
	   || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_GRACE16)
	   || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_GRACE32)
         || (type == ICON && subtype == ICON_SBEAM)
         || (type == ICON && subtype == ICON_MBEAM)
         || (type == ICON && subtype == ICON_NBEAM)
         || (type == ICON && subtype == ICON_BEAM32)
         || (type == ICON && subtype == ICON_BEAM64)
         || (type == ICON && subtype == ICON_AUTOBEAM)
         || (type == SYMBOL)
         || (type == CLEF)
         || (type == BAR_LINE)
         || (type == GLISSANDO)
         || (type == SLUR)
         || (type == STAFF_TEXT)
         || (type == TEMPO_TEXT)
         || (type == BEND && (staff()->useTablature())));
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Note::drop(ScoreView* view, const QPointF& p1, const QPointF& p2, Element* e)
      {
      Chord* ch = chord();
      switch(e->type()) {
            case TEXT:
                  if (e->subtype() == TEXT_REHEARSAL_MARK)
                        return ch->drop(view, p1, p2, e);

            case SYMBOL:
            case IMAGE:
            case FINGERING:
                  e->setParent(this);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;

            case SLUR:
                  delete e;
                  view->cmdAddSlur(this, 0);
                  return 0;

            case LYRICS:
                  e->setParent(ch->segment());
                  e->setTrack((track() / VOICES) * VOICES);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;

            case ACCIDENTAL:
                  score()->changeAccidental(this, AccidentalType(e->subtype()));
                  if (_accidental)
                        score()->select(_accidental);
                  break;

            case ARPEGGIO:
                  {
                  Arpeggio* a = static_cast<Arpeggio*>(e);
                  a->setParent(ch);
                  a->setHeight(spatium() * 5);   //DEBUG
                  score()->undoAddElement(a);
                  }
                  break;

            case BEND:
                  {
                  Bend* b = static_cast<Bend*>(e);
                  b->setParent(this);
                  b->setTrack(track());
                  score()->undoAddElement(b);
                  }
                  break;

            case NOTEHEAD:
                  {
                  Symbol* s = (Symbol*)e;
                  int group = -1;

                  for (int i = 0; i < HEAD_GROUPS; ++i) {
                        if (noteHeads[0][i][1] == s->sym()) {
                              group = i;
                              break;
                              }
                        }
                  if (group == -1) {
                        printf("unknown note head\n");
                        group = 0;
                        }
                  delete s;
                  if (group != _headGroup) {
                        score()->undo()->push(new ChangeNoteHead(this, group, _headType));
                        score()->select(this);
                        }
                  }
                  break;

            case ICON:
                  {
                  switch(e->subtype()) {
                        case ICON_ACCIACCATURA:
                              score()->setGraceNote(ch, pitch(), NOTE_ACCIACCATURA, AL::division/2);
                              break;
                        case ICON_APPOGGIATURA:
                              score()->setGraceNote(ch, pitch(), NOTE_APPOGGIATURA, AL::division/2);
                              break;
                        case ICON_GRACE4:
                              score()->setGraceNote(ch, pitch(), NOTE_GRACE4, AL::division);
                              break;
                        case ICON_GRACE16:
                              score()->setGraceNote(ch, pitch(), NOTE_GRACE16, AL::division/4);
                              break;
                        case ICON_GRACE32:
                              score()->setGraceNote(ch, pitch(), NOTE_GRACE32, AL::division/8);
                              break;
                        case ICON_SBEAM:
                              score()->undoChangeBeamMode(ch, BEAM_BEGIN);
                              break;
                        case ICON_MBEAM:
                              score()->undoChangeBeamMode(ch, BEAM_MID);
                              break;
                        case ICON_NBEAM:
                              score()->undoChangeBeamMode(ch, BEAM_NO);
                              break;
                        case ICON_BEAM32:
                              score()->undoChangeBeamMode(ch, BEAM_BEGIN32);
                              break;
                        case ICON_BEAM64:
                              score()->undoChangeBeamMode(ch, BEAM_BEGIN64);
                              break;
                        case ICON_AUTOBEAM:
                              score()->undoChangeBeamMode(ch, BEAM_AUTO);
                              break;
                        }
                  }
                  delete e;
                  break;

            case NOTE:
                  {
                  Chord* ch = chord();
                  e->setParent(ch);
                  score()->undoRemoveElement(this);
                  score()->undoAddElement(e);
                  }
                  break;

            case GLISSANDO:
                  {
                  Segment* s = ch->segment();
                  s = s->next1();
                  while (s) {
                        if (s->subtype() == SegChordRest && s->element(track()))
                              break;
                        s = s->next1();
                        }
                  if (s == 0) {
                        printf("no segment for second note of glissando found\n");
                        delete e;
                        return 0;
                        }
                  ChordRest* cr1 = static_cast<ChordRest*>(s->element(track()));
                  if (cr1 == 0 || cr1->type() != CHORD) {
                        printf("no second note for glissando found, track %d\n", track());
                        delete e;
                        return 0;
                        }
                  e->setTrack(track());
                  e->setParent(cr1);
                  score()->undoAddElement(e);
                  score()->setLayout(cr1->measure());
                  }
                  break;

            case TREMOLO:
                  {
                  Tremolo* tremolo = static_cast<Tremolo*>(e);
                  if (tremolo->twoNotes()) {
                        Segment* s = ch->segment()->next();
                        while (s) {
                              if (s->element(track()) && s->element(track())->type() == CHORD)
                                    break;
                              s = s->next();
                              }
                        if (s == 0) {
                              printf("no segment for second note of tremolo found\n");
                              delete e;
                              return 0;
                              }
                        Chord* ch2 = static_cast<Chord*>(s->element(track()));
                        tremolo->setChords(ch, ch2);
                        }
                  e->setParent(ch);
                  e->setTrack(track());
                  if (ch->tremolo())
                        score()->undoRemoveElement(ch->tremolo());
                  score()->undoAddElement(e);
                  }
                  break;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  Direction dir = c->stemDirection();
                  int t         = (staffIdx() * VOICES) + (n->voice() % VOICES);
                  score()->select(0, SELECT_SINGLE, 0);
                  NoteVal nval;
                  nval.pitch = n->pitch();
                  nval.headGroup = n->headGroup();
                  Segment* seg = score()->setNoteRest(chord(), t, nval,
                     score()->inputState().duration().fraction(), dir);
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(t));
                  if (cr)
                        score()->nextInputPos(cr, true);
                  delete e;
                  }
                  break;

            default:
                  return ch->drop(view, p1, p2, e);
            }
      return 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Note::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Note Properties..."));
      a->setData("props");
      if (chord()->tuplet()) {
            QMenu* menuTuplet = popup->addMenu(tr("Tuplet..."));
            a = menuTuplet->addAction(tr("Tuplet Properties..."));
            a->setData("tupletProps");
            a = menuTuplet->addAction(tr("Delete Tuplet"));
            a->setData("tupletDelete");
            }
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Note::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            ChordProperties vp(this);
            int rv = vp.exec();
            if (rv) {
                  foreach(Note* note, score()->selection().noteList()) {
                        Chord* chord = note->chord();
                        if (vp.small() != chord->small())
                              score()->undoChangeChordRestSize(chord, vp.small());
                        if (Spatium(vp.leadingSpace()) != chord->extraLeadingSpace()
                           || Spatium(vp.trailingSpace()) != chord->extraTrailingSpace()) {
                              score()->undoChangeChordRestSpace(chord, Spatium(vp.leadingSpace()),
                                 Spatium(vp.trailingSpace()));
                              }
                        if (vp.noStem() != chord->noStem())
                              score()->undoChangeChordNoStem(chord, vp.noStem());
                        if (vp.getStemDirection() != chord->stemDirection())
                              score()->undo()->push(new SetStemDirection(chord, Direction(vp.getStemDirection())));
                        if (vp.tuning() != note->tuning())
                              score()->undoChangeTuning(note, vp.tuning());
                        if (DirectionH(vp.getUserMirror()) != note->userMirror())
                              score()->undoChangeUserMirror(note, DirectionH(vp.getUserMirror()));
                        if (vp.getHeadType() != note->headType() || vp.getHeadGroup() != note->headGroup())
                              score()->undo()->push(new ChangeNoteHead(note, vp.getHeadGroup(), vp.getHeadType()));
                        }
                  if (veloType() != vp.veloType() || veloOffset() != vp.veloOffset()
                     || onTimeUserOffset() != vp.onTimeUserOffset()
                     || offTimeUserOffset() != vp.offTimeUserOffset()) {
                        score()->undo()->push(new ChangeNoteProperties(this,
                           vp.veloType(), vp.veloOffset(),
                           vp.onTimeUserOffset(),
                           vp.offTimeUserOffset()));
                        }
                  }
            }
      else if (s == "tupletProps") {
            TupletProperties vp(chord()->tuplet());
            if (vp.exec()) {
                  Tuplet* tuplet = chord()->tuplet();
                  int bracketType = vp.bracketType();
                  int numberType  = vp.numberType();
                  if ((bracketType != tuplet->bracketType()) || (numberType != tuplet->numberType()))
                        score()->undo()->push(new ChangeTupletProperties(tuplet, numberType, bracketType));
                  }
            }
      else if (s == "tupletDelete")
            score()->cmdDeleteTuplet(chord()->tuplet(), true);
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Note::layout()
      {
      bool useTablature = staff() && staff()->useTablature();
      if (useTablature) {
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            double mags = magS();
            QFont f(tab->fretFontName());
            int size = lrint(tab->fretFontSize() * DPI / PPI);
            f.setPixelSize(size);
            QFontMetricsF fm(f);
            // when using letters, "+(_fret > 8)" skips 'j'
            QString s = _ghost ? "X" :
                        ( tab->useNumbers() ? QString::number(_fret) : QString('a' + _fret + (_fret > 8)) );
            double w = fm.width(s) * mags;
            _bbox = QRectF(0.0, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags);
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
                        _dots[i]->setTrack(track());  // needed to know the staff it belongs to (and detect tablature)
                        }
                  _dots[i]->layout();
                  }
            else {
                  delete _dots[i];
                  _dots[i] = 0;
                  }
            }
      // with tablature, dots are hidden: do not spend time positioning them!
      if (dots && !useTablature) {
            double _spatium = spatium();
            double d  = point(score()->styleS(ST_dotNoteDistance));
            double dd = point(score()->styleS(ST_dotDotDistance));
            double y  = 0;
            double x  = chord()->dotPosX() - pos().x();

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
      double xp = x();
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
      double yp = y() + system->staff(staffIdx() + chord()->staffMove())->y() + system->y();
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
//      if (!chord())
//            return;
      for (int i = 0; i < chord()->dots(); ++i) {
            if (_dots[i])
                  _dots[i]->setTrack(val);
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Note::toDefault()
      {
      score()->undoChangeChordRestSpace(chord(), Spatium(0.0), Spatium(0.0));
      score()->undoChangeUserOffset(this, QPointF());
      score()->undoChangeUserOffset(chord(), QPointF());
      score()->undo()->push(new SetStemDirection(chord(), AUTO));
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Note::setMag(double val)
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
      assert(val >= 0 && val < HEAD_GROUPS);
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
//   endEdit
//---------------------------------------------------------

void Note::endEdit()
      {
      Chord* ch = chord();
      if (ch->notes().size() == 1) {
            ch->setUserOff(userOff());
            setUserOff(QPointF());
            }
      }

//---------------------------------------------------------
//   updateAccidental
//---------------------------------------------------------

void Note::updateAccidental(char* tversatz)
      {
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
                  Accidental* a = new Accidental(score());
                  a->setParent(this);
                  a->setSubtype(acci);
                  score()->undoAddElement(a);
                  }
            else if (_accidental->subtype() != acci) {
                  Accidental* a = new Accidental(score());
                  a->setParent(this);
                  a->setSubtype(acci);
                  score()->undoChangeElement(_accidental, a);
                  }
            }
      else {
            if (_accidental)
                  score()->undoRemoveElement(_accidental);
            }
      //
      // calculate the real note line depending on clef
      //
      Staff* s = score()->staff(staffIdx() + chord()->staffMove());
      int tick = chord()->tick();
      int clef = s->clefList()->clef(tick);
      _line    = 127 - _line - 82 + clefTable[clef].yOffset;
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
