//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Score* s)
   : Element(s)
      {
      dragMode           = false;
      _pitch             = 0;
      _ppitch            = 0;
      _tuning            = 0.0;
      _userAccidental    = ACC_NONE;
      _accidental        = 0;
      _mirror            = false;
      _userMirror        = DH_AUTO;
      _line              = 0;
      _lineOffset        = 0;
      _tieFor            = 0;
      _tieBack           = 0;
      _tpc               = -1;
      _headGroup         = 0;
      _headType          = HEAD_AUTO;

      _hidden            = false;
      _subchannel        = 0;

      _veloType          = AUTO_VAL;
      _velocity          = 80;
      _veloOffset        = 0;

      _onTimeType        = AUTO_VAL;
      _onTimeOffset      = 0;
      _onTimeUserOffset  = 0;

      _offTimeType       = AUTO_VAL;
      _offTimeOffset     = 0;
      _offTimeUserOffset = 0;
      }

Note::Note(const Note& n)
   : Element(n)
      {
      dragMode        = n.dragMode;
      _subchannel     = n._subchannel;
      _pitch          = n._pitch;
      _ppitch         = n._ppitch;
      _tuning         = n._tuning;
      _tpc            = n._tpc;
      _line           = n._line;
      _userAccidental = n._userAccidental;
      _accidental     = 0;
      if (n._accidental)
            add(new Accidental(*(n._accidental)));
      _headGroup      = n._headGroup;
      _headType       = n._headType;
      _mirror         = n._mirror;
      _userMirror     = n._userMirror;

      foreach(Element* e, n._el)
            add(e->clone());

      _tieFor            = 0;
      _tieBack           = 0;

      _lineOffset        = n._lineOffset;
      _hidden            = n._hidden;

      _veloType          = n._veloType;
      _velocity          = n._velocity;
      _veloOffset        = n._veloOffset;

      _onTimeType        = n._onTimeType;
      _onTimeOffset      = n._onTimeOffset;
      _onTimeUserOffset  = n._onTimeUserOffset;

      _offTimeType       = n._offTimeType;
      _offTimeOffset     = n._offTimeOffset;
      _offTimeUserOffset = n._offTimeUserOffset;
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool Note::isMovable() const
      {
      //
      // drumset notes are not movable
      //
      return !staff()->part()->useDrumset();
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
                  pitchOffset = score()->styleB(ST_concertPitch) ? 0 : part->transpose().chromatic;
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
      _userAccidental = 0;
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::~Note()
      {
      delete _accidental;
      delete _tieFor;
      foreach(Element* e, _el)
            delete e;
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

int Note::noteHead() const
      {
      switch(_headType) {
            default:
            case HEAD_AUTO:
                  return noteHeads[chord()->up()][int(_headGroup)][chord()->duration().headType()];
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
      return symbols[noteHead()].width(magS());
      }

//---------------------------------------------------------
//   headHeight
//---------------------------------------------------------

double Note::headHeight() const
      {
      return symbols[noteHead()].height(magS());
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Note::bbox() const
      {
      return symbols[noteHead()].bbox(magS());
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
      setPitch(n);
      if (chord())
            chord()->pitchChanged();
      score()->spell(this);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* e)
      {
	e->setParent(this);
      switch(e->type()) {
            case SYMBOL:
            case IMAGE:
                  {
                  BSymbol* b = static_cast<BSymbol*>(e);
                  foreach(Element* ee, b->getLeafs())
                        ee->setParent(b);
                  _el.append(e);
                  }
                  break;
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
            default:
                  printf("Note::add() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   setTieBack
//---------------------------------------------------------

void Note::setTieBack(Tie* t)
      {
      _tieBack = t;
      if (t && _accidental)         // dont show prefix of second tied note
            score()->undoRemoveElement(_accidental);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Note::remove(Element* e)
      {
      switch(e->type()) {
            case TEXT:
            case SYMBOL:
            case IMAGE:
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

            default:
                  printf("Note::remove() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   stemPos
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
      QPointF off = symbols[noteHead()].attach(magS());
      if (upFlag) {
            pt.rx() += off.x() - sw;
            pt.ry() += off.y();
            }
      else {
            pt.rx() += symbols[noteHead()].width(magS()) - off.x() + sw;
            pt.ry() -= off.y();
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

void Note::draw(QPainter& p) const
      {
      if (!_hidden || !userOff().isNull()) {
            //
            // warn if pitch extends usable range of instrument
            // by coloring the note head
            //
            if (staff() && !selected() && !score()->printing() && preferences.warnPitchRange) {
                  Part* in = staff()->part();
                  int i = ppitch();
                  if (i < in->minPitchP() || i > in->maxPitchP())
                        p.setPen(Qt::red);
                  else if (i < in->minPitchA() || i > in->maxPitchA())
                        p.setPen(Qt::darkYellow);
                  }
            symbols[noteHead()].draw(p, magS());
            }

      if (chord()) {
            double _spatium = spatium();
            int dots = chord()->dots();
            double x = chord()->dotPosX() - ipos().x();
            if (dots) {
                  double d  = point(score()->styleS(ST_dotNoteDistance));
                  double dd = point(score()->styleS(ST_dotDotDistance));
                  double y = 0;

                  // do not draw dots on staff line
                  if ((_line & 1) == 0) {
                        Measure* m = chord()->measure();
                        if (m->mstaff(staffIdx())->hasVoices) {
                              if (voice() == 0 || voice() == 2) {
                                    y = -_spatium *.5;
                                    }
                              else {
                                    y = _spatium *.5;
                                    }
                              }
                        else
                              y = -_spatium *.5;
                        }

                  for (int i = 0; i < dots; ++i)
                        symbols[dotSym].draw(p, magS(), x + d + dd * i, y);
                  }
            }
      }

//---------------------------------------------------------
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
            if (part->transpose().chromatic)
                  transposeInterval(pitch(), tpc(), &rpitch, &rtpc, part->transpose(), true);
            }

      xml.tag("pitch", rpitch);
      xml.tag("tpc", rtpc);

      if (_tuning != 0.0)
            xml.tag("tuning", _tuning);

      if (userAccidental())
            xml.tag("userAccidental", userAccidental() & 0x7fff);
      if (_accidental &&
         (!_accidental->userOff().isNull() || !_accidental->visible()
          || _accidental->hasBracket())
         ) {
            _accidental->write(xml);
            }
      _el.write(xml);
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
            int val = _veloType == USER_VAL ? _velocity : _veloOffset;
            xml.tag("velocity", val);
            }
      if (_onTimeType != AUTO_VAL) {
            xml.valueTypeTag("onTimeType", _onTimeType);
            int val = _onTimeType == USER_VAL ? _onTimeOffset : _onTimeUserOffset;
            xml.tag("onTimeOffset", val);
            }
      if (_offTimeType != AUTO_VAL) {
            xml.valueTypeTag("offTimeType", _offTimeType);
            int val = _offTimeType == USER_VAL ? _offTimeOffset : _offTimeUserOffset;
            xml.tag("offTimeOffset", val);
            }
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
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(e);
                  // DEBUG:   convert to plain
#if 0
                  QString s = f->getText();
                  f->setText(s);
#endif
                  //
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
            else if (tag == "userAccidental")
                  setUserAccidental(i);
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
            else if (tag == "velocity") {
                  if (_veloType == USER_VAL)
                        _velocity = i;
                  else if (_veloType == OFFSET_VAL)
                        _veloOffset = i;
                  // else
                  //      ignore value;
                  }
            else if (tag == "onTimeType")
                  _onTimeType = readValueType(e);
            else if (tag == "onTimeOffset") {
                  if (_onTimeType == USER_VAL)
                        _onTimeOffset = i;
                  else if (_onTimeType == OFFSET_VAL)
                        _onTimeUserOffset = i;
                  }
            else if (tag == "offTimeType")
                  _offTimeType = readValueType(e);
            else if (tag == "offTimeOffset") {
                  if (_offTimeType == USER_VAL)
                        _offTimeOffset = i;
                  else if (_offTimeType == OFFSET_VAL)
                        _offTimeUserOffset = i;
                  }

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
      _lineOffset = lrint(s.y() * 2.0 / spatium());

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
      setLine(_line + _lineOffset);
      _lineOffset  = 0;
      dragMode     = false;
      int staffIdx = chord()->staffIdx() + chord()->staffMove();
      Staff* staff = score()->staff(staffIdx);
      int tick     = chord()->tick();
      int clef     = staff->clef(tick);
      int key      = staff->key(tick).accidentalType();
      int npitch   = line2pitch(_line, clef, key);

      Note* n = this;
      while (n->tieBack())
            n = n->tieBack()->startNote();
      for (; n; n = n->tieFor() ? n->tieFor()->endNote() : 0)
            score()->undoChangePitch(n, npitch, pitch2tpc(npitch, key), 0);

      score()->select(this, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
   : Element(s)
      {
      _line = 1000;
      _headGroup = 0;
      _head      = 2;   // 1/4 note
      }

void ShadowNote::setHeadGroup(int val)
      {
      if (val >= HEAD_GROUPS) {
            printf("wrong head group %d\n", val);
            abort();
            }
      _headGroup = val;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(QPainter& p) const
      {
      if (!visible())
            return;

      QPointF ap(canvasPos());
      QRect r(abbox().toRect());

      p.translate(ap);
      qreal lw = point(score()->styleS(ST_ledgerLineWidth));
      InputState ps = score()->inputState();
      int voice;
      if (ps.drumNote() != -1 && ps.drumset())
            voice = ps.drumset()->voice(ps.drumNote());
      else
            voice = ps.voice();

      QPen pen(preferences.selectColor[voice].light(140));  // was 160
      pen.setWidthF(lw);
      p.setPen(pen);

      symbols[noteHeads[0][_headGroup][_head]].draw(p, magS());

      double ms = spatium();

      double x1 = symbols[noteHeads[0][_headGroup][_head]].width(magS())*.5 - ms;
      double x2 = x1 + 2 * ms;

      ms *= .5;
      if (_line < 100 && _line > -100) {
            for (int i = -2; i >= _line; i -= 2) {
                  double y = ms * (i - _line);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            for (int i = 10; i <= _line; i += 2) {
                  double y = ms * (i - _line);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            }
      p.translate(-ap);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF ShadowNote::bbox() const
      {
      QRectF b = symbols[noteHeads[0][_headGroup][_head]].bbox(magS());
      double _spatium = spatium();
      double x  = b.width()/2 - _spatium;
      double lw = point(score()->styleS(ST_ledgerLineWidth));

      if (_line < 100 && _line > -100) {
            QRectF r(0, -lw/2.0, 2 * _spatium, lw);
            for (int i = -2; i >= _line; i -= 2)
                  b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
            for (int i = 10; i <= _line; i += 2)
                  b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
            }
      return b;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(ScoreView*, const QPointF&, int type, int subtype) const
      {
      if (type == ARTICULATION
         || type == TEXT
         || type == ACCIDENTAL
         || type == BREATH
         || type == ARPEGGIO
         || type == NOTEHEAD
         || type == NOTE
         || type == TREMOLO
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
         || (type == ICON && subtype == ICON_AUTOBEAM)
         || (type == SYMBOL)
         || (type == CLEF)
         || (type == BAR_LINE)
         || (type == GLISSANDO)
         || (type == SLUR)
         || (type == STAFF_TEXT)
         ) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Note::drop(ScoreView* view, const QPointF& p1, const QPointF& p2, Element* e)
      {
      Chord* ch = chord();
      switch(e->type()) {
            case ARTICULATION:
                  {
                  Articulation* atr = static_cast<Articulation*>(e);
                  Articulation* oa = ch->hasArticulation(atr);
                  if (oa) {
                        delete atr;
                        atr = 0;
                        // if attribute is already there, remove
                        // score()->cmdRemove(oa); // unexpected behaviour?
                        score()->select(oa, SELECT_SINGLE, 0);
                        }
                  else {
                        atr->setParent(ch);
                        atr->setTrack(track());
                        score()->select(atr, SELECT_SINGLE, 0);
                        score()->cmdAdd(atr);
                        }
                  return atr;
                  }
            case TEXT:
            case SYMBOL:
            case IMAGE:
                  e->setParent(this);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;

            case SLUR:
                  delete e;
                  view->cmdAddSlur(this, 0);
                  return 0;

            case HARMONY:
                  e->setParent(chord()->measure());
                  e->setTick(chord()->tick());
                  e->setTrack((track() / VOICES) * VOICES);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;
            case LYRICS:
                  e->setParent(chord()->segment());
                  e->setTick(chord()->tick());
                  e->setTrack(chord()->staffIdx() * VOICES);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;

            case ACCIDENTAL:
                  score()->changeAccidental(this, e->subtype());
                  delete e;
                  break;

            case ARPEGGIO:
                  {
                  Arpeggio* a = (Arpeggio*)e;
                  a->setParent(ch);
                  a->setHeight(spatium() * 5);   //DEBUG
                  score()->undoAddElement(a);
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
                  e->setTick(cr1->tick());
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
                        Chord* ch1 = static_cast<Chord*>(s->element(track()));
                        tremolo->setChords(ch, ch1);
                        score()->setLayout(ch1->measure());
                        tremolo->setParent(ch1);
                        }
                  else {
                        e->setParent(ch);
                        score()->setLayout(ch->measure());
                        }
                  score()->undoAddElement(e);
                  }
                  break;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  int headGroup = n->headGroup();
                  Direction dir = c->stemDirection();
                  int t         = (staffIdx() * VOICES) + (n->voice() % VOICES);
                  score()->select(0, SELECT_SINGLE, 0);
                  Segment* seg = score()->setNoteRest(chord(), t, n->pitch(),
                     score()->inputState().duration().fraction(),
                     headGroup, dir);
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
                  if (veloType() != vp.veloType() || velocity() != vp.velo()
                     || veloOffset() != vp.veloOffset()
                     || onTimeType() != vp.onTimeType() || onTimeOffset() != vp.onTimeOffset()
                     || onTimeUserOffset() != vp.onTimeUserOffset()
                     || offTimeType() != vp.offTimeType() || offTimeOffset() != vp.offTimeOffset()
                     || offTimeUserOffset() != vp.offTimeUserOffset()) {
                        score()->undo()->push(new ChangeNoteProperties(this,
                           vp.veloType(), vp.velo(), vp.veloOffset(),
                           vp.onTimeType(), vp.onTimeOffset(), vp.onTimeUserOffset(),
                           vp.offTimeType(), vp.offTimeOffset(), vp.offTimeUserOffset()));
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
      if (parent() == 0)
            return;
      foreach(Element* e, _el) {
            e->setMag(mag());
            e->layout();
            }
      }

//---------------------------------------------------------
//   layout1
//    compute actual accidental and line
//---------------------------------------------------------

void Note::layout1(char* tversatz)
      {
      _line          = tpc2step(_tpc) + (_pitch/12) * 7;
      int tpcPitch   = tpc2pitch(_tpc);
      if (tpcPitch < 0)
            _line += 7;
      else
            _line -= (tpcPitch/12)*7;

      int acci = 0;
      if (_userAccidental)
            acci = _userAccidental;
      else  {
            int accVal = ((_tpc + 1) / 7) - 2;
            acci       = ACC_NONE;
            if ((accVal != tversatz[_line]) || hidden()) {
                  if (_tieBack == 0)
                        tversatz[_line] = accVal;
                  acci = Accidental::value2subtype(accVal);
                  if (acci == ACC_NONE)
                        acci = ACC_NATURAL;
                  }
            }
      if (acci != ACC_NONE && !_tieBack && !_hidden) {
            if (!_accidental)
                  add(new Accidental(score()));
            _accidental->setSubtype(acci & 0x7fff);
            if (acci & 0x8000)
                  _accidental->setHasBracket(true);
            }
      else {
            if (_accidental) {
                  score()->undoRemoveElement(_accidental);
                  // TODO: memory leak, cannot delete because _accidental may be
                  //       referenced by undo/redo history
                  // _accidental = 0;
                  }
            }
      //
      // calculate the real note line depending on clef
      //
      Staff* s     = score()->staff(staffIdx() + chord()->staffMove());
      int tick     = chord()->tick();
      int clef     = s->clefList()->clef(tick);
      _line        = 127 - _line - 82 + clefTable[clef].yOffset;
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
      if (_tieFor)
            _tieFor->scanElements(data, func);
      foreach(Element* e, _el)
            e->scanElements(data, func);
      if (!dragMode && _accidental)
            func(data, _accidental);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Note::setTrack(int val)
      {
      Element::setTrack(val);
      if (_tieFor) {
            _tieFor->setTrack(val);
            foreach(SlurSegment* seg, *_tieFor->slurSegments())
                  seg->setTrack(val);
            }
      foreach(Element* e, _el)
            e->setTrack(val);
      if (_accidental)
            _accidental->setTrack(val);
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Note::toDefault()
      {
      score()->undoChangeChordRestSpace(chord(), Spatium(0.0), Spatium(0.0));
      score()->undoChangeUserOffset(this, QPointF());
      score()->undoChangeUserOffset(chord(), QPointF());
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Note::setMag(double val)
      {
      Element::setMag(val);
      if (_accidental)
            _accidental->setMag(val);
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void Note::setLine(int n)
      {
      _line = n;
      _pos.ry() = _line * spatium() * .5;
      }

//---------------------------------------------------------
//   setAccidentalType
//---------------------------------------------------------

void Note::setAccidentalType(int pre)
      {
      if (pre != ACC_NONE && !_tieBack && !_hidden) {
            if (!_accidental)
                  add(new Accidental(score()));
            _accidental->setSubtype(pre);
            }
      else {
            if (_accidental)
                  score()->undoRemoveElement(_accidental);
            }
      }

//---------------------------------------------------------
//   accidentalType
//---------------------------------------------------------

int Note::accidentalType() const
      {
      if (!_accidental)
            return ACC_NONE;
      return _accidental->subtype();
      }

//---------------------------------------------------------
//   setHeadGroup
//---------------------------------------------------------

void Note::setHeadGroup(int val)
      {
      assert(val >= 0 && val < HEAD_GROUPS);
      _headGroup = val;
      }

