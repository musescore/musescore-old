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
 Implementation of classes Chord, LedgerLine, NoteList Stem ans StemSlash.
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
#include "slur.h"
#include "arpeggio.h"
#include "score.h"
#include "tremolo.h"
#include "glissando.h"
#include "staff.h"
#include "utils.h"
#include "articulation.h"
#include "preferences.h"

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

void Stem::draw(QPainter& p) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      QPen pen(p.pen());
      pen.setWidthF(lw);
//      pen.setCapStyle(Qt::FlatCap);
      pen.setCapStyle(Qt::RoundCap);
      p.setPen(pen);
      p.drawLine(QLineF(0.0, 0.0, 0.0, stemLen()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Stem::write(Xml& xml) const
      {
      xml.stag("Stem");
      Element::writeProperties(xml);
      if (_userLen.val() != 0.0)
            xml.sTag("userLen", _userLen);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "userLen")
                  _userLen = Spatium(e.text().toDouble());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Stem::setVisible(bool f)
      {
      Element::setVisible(f);
      Chord* chord = static_cast<Chord*>(parent());
      if (chord && chord->hook() && chord->hook()->visible() != f)
            chord->hook()->setVisible(f);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Stem::bbox() const
      {
      double w = point(score()->styleS(ST_stemWidth));
      double l = _len + point(_userLen);
      return QRectF(-w * .5, 0, w, l).normalized();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Stem::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      QPointF p(0.0, stemLen());
      grip[0].translate(canvasPos() + p);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Stem::editDrag(int, const QPointF& delta)
      {
      _userLen += Spatium(delta.y() / spatium());
      Chord* c = static_cast<Chord*>(parent());
      if (c->hook())
            c->hook()->move(0.0, delta.y());
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Stem::toDefault()
      {
      _userLen = Spatium(0.0);
      setUserOff(QPointF());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Stem::acceptDrop(ScoreView*, const QPointF&, int type, int subtype) const
      {
      if ((type == TREMOLO) && (subtype <= TREMOLO_3)) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Stem::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      Chord* ch = chord();
      switch(e->type()) {
            case TREMOLO:
                  e->setParent(ch);
                  score()->setLayout(ch->measure());
                  score()->undoAddElement(e);
                  break;
            default:
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   StemSlash
//---------------------------------------------------------

StemSlash::StemSlash(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StemSlash::draw(QPainter& p) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      QPen pen(p.pen());
      pen.setWidthF(lw);
      p.setPen(pen);
      p.drawLine(line);
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void StemSlash::setLine(const QLineF& l)
      {
      line = l;
      double w = point(score()->styleS(ST_stemWidth)) * .5;
      _bbox = QRectF(line.p1(), line.p2()).normalized().adjusted(-w, w, 2.0*w, 2.0*w);
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

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

Chord::Chord(Score* s)
   : ChordRest(s)
      {
      _up            = true;
      _stem          = 0;
      _hook          = 0;
      _stemDirection = AUTO;
      _arpeggio      = 0;
      _tremolo       = 0;
      _glissando     = 0;
      _noteType      = NOTE_NORMAL;
      _stemSlash     = 0;
      _noStem        = false;
      minSpace       = 0.0;
      extraSpace     = 0.0;
      }

Chord::Chord(const Chord& c)
   : ChordRest(c)
      {
      foreach(Note* n, c.notes())
            add(new Note(*n));

      foreach(const LedgerLine* ll, c._ledgerLines) {
            LedgerLine* l = new LedgerLine(*ll);
            l->setParent(this);
            l->setTrack(track());
            _ledgerLines.append(new LedgerLine(*ll));
            }

      _stem          = 0;
      _hook          = 0;
      _glissando     = 0;
      _arpeggio      = 0;
      _stemSlash     = 0;

      _noStem = c._noStem;
      if (c._stem)
            add(new Stem(*(c._stem)));
      if (c._hook)
            add(new Hook(*(c._hook)));
      if (c._glissando)
            add(new Glissando(*(c._glissando)));
      if (c._arpeggio)
            add(new Arpeggio(*(c._arpeggio)));
      if (c._stemSlash) {
            _stemSlash = new StemSlash(*(c._stemSlash));
            _stemSlash->setParent(this);
            _stemSlash->setTrack(track());
            }
      _stemDirection = c._stemDirection;
      _tremolo       = 0;
      _noteType      = c._noteType;
      minSpace       = c.minSpace;
      extraSpace     = c.extraSpace;
      }

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
      {
      delete _arpeggio;
      delete _tremolo;
      delete _glissando;
      delete _stemSlash;
      delete _stem;
      delete _hook;
      foreach(LedgerLine* h, _ledgerLines)
            delete h;
      foreach(Note* n, _notes)
            delete n;
      }

//---------------------------------------------------------
//   setHook
//---------------------------------------------------------

void Chord::setHook(Hook* f)
      {
      delete _hook;
      _hook = f;
      if (_hook) {
            _hook->setParent(this);
            if (_stem)        // should always be true
                  _hook->setVisible(_stem->visible());
            }
      }

//---------------------------------------------------------
//   setStem
//---------------------------------------------------------

void Chord::setStem(Stem* s)
      {
      delete _stem;
      _stem = s;
      if (_stem) {
            _stem->setParent(this);
            _stem->setTrack(track());
            }
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF Chord::stemPos(bool upFlag, bool top) const
      {
      const Note* note = (top ? !upFlag : upFlag) ? downNote() : upNote();
      return note->stemPos(upFlag);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Chord::setSelected(bool f)
      {
      Element::setSelected(f);
      foreach(Note* n, _notes)
            n->setSelected(f);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Chord::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      if (e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            bool found = false;
            for (int idx = 0; idx < _notes.size(); ++idx) {
                  if (note->pitch() < _notes[idx]->pitch()) {
                        _notes.insert(idx, note);
                        found = true;
                        break;
                        }
                  }
            if (!found)
                  _notes.append(note);
            if (note->tieFor()) {
                  if (note->tieFor()->endNote())
                        note->tieFor()->endNote()->setTieBack(note->tieFor());
                  }
            }
      else if (e->type() == ARTICULATION)
            articulations.push_back(static_cast<Articulation*>(e));
      else if (e->type() == ARPEGGIO)
            _arpeggio = static_cast<Arpeggio*>(e);
      else if (e->type() == TREMOLO) {
            Tremolo* tr = static_cast<Tremolo*>(e);
            if (tr->twoNotes()) {
                  Duration d = duration();
                  int dots = d.dots();
                  d  = d.shift(-1);
                  d.setDots(dots);
                  if (tr->chord1())
                        tr->chord1()->setDuration(d);
                  if (tr->chord2())
                        tr->chord2()->setDuration(d);
                  }
            _tremolo = tr;
            }
      else if (e->type() == GLISSANDO)
            _glissando = static_cast<Glissando*>(e);
      else if (e->type() == STEM)
            _stem = static_cast<Stem*>(e);
      else if (e->type() == HOOK)
            _hook = static_cast<Hook*>(e);
      else
            printf("Chord::add: unknown element\n");
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(Element* e)
      {
      if (e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            if (_notes.removeOne(note)) {
                  if (note->tieFor()) {
                        if (note->tieFor()->endNote())
                              note->tieFor()->endNote()->setTieBack(0);
                        }
                  }
            else
                  printf("Chord::remove() note %p not found!\n", e);
            }
      else if (e->type() == ARTICULATION) {
            if (!articulations.removeOne(static_cast<Articulation*>(e)))
                  printf("Chord::remove(): attribute not found\n");
            }
      else if (e->type() == ARPEGGIO)
            _arpeggio = 0;
      else if (e->type() == TREMOLO) {
            Tremolo* tremolo = static_cast<Tremolo*>(e);
            if (tremolo->twoNotes()) {
                  Duration d = duration();
                  int dots = d.dots();
                  d          = d.shift(1);
                  d.setDots(dots);
                  if (tremolo->chord1())
                        tremolo->chord1()->setDuration(d);
                  if (tremolo->chord2())
                        tremolo->chord2()->setDuration(d);
                  }
            _tremolo = 0;
            }
      else if (e->type() == GLISSANDO)
            _glissando = 0;
      else if (e->type() == STEM)
            _stem = 0;
      else if (e->type() == HOOK)
            _hook = 0;
      else
            printf("Chord::remove: unknown element\n");
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Chord::bbox() const
      {
      QRectF _bbox;
      foreach (const Note* n, _notes)
            _bbox |= n->bbox().translated(n->pos());
      foreach(const LedgerLine* l, _ledgerLines)
            _bbox |= l->bbox().translated(l->pos());
      for (ciArticulation i = articulations.begin(); i != articulations.end(); ++i)
            _bbox |= (*i)->bbox().translated((*i)->pos());
      if (_hook)
            _bbox |= _hook->bbox().translated(_hook->pos());
      if (_stem)
            _bbox |= _stem->bbox().translated(_stem->pos());
      if (_arpeggio)
            _bbox |= _arpeggio->bbox().translated(_arpeggio->pos());
      if (_glissando)
            _bbox |= _glissando->bbox().translated(_glissando->pos());
      if (_stemSlash)
            _bbox |= _stemSlash->bbox().translated(_stemSlash->pos());
      if (_tremolo)
            _bbox |= _tremolo->bbox().translated(_tremolo->pos());
      return _bbox;
      }

//---------------------------------------------------------
//   addLedgerLine
///   Add a ledger line to a chord.
///   \arg x          center of note head
///   \arg staffIdx   determines the y origin
///   \arg line       vertical position of line
//---------------------------------------------------------

void Chord::addLedgerLine(double x, int staffIdx, int line, int lr, bool visible)
      {
      double _spatium = spatium();
      double hw       = upNote()->headWidth();
      double hw2      = hw * .5;

      double y = line * _spatium * .5;

      LedgerLine* h   = new LedgerLine(score());
      h->setTrack(staffIdx * VOICES);
      if (staff()->invisible())
            visible = false;
      h->setVisible(visible);

      // ledger lines extend less than half a space on each side
	 	  // of the notehead:
	 	  //
	 	  double ll = _notes[0]->headWidth() + _spatium * .60;
	 	  Spatium len(ll / _spatium);

      if (_noteType != NOTE_NORMAL)
            len *= score()->style(ST_graceNoteMag).toDouble();
      x -= len.val() * _spatium * .5;

      x += (lr & 1) ? -hw2 : hw2;
      if (lr == 3)
            len += Spatium(hw / spatium());

      h->setParent(this);
      h->setTrack(staffIdx * VOICES);

      //
      // Experimental:
      //  shorten ledger line to avoid collisions with accidentals
      //

      foreach(const Note* n, _notes) {
            if (n->line() >= (line-1) && n->line() <= (line+1) && n->accidentalType() != ACC_NONE) {
                  x   += _spatium * .25;
                  len -= Spatium(.25);
                  break;
                  }
            }
      h->setLen(len);
      h->setPos(x, y);
      _ledgerLines.push_back(h);
      }

//---------------------------------------------------------
//   addLedgerLines
//---------------------------------------------------------

void Chord::addLedgerLines(double x, int move)
      {
      int uppos = 1000;
      int ulr   = 0;
      int idx   = staffIdx() + move;
      // make ledger lines invisible if all notes are invisible
      bool visible = false;
      foreach(const Note* note, _notes) {
      if (note->visible()) {
            visible = true;
            break;
	 	             }
            }
      for (int ni = _notes.size() - 1; ni >= 0; --ni) {
            const Note* note = _notes[ni];
            int l = note->line();
            if (l >= 0)
                  break;
            for (int i = (uppos+1) & ~1; i < l; i += 2)
                  addLedgerLine(x, idx, i, ulr, visible);
            ulr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            uppos = l;
            }
      for (int i = (uppos+1) & ~1; i <= -2; i += 2)
            addLedgerLine(x, idx, i, ulr, visible);

      int downpos = -1000;
      int dlr = 0;
      foreach(const Note* note, _notes) {
            int l = note->line();
            if (l <= 8)
                  break;
            for (int i = downpos & ~1; i > l; i -= 2)
                  addLedgerLine(x, idx, i, dlr, visible);
            dlr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            downpos = l;
            }
      for (int i = downpos & ~1; i >= 10; i -= 2)
            addLedgerLine(x, idx, i, dlr, visible);
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
            }
      else if (_noteType != NOTE_NORMAL) {     // grace notes always go up
            _up = true;
            }
      else if (measure()->mstaff(staffIdx())->hasVoices) {
            switch(voice()) {
                  case 0:  _up = (score()->style(ST_stemDir1).toDirection() == UP); break;
                  case 1:  _up = (score()->style(ST_stemDir2).toDirection() == UP); break;
                  case 2:  _up = (score()->style(ST_stemDir3).toDirection() == UP); break;
                  case 3:  _up = (score()->style(ST_stemDir4).toDirection() == UP); break;
                  }
            }
      else if (_notes.size() == 1 || staffMove()) {
            if (staffMove() > 0)
                  _up = true;
            else if (staffMove() < 0)
                  _up = false;
            else
                  _up = upNote()->line() > 4;
            }
      else {
            Note* un = upNote();
            Note* dn = downNote();
            int ud = un->line() - 4;
            int dd = dn->line() - 4;
            if (-ud == dd) {
                  int up = 0;
                  foreach(const Note* n, _notes) {
                        int l = n->line();
                        if (l <= 4)
                              --up;
                        else
                              ++up;
                        }
                  _up = up > 0;
                  }
            else
                  _up = dd > -ud;
            }
      }

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
      {
      Note* note = 0;
      foreach(Note* n, _notes) {
            if (n->selected()) {
                  if (note)
                        return 0;
                  note = n;
                  }
            }
      return note;
      }

//---------------------------------------------------------
//   Chord::write
//---------------------------------------------------------

void Chord::write(Xml& xml, int startTick, int endTick) const
      {
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
      if (_noteType != NOTE_NORMAL) {
            switch(_noteType) {
                  case NOTE_INVALID:
                  case NOTE_NORMAL:
                        break;
                  case NOTE_ACCIACCATURA:
                        xml.tagE("acciaccatura");
                        break;
                  case NOTE_APPOGGIATURA:
                        xml.tagE("appoggiatura");
                        break;
     	            case NOTE_GRACE4:
                        xml.tagE("grace4");
                        break;
                  case NOTE_GRACE16:
                        xml.tagE("grace16");
                        break;
                  case NOTE_GRACE32:
                        xml.tagE("grace32");
                        break;
                  }
            }
      if (_noStem)
            xml.tag("noStem", _noStem);
      else if (_stem) {

            if (!_stem->userOff().isNull() || (_stem->userLen().val() != 0.0) || !_stem->visible() || (_stem->color() != preferences.defaultColor))
                  _stem->write(xml);
            }
      switch(_stemDirection) {
            case UP:   xml.tag("StemDirection", QVariant("up")); break;
            case DOWN: xml.tag("StemDirection", QVariant("down")); break;
            case AUTO: break;
            }
      foreach (const Note* n, _notes)
            n->write(xml, startTick, endTick);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_glissando)
            _glissando->write(xml);
      if (_tremolo)
            _tremolo->write(xml);
      xml.etag();
      xml.curTick = tick() + ticks();
      }

//---------------------------------------------------------
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(QDomElement e, QList<Tuplet*>& tuplets, Measure* measure)
      {
      Note* note = new Note(score());
      int ptch   = e.attribute("pitch", "-1").toInt();
      int ticks  = e.attribute("ticks", "-1").toInt();
      int tpc    = e.attribute("tpc", "-1").toInt();

      if (ticks != -1) {
            Duration d;
            d.setVal(ticks);
            setDuration(d);
            }

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (tag == "pitch")
                  note->setPitch(i);
            else if (tag == "prefix") {
                  printf("read Note:: prefix: TODO\n");
                  }
            else if (tag == "line")
                  note->setLine(i);
            else if (tag == "Tie") {
                  Tie* _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(note);
                  note->setTieFor(_tieFor);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(e);
                  f->setParent(this);
                  note->add(f);
                  }
            else if (tag == "move")
                  setStaffMove(i);
            else if (!ChordRest::readProperties(e, tuplets, measure))
                  domError(e);
            }
      if (ptch != -1) {
            note->setPitch(ptch);
            note->setTpcFromPitch();
            }
      if (tpc != -1)
            note->setTpc(tpc);
      add(note);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Chord::read(QDomElement e)
      {
      QList<Tuplet*> tl;
      read(e, tl, 0);
      if (!duration().isValid())
            convertTicks();
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(QDomElement e, QList<Tuplet*>& tuplets, Measure* measure)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "Note") {
                  Note* note = new Note(score());
                  note->setChord(this);
                  // the note needs to know the properties of the track it belongs to
                  note->setTrack(track());
                  note->read(e);
                  add(note);
                  }
            else if (tag == "appoggiatura")
                  _noteType = NOTE_APPOGGIATURA;
            else if (tag == "acciaccatura")
                  _noteType = NOTE_ACCIACCATURA;
            else if (tag == "grace4")
                  _noteType = NOTE_GRACE4;
            else if (tag == "grace16")
                  _noteType = NOTE_GRACE16;
            else if (tag == "grace32")
                  _noteType = NOTE_GRACE32;
            else if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (tag == "noStem")
                  _noStem = i;
            else if (tag == "Arpeggio") {
                  _arpeggio = new Arpeggio(score());
                  _arpeggio->setTrack(track());
                  _arpeggio->read(e);
                  _arpeggio->setParent(this);
                  }
            else if (tag == "Glissando") {
                  _glissando = new Glissando(score());
                  _glissando->setTrack(track());
                  _glissando->read(e);
                  _glissando->setParent(this);
                  }
            else if (tag == "Tremolo") {
                  _tremolo = new Tremolo(score());
                  _tremolo->setTrack(track());
                  _tremolo->read(e);
                  _tremolo->setParent(this);
                  }
            else if (tag == "tickOffset")       // obsolete
                  ;
            else if (tag == "Stem") {
                  _stem = new Stem(score());
                  _stem->read(e);
                  add(_stem);
                  }
            else if (!ChordRest::readProperties(e, tuplets, measure))
                  domError(e);
            }
      if (!duration().isValid())
            convertTicks();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Chord::dump() const
      {
      printf("Chord tick %d  len %d\n", tick(), ticks());
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Chord::upPos() const
      {
      return upNote()->pos().y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Chord::downPos() const
      {
      return downNote()->pos().y();
      }

//---------------------------------------------------------
//   centerX
//    return x position for attributes
//---------------------------------------------------------

qreal Chord::centerX() const
      {
      const Note* note = up() ? upNote() : downNote();
      qreal x = note->pos().x();
      x += note->headWidth() * .5;
      if (note->mirror()) {
            x += note->headWidth() * (up() ? -1.0 : 1.0);
            }
      return x;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Chord::scanElements(void* data, void (*func)(void*, Element*))
      {
      if (_hook)
            func(data, _hook);
      if (_stem)
            func(data, _stem);
      if (_stemSlash)
            func(data, _stemSlash);
      if (_arpeggio)
            func(data, _arpeggio);
      if (_tremolo)
            func(data, _tremolo);
      if (_glissando)
            func(data, _glissando);

      foreach(LedgerLine* h, _ledgerLines)
            func(data, h);

      foreach(Note* n, _notes)
            n->scanElements(data, func);
      ChordRest::scanElements(data, func);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Chord::setTrack(int val)
      {
      if (_hook)
            _hook->setTrack(val);
      if (_stem)
            _stem->setTrack(val);
      if (_stemSlash)
            _stemSlash->setTrack(val);
      if (_arpeggio)
            _arpeggio->setTrack(val);
      if (_glissando)
            _glissando->setTrack(val);
      if (_tremolo)
            _tremolo->setTrack(val);

      foreach(LedgerLine* h, _ledgerLines)
            h->setTrack(val);

      foreach(Note* n, _notes)
            n->setTrack(val);
      ChordRest::setTrack(val);
      }

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Line(s, false)
      {
      setSelectable(false);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF LedgerLine::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = chord()->measure()->system();
      int st = track() / VOICES;
      double yp = y() + system->staff(st)->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LedgerLine::layout()
      {
      setLineWidth(score()->styleS(ST_ledgerLineWidth));
      Line::layout();
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Chord::setMag(double val)
      {
      Element::setMag(val);
      foreach (LedgerLine* ll, _ledgerLines)
            ll->setMag(val);
      if (_stem)
            _stem->setMag(val);
      if (_hook)
            _hook->setMag(val);
      if (_stemSlash)
            _stemSlash->setMag(val);
      if (_arpeggio)
            _arpeggio->setMag(val);
      if (_tremolo)
            _tremolo->setMag(val);
      if (_glissando)
            _glissando->setMag(val);
      foreach (Note* n, _notes)
            n->setMag(val);
      }

//---------------------------------------------------------
//   layoutStem1
//    called before layout spacing of notes
//    set hook if necessary to get right note width for next
//       pass
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

void Chord::layoutStem1()
      {
      int istaff = staffIdx();

      //-----------------------------------------
      //  process stem
      //-----------------------------------------

      bool hasStem = duration().hasStem() && !(_noStem || measure()->slashStyle(istaff));
      int hookIdx  = hasStem ? duration().hooks() : 0;

      if (hasStem) {
            if (!_stem)
                  setStem(new Stem(score()));
            }
      else
            setStem(0);

      if (hasStem && _noteType == NOTE_ACCIACCATURA) {
            _stemSlash = new StemSlash(score());
            _stemSlash->setMag(mag());
            _stemSlash->setParent(this);
            }
      else {
            delete _stemSlash;
            _stemSlash = 0;
            }

      //-----------------------------------------
      //  process hook
      //-----------------------------------------

      if (hookIdx) {
            if (!up())
                  hookIdx = -hookIdx;
            if (!_hook)
                  setHook(new Hook(score()));
            _hook->setMag(mag());
            _hook->setSubtype(hookIdx);
            }
      else
            setHook(0);
      }

//---------------------------------------------------------
//   layoutStem
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

void Chord::layoutStem()
      {
      System* s = segment()->measure()->system();
      if (s == 0)       //DEBUG
            return;

      if (_stem) {
            Spatium stemLen;

            int hookIdx      = duration().hooks();
            Note* upnote     = upNote();
            Note* downnote   = downNote();
            double ul        = upnote->line() * .5;
            double dl        = downnote->line() * .5;
            bool shortenStem = score()->styleB(ST_shortenStem);
            Spatium progression(score()->styleS(ST_shortStemProgression));
            Spatium shortest(score()->styleS(ST_shortestStem));

            double normalStemLen = small() ? 2.5 : 3.5;
            switch(hookIdx) {
                  case 3: normalStemLen += small() ? .5  : 0.75; break; //32nd notes
                  case 4: normalStemLen += small() ? 1.0 : 1.5;  break; //64th notes
                  //case 5: normalStemLen += small() ? 1.5 : 2.25; break; //128th notes not yet supported in MuseScore
                  }

            if (_noteType != NOTE_NORMAL) {
                  // grace notes stems are not subject to normal
                  // stem rules
                  stemLen = Spatium(qAbs(ul - dl));
                  stemLen += Spatium(normalStemLen * score()->style(ST_graceNoteMag).toDouble());
                  if (up())
                        stemLen *= -1;
                  }
            else {
                  if (up()) {
                        double dy  = dl + downnote->stemYoff(true);
                        double sel = ul - normalStemLen;

                        if (shortenStem && (sel < 0.0) && (hookIdx == 0 || !downnote->mirror()))
                              sel -= sel  * progression.val();
                        if (sel > 2.0)
                              sel = 2.0;
                        stemLen = Spatium(sel - dy);
                        if (-stemLen < shortest)
                              stemLen = -shortest;
                        }
                  else {
                        double uy  = ul + upnote->stemYoff(false);
                        double sel = dl + normalStemLen;

                        if (shortenStem && (sel > 4.0) && (hookIdx == 0 || downnote->mirror()))
                              sel -= (sel - 4.0)  * progression.val();
                        if (sel < 2.0)
                              sel = 2.0;
                        stemLen    = Spatium(sel - uy);
                        if (stemLen < shortest)
                              stemLen = shortest;
                        }
                  }


            QPointF npos(stemPos(_up, false));

            _stem->setLen(point(stemLen));
            _stem->setPos(npos - canvasPos());

            if (_stemSlash) {
                  // TODO: does not work for chords
                  double l = spatium() * 1.0;
                  double x = _stem->pos().x() + l * .1;
                  double y = _stem->pos().y() + point(stemLen);
                  if (up())
                        y += l * 1.2;
                  else
                        y -= l * 1.2;
                  double h2 = l * (up() ? .4 : -.4);
                  double w  = upnote->headWidth() * .7;
                  _stemSlash->setLine(QLineF(QPointF(x + w, y - h2), QPointF(x - w, y + h2)));
                  }

            if (hookIdx) {
                  if (!up())
                        hookIdx = -hookIdx;
                  _hook->setSubtype(hookIdx);
                  qreal lw  = point(score()->styleS(ST_stemWidth)) * .5;
                  QPointF p = npos + QPointF(lw, _stem->stemLen());
                  _hook->setPos(p - canvasPos());
                  }
            else
                  setHook(0);
            }
      else
            setHook(0);

      //-----------------------------------------
      //    process tremolo
      //-----------------------------------------

      if (_tremolo)
            _tremolo->layout();
      }

//---------------------------------------------------------
//   layout2
//    Called after horizontal positions of all elements
//    are fixed.
//---------------------------------------------------------

void Chord::layout2()
      {
      double _spatium = spatium();

      //
      // Experimental:
      //    look for colliding ledger lines
      //

      const double minDist = _spatium * .17;

      Segment* s = segment()->prev();
      if (s && (s->subtype() & (SegChordRest | SegGrace))) {
            int strack = staffIdx() * VOICES;
            int etrack = strack + VOICES;
            foreach (LedgerLine* h, _ledgerLines) {
                  Spatium len(h->len());
                  double y   = h->y();
                  double x   = h->x();
                  bool found = false;
                  double cx  = h->canvasPos().x();

                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (!e || e->type() != CHORD)
                              continue;
                        foreach (LedgerLine* ll, *static_cast<Chord*>(e)->ledgerLines()) {
                              if (ll->y() != y)
                                    continue;

                              double d = cx - ll->canvasPos().x() - (ll->len().val() * _spatium);
                              if (d < minDist) {
                                    //
                                    // the ledger lines overlap
                                    //
                                    double shorten = (minDist - d) * .5;
                                    x   += shorten;
                                    len -= Spatium(shorten / _spatium);
                                    ll->setLen(ll->len() - Spatium(shorten / _spatium));
                                    h->setLen(len);
                                    h->setPos(x, y);
                                    }
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Chord::layout()
      {
      if (_notes.empty())
            return;

      double _spatium  = spatium();

      if (!segment()) {
            //
            // hack for use in palette
            //
            foreach(Note* note, _notes) {
                  note->layout();
                  double x = 0.0;
                  double y = note->line() * _spatium * .5;
                  note->setPos(x, y);
                  }
            return;
            }

      Note* upnote     = upNote();
      double headWidth = upnote->headWidth();

      //-----------------------------------------
      //  process notes
      //    - position
      //-----------------------------------------

      double lx = 0.0;
      _dotPosX  = 0.0;
      foreach(Note* note, _notes) {
            note->layout();

            double x = 0.0;

            bool stemUp = up();
#if 1
            if (staffMove() < 0)
                  stemUp = false;
            else if (staffMove() > 0)
                  stemUp = true;
#endif

            if (note->mirror())
                  x += stemUp ? headWidth : - headWidth;

            note->setPos(x, note->line() * _spatium * .5);
            double xx = x + headWidth;
            if (xx > _dotPosX)
                  _dotPosX = xx;

            Accidental* accidental = note->accidental();
            if (accidental){
                  x = (accidental->x() + accidental->bbox().x()) * mag()+ note->x();
                  }

            if (x < lx)
                  lx = x;
            }

      //-----------------------------------------
      //  process ledger lines
      //-----------------------------------------

      foreach(const LedgerLine* l, _ledgerLines)
            delete l;
      _ledgerLines.clear();

      //---------------------------------------------------
      //    create ledger lines for notes moved to
      //    upper staff
      //---------------------------------------------------

      double x  = upnote->pos().x();
      if ((up() && !upnote->mirror()) || (!up() && upnote->mirror()))
            x += headWidth;

      addLedgerLines(x, staffMove());

      foreach(LedgerLine* l, _ledgerLines)
            l->layout();

      //-----------------------------------------
      //  Fingering
      //-----------------------------------------

#if 0 // TODO
      foreach(Note* note, _notes) {
            QList<Text*>& fingering = note->fingering();
            double x = _spatium * 0.8 + note->headWidth();
            foreach(const Text* f, fingering) {
                  f->setPos(x, 0.0);
                  // TODO: x += _spatium;
                  // if we have two fingerings and move the first,
                  // the second will also change position because their
                  // position in this list changes
                  }
            }
#endif
#if 1
      if (_arpeggio) {
            double headHeight = upnote->headHeight();
            _arpeggio->layout();
            lx -= _arpeggio->width() + _spatium * .5;
            double y = upNote()->pos().y() - headHeight * .5;
            double h = downNote()->pos().y() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(lx, y);
            }
#endif
      extraSpace    = -lx + _extraLeadingSpace.val() * _spatium;
      double mirror = 0.0;
      double hw     = 0.0;

      if (_glissando)
            extraSpace += _spatium * .5;

      foreach(const Note* note, _notes) {
            double lhw = note->headWidth();
            if (note->mirror() && up() && (mirror < lhw))  // note head on the right side of stem
                  mirror = lhw;
            else if (lhw > hw)
                  hw = lhw;
            }
      double rs = _dotPosX;

      if (dots())
            rs += point(score()->styleS(ST_dotNoteDistance)) + dots() * point(score()->styleS(ST_dotDotDistance));

      // minSpace = mirror + hw + _extraTrailingSpace.val() * _spatium;
      minSpace = rs + _extraTrailingSpace.val() * _spatium;

      if (up() && _hook)
            minSpace += _hook->width();
      extraSpace += point(_extraLeadingSpace);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Chord::space() const
      {
      return Space(extraSpace, minSpace);
      }

//---------------------------------------------------------
//   findNote
//---------------------------------------------------------

Note* Chord::findNote(int pitch) const
      {
      foreach(Note* n, _notes) {
            if (n->pitch() == pitch)
                  return n;
            }
      return 0;
      }

//---------------------------------------------------------
//   noteLessThan
//---------------------------------------------------------

static bool noteLessThan(const Note* n1, const Note* n2)
      {
      return n1->pitch() <= n2->pitch();
      }

//---------------------------------------------------------
//   pitchChanged
//---------------------------------------------------------

void Chord::pitchChanged()
      {
      qSort(_notes.begin(), _notes.end(), noteLessThan);
      }

