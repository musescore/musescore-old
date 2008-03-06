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
#include "chordproperties.h"
#include "icons.h"
#include "utils.h"
#include "image.h"

int Note::noteHeads[HEAD_GROUPS][4] = {
      { wholeheadSym,         halfheadSym,         quartheadSym,    brevisheadSym},
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,  wholecrossedheadSym },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,  wholediamondheadSym},
      { wholetriangleheadSym, halftriangleheadSym, triangleheadSym, wholetriangleheadSym},
      { wholediamond2headSym, halfdiamond2headSym, diamond2headSym, wholediamond2headSym },
      { wholeslashheadSym,    halfslashheadSym,   quartslashheadSym, wholeslashheadSym },
      { xcircledheadSym,      xcircledheadSym,    xcircledheadSym,   xcircledheadSym },
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
      _accidental     = 0;
      _mirror         = false;
      _line           = 0;
      _staffMove      = 0;
      _userAccidental = 0;
      _lineOffset     = 0;
      _dots           = 0;
      _tieFor         = 0;
      _tieBack        = 0;
      _tpc            = -1;
      _headGroup      = 0;
      _hidden         = false;
      _head = noteHeads[0][2];
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
      return symbols[_head].width(mag());
      }

//---------------------------------------------------------
//   headHeight
//---------------------------------------------------------

double Note::headHeight() const
      {
      return symbols[_head].height(mag());
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
 This recomputes _pitch and _tpc. If the accidental is
 redundant, it is set as an editorial accidental
 "userAccidental"
*/

void Note::changeAccidental(int accType)
      {
      _userAccidental = 0;
      int acc1  = Accidental::subtype2value(accType);
      int line  = tpc2line(_tpc);
      _tpc      = line2tpc(line, acc1);
      _pitch    = tpc2pitch(_tpc) + (_pitch / 12) * 12;

      // compute the "normal" accidental of this note in
      // measure context:
      Measure* m = chord()->measure();
      int type2  = m->findAccidental(this);

      // if the accidentals differ, this which means acc1 is
      // redundant and is set as an editorial accidental

      if (accType != type2)
            _userAccidental = accType;    // editorial accidental
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* e)
      {
	e->setParent(this);
      switch(e->type()) {
            case TEXT:
            case SYMBOL:
            case IMAGE:
                  e->setMag(mag());
                  _el.append(e);
                  break;
            case TIE:
                  {
                  Tie* tie = (Tie*)e;
	      	tie->setStartNote(this);
                  tie->setTrack(track());
      		setTieFor(tie);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(tie);
                  }
                  break;
            case ACCIDENTAL:
                  _accidental = (Accidental*)e;
                  _accidental->setMag(mag());
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
      if (t && _accidental) {          // dont show prefix of second tied note
            delete _accidental;
            _accidental = 0;
            }
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
                  Tie* tie = (Tie*) e;
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
      double sw = point(score()->style()->stemWidth) * .5 * mag();
      double x  = pos().x();
      double y  = pos().y();

      if (_mirror)
            upFlag = !upFlag;
      qreal yo = _spatium * .2 * mag();
      if (upFlag) {
            x += symbols[_head].width(mag()) - sw;
            y -= yo;
            }
      else {
            x += sw;
            y += yo;
            }
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   setAccidentalSubtype
//---------------------------------------------------------

void Note::setAccidentalSubtype(int pre)
      {
      if (pre && !_tieBack) {
            if (!_accidental) {
                  _accidental = new Accidental(score());
                  add(_accidental);
                  }
            _accidental->setSubtype(pre);
            _accidental->setMag(mag());
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
      DurationType dt;
      headType(ticks, &dt, &_dots);
      setType(dt);
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
      _head = noteHeads[_headGroup][headType];
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(QPainter& p) const
      {
      if (!_hidden)
            symbols[_head].draw(p, mag());

      if (_dots) {
            double y = 0;
            // do not draw dots on line
            if (_line >= 0 && (_line & 1) == 0) {
                  if (chord()->isUp())
                        y = -_spatium *.5 * mag();
                  else
                        y = _spatium * .5 * mag();
                  }
            for (int i = 1; i <= _dots; ++i)
                  symbols[dotSym].draw(p, mag(), symbols[_head].width(mag()) + point(score()->style()->dotNoteDistance) * i, y);
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Note::bbox() const
      {
      return symbols[_head].bbox(mag());
      }

//---------------------------------------------------------
//   isSimple
//---------------------------------------------------------

bool Note::isSimple(Xml& xml) const
      {
      QList<Prop> pl = Element::properties(xml);
      if (_accidental && !_accidental->userOff().isNull())
            return false;
      return (pl.empty() && _el.empty() && _tieFor == 0 && _staffMove == 0
         && _headGroup == 0
         && _userAccidental == 0);
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

            _el.write(xml);
            if (_tieFor)
                  _tieFor->write(xml);
            if (_staffMove)
                  xml.tag("move", _staffMove);
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
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(this);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setTrack(track());
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
                  if (path.endsWith(".svg"))
                        image = new SvgImage(score());
                  else if (path.endsWith(".jpg")
                     || path.endsWith(".png")
                     || path.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->setTrack(track());
                        image->read(e);
                        add(image);
                        }
                  }
            else if (tag == "head")
                  _headGroup = i;
            else if (tag == "userAccidental")
                  _userAccidental = i;
            else if (tag == "Accidental") {
                  _accidental = new Accidental(score());
                  _accidental->read(e);
                  add(_accidental);
                  }
            else if (tag == "move")
                  _staffMove = i;
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
      int key     = staff()->keymap()->key(chord()->tick());
      int npitch = line2pitch(_line, clef, key);
      setPitch(npitch);
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

void ShadowNote::draw(QPainter& p) const
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

bool Note::acceptDrop(Viewer* viewer, const QPointF&, int type, int subtype) const
      {
      if (type == ATTRIBUTE
         || type == TEXT
         || type == ACCIDENTAL
         || type == BREATH
         || type == ARPEGGIO
         || type == NOTEHEAD
         || type == NOTE
         || type == TREMOLO
         || type == IMAGE
         || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_ACCIACCATURA)
         || (noteType() == NOTE_NORMAL && type == ICON && subtype == ICON_APPOGGIATURA)
         || (type == ICON && subtype == ICON_SBEAM)
         || (type == ICON && subtype == ICON_MBEAM)
         || (type == ICON && subtype == ICON_NBEAM)
         || (type == ICON && subtype == ICON_BEAM32)
         || (type == ICON && subtype == ICON_AUTOBEAM)
         || (type == SYMBOL)
         || (type == CLEF)
         ) {
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
      Chord* cr = chord();
      switch(e->type()) {
            case ATTRIBUTE:
                  {
                  NoteAttribute* atr = (NoteAttribute*)e;
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
                        atr->setTrack(track());
                        score()->select(atr, 0, 0);
                        score()->cmdAdd(atr);
                        }
                  return atr;
                  }
            case TEXT:
            case SYMBOL:
            case IMAGE:
                  e->setParent(this);
                  score()->select(e, 0, 0);
                  score()->undoAddElement(e);
                  return e;

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
                  int tick   = cr->tick();
                  b->setTrack(track());
                  Measure* m = cr->segment()->measure();

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
                  a->setParent(cr);
                  a->setHeight(_spatium * 5);   //DEBUG
                  score()->undoAddElement(a);
                  }
                  break;
            case NOTEHEAD:
                  {
                  Symbol* s = (Symbol*)e;
                  int group = _headGroup;
                  switch(s->sym()) {
                        case halfheadSym:         group = 0; break;
                        case halfcrossedheadSym:  group = 1; break;
                        case halfdiamondheadSym:  group = 2; break;
                        case halftriangleheadSym: group = 3; break;
                        case halfdiamond2headSym: group = 4; break;
                        case halfslashheadSym:    group = 5; break;
                        case xcircledheadSym:     group = 6; break;
                        default: printf("unknown note head\n"); break;
                        }
                  delete s;
                  if (group != _headGroup)
                        score()->undoChangeNoteHead(this, group);
                  }
                  break;
            case TREMOLO:
                  {
                  Tremolo* tremolo = (Tremolo*)e;
                  tremolo->setParent(cr);
                  score()->undoAddElement(tremolo);
                  }
                  break;

            case ICON:
                  {
                  switch(e->subtype()) {
                        case ICON_ACCIACCATURA:
                              score()->setGraceNote(cr, pitch(), NOTE_ACCIACCATURA, division/2);
                              break;
                        case ICON_APPOGGIATURA:
                              score()->setGraceNote(cr, pitch(), NOTE_APPOGGIATURA, division/2);
                              break;
                        case ICON_SBEAM:
                              if (!cr->tuplet())
                                    score()->undoChangeBeamMode(cr, BEAM_BEGIN);
                              break;
                        case ICON_MBEAM:
                              if (!cr->tuplet())
                                    score()->undoChangeBeamMode(cr, BEAM_MID);
                              break;
                        case ICON_NBEAM:
                              if (!cr->tuplet())
                                    score()->undoChangeBeamMode(cr, BEAM_NO);
                              break;
                        case ICON_BEAM32:
                              if (!cr->tuplet())
                                    score()->undoChangeBeamMode(cr, BEAM_BEGIN32);
                              break;
                        case ICON_AUTOBEAM:
                              if (!cr->tuplet())
                                    score()->undoChangeBeamMode(cr, BEAM_AUTO);
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

            case CLEF:
                  cr->staff()->changeClef(cr->tick(), e->subtype());
                  break;

            default:
                  printf("note: cannot accept drop %s\n", e->name());
                  delete e;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Note::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addSeparator();
      a->setText(tr("Chord"));
      a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Note::propertyAction(const QString& s)
      {
      if (s == "props") {
            ChordProperties vp;
            vp.setSmall(chord()->small());
            int rv = vp.exec();
            if (rv) {
                  bool val = vp.small();
                  if (val != chord()->small())
                        score()->undoChangeChordRestSize(chord(), val);
                  }
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Note::layout(ScoreLayout* layout)
      {
      if (parent() == 0)
            return;
      setMag(chord()->mag());
      if (_accidental)
            _accidental->setMag(mag());
      foreach(Element* e, _el) {
            e->setMag(mag());
            e->layout(layout);
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
      System* system = chord()->measure()->system();
      double yp = y() + system->staff(staffIdx() + staffMove())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void Note::collectElements(QList<const Element*>& elist) const
      {
      elist.append(this);
      if (_tieFor)
            _tieFor->collectElements(elist);
      foreach(Element* e, _el)
            elist.append(e);
      if (_accidental)
            elist.append(_accidental);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Note::setTrack(int val)
      {
      Element::setTrack(val);
      if (_tieFor) {
            _tieFor->setTrack(val);
            foreach(SlurSegment* seg, *_tieFor->elements())
                  seg->setTrack(val);
            }
      foreach(Element* e, _el)
            e->setTrack(val);
      if (_accidental)
            _accidental->setTrack(val);
      }
