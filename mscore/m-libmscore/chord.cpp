//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chord.cpp 3708 2010-11-16 09:54:31Z wschweer $
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
#include "m-al/xml.h"
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
#include "noteevent.h"
#include "chordline.h"
#include "lyrics.h"
#include "painter.h"

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      _len = 0.0;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(Painter* p) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      p->setLineCap(Qt::RoundCap);
      p->setPenWidth(lw);
      p->drawLine(0.0, 0.0, 0.0, stemLen());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(XmlReader* r)
      {
      qreal val;
      while (r->readElement()) {
            if (r->readReal("userLen", &val))
                  _userLen = Spatium(val);
            else if (!Element::readProperties(r))
                  r->unknown();
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
      qreal w = point(score()->styleS(ST_stemWidth));
      qreal l = _len + point(_userLen);
      return QRectF(-w * .5, 0, w, l).normalized();
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

void StemSlash::draw(Painter* p) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      p->setPenWidth(lw);
      p->drawLine(line.x1(), line.y1(), line.x2(), line.y2());
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void StemSlash::setLine(const QLineF& l)
      {
      line = l;
      qreal w = point(score()->styleS(ST_stemWidth)) * .5;
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
      _stem          = 0;
      _hook          = 0;
      _stemDirection = AUTO;
      _arpeggio      = 0;
      _tremolo       = 0;
      _tremoloChordType = TremoloSingle;
      _glissando     = 0;
      _noteType      = NOTE_NORMAL;
      _stemSlash     = 0;
      _noStem        = false;
      setFlags(ELEMENT_MOVABLE);
      }

Chord::Chord(const Chord& c)
   : ChordRest(c)
      {
      foreach(Note* n, c.notes())
            add(new Note(*n));

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
      _stemDirection    = c._stemDirection;
      _tremoloChordType = TremoloSingle;
      _tremolo          = 0;
      _noteType         = c._noteType;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
      {
      Element::setScore(s);
      foreach(Note* n, notes())
            n->setScore(s);
      if (_stem)
           _stem->setScore(s);
      if (_hook)
            _hook->setScore(s);
      if (_glissando)
            _glissando->setScore(s);
      if (_arpeggio)
            _arpeggio->setScore(s);
      if (_stemSlash)
            _stemSlash->setScore(s);
      }

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
      {
      delete _arpeggio;
      if (_tremolo && _tremolo->chord1() == this)
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
//    return canvas coordinates
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
      switch(e->type()) {
            case NOTE:
                  {
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
                  break;
            case ARTICULATION:
                  articulations.push_back(static_cast<Articulation*>(e));
                  break;
            case ARPEGGIO:
                  _arpeggio = static_cast<Arpeggio*>(e);
                  break;
            case TREMOLO:
                  {
                  Tremolo* tr = static_cast<Tremolo*>(e);
                  if (tr->twoNotes()) {
                        if (!(_tremolo && _tremolo->twoNotes())) {
                              TimeDuration d = durationType();
                              int dots = d.dots();
                              d  = d.shift(-1);
                              d.setDots(dots);
                              if (tr->chord1())
                                    tr->chord1()->setDurationType(d);
                              if (tr->chord2())
                                    tr->chord2()->setDurationType(d);
                              }
                        _tremoloChordType = TremoloFirstNote;
                        tr->chord2()->setTremolo(tr);
                        tr->chord2()->setTremoloChordType(TremoloSecondNote);
                        }
                  else
                        _tremoloChordType = TremoloSingle;
                  _tremolo = tr;
                  }
                  break;
            case GLISSANDO:
                  _glissando = static_cast<Glissando*>(e);
                  break;
            case STEM:
                  _stem = static_cast<Stem*>(e);
                  break;
            case HOOK:
                  _hook = static_cast<Hook*>(e);
                  break;
            case CHORDLINE:
                  _el.append(e);
                  break;
            default:
                  ChordRest::add(e);
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(Element* e)
      {
      switch(e->type()) {
            case NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  if (_notes.removeOne(note)) {
                        if (note->tieFor()) {
                              if (note->tieFor()->endNote())
                                    note->tieFor()->endNote()->setTieBack(0);
                              }
                        }
                  }
                  break;

            case ARTICULATION:
                  if (!articulations.removeOne(static_cast<Articulation*>(e))) {
                        printf("Chord::remove(): articulation not found\n");
                        }
                  break;
            case ARPEGGIO:
                  _arpeggio = 0;
                  break;
            case TREMOLO:
                  {
                  Tremolo* tremolo = static_cast<Tremolo*>(e);
                  if (tremolo->twoNotes()) {
                        TimeDuration d = durationType();
                        int dots = d.dots();
                        d          = d.shift(1);
                        d.setDots(dots);
                        if (tremolo->chord1())
                              tremolo->chord1()->setDurationType(d);
                        if (tremolo->chord2())
                              tremolo->chord2()->setDurationType(d);
                        tremolo->chord2()->setTremolo(0);
                        }
                  _tremolo = 0;
                  }
                  break;
            case GLISSANDO:
                  _glissando = 0;
                  break;
            case STEM:
                  _stem = 0;
                  break;
            case HOOK:
                  _hook = 0;
                  break;
            case CHORDLINE:
                  _el.removeOne(e);
                  break;
            default:
                  ChordRest::remove(e);
                  break;
            }
      }

//---------------------------------------------------------
//   bbox
//    only used for debugging
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

void Chord::addLedgerLine(qreal x, int staffIdx, int line, int lr)
      {
      qreal _spatium = spatium();
      qreal hw       = upNote()->headWidth();
      qreal hw2      = hw * .5;

      qreal y = line * _spatium * .5;

      LedgerLine* h   = new LedgerLine(score());
      h->setTrack(staffIdx * VOICES);
      h->setVisible(!staff()->invisible());

      // ledger lines extend less than half a space on each side
      // of the notehead:
      //
      qreal ll = _notes[0]->headWidth() + _spatium * .95;
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
            if (n->line() >= (line-1) && n->line() <= (line+1) && n->accidental()) {
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

void Chord::addLedgerLines(qreal x, int move)
      {
      int uppos = 1000;
      int ulr   = 0;
      int idx   = staffIdx() + move;
      for (int ni = _notes.size() - 1; ni >= 0; --ni) {
            const Note* note = _notes[ni];
            int l = note->line();
            if (l >= 0)
                  break;
            for (int i = (uppos+1) & ~1; i < l; i += 2)
                  addLedgerLine(x, idx, i, ulr);
            ulr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            uppos = l;
            }
      for (int i = (uppos+1) & ~1; i <= -2; i += 2)
            addLedgerLine(x, idx, i, ulr);

      int downpos = -1000;
      int dlr = 0;
      foreach(const Note* note, _notes) {
            int l = note->line();
            if (l <= 8)
                  break;
            for (int i = downpos & ~1; i > l; i -= 2)
                  addLedgerLine(x, idx, i, dlr);
            dlr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            downpos = l;
            }
      for (int i = downpos & ~1; i >= 10; i -= 2)
            addLedgerLine(x, idx, i, dlr);
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
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(XmlReader* r, const QList<Tuplet*>& tuplets, const QList<Slur*>& slurs)
      {
      Note* note = new Note(score());

      int ptch = -1;
      int ticks = -1;
      int tpc = -1;

      while (r->readAttribute()) {
            if (r->tag() == "pitch")
                  ptch = r->intValue();
            else if (r->tag() == "ticks")
                  ticks = r->intValue();
            else if (r->tag() == "tpc")
                  tpc = r->intValue();
            }

      if (ticks != -1) {
            TimeDuration d;
            d.setVal(ticks);
            setDurationType(d);
            }

      while (r->readElement()) {
            MString8 tag = r->tag();
            QString val;
            int i;

            if (r->readString("StemDirection", &val)) {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (r->readInt("pitch", &i))
                  note->setPitch(i);
            else if (r->readInt("line", &i))
                  note->setLine(i);
            else if (tag == "Tie") {
                  Tie* _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(r);
                  _tieFor->setStartNote(note);
                  note->setTieFor(_tieFor);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(r);
                  f->setParent(this);
                  note->add(f);
                  }
            else if (r->readInt("move", &i))
                  setStaffMove(i);
            else if (!ChordRest::readProperties(r, tuplets, slurs))
                  r->unknown();
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

void Chord::read(XmlReader* r)
      {
      QList<Tuplet*> tl;
      QList<Slur*> sl;
      read(r, tl, sl);
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(XmlReader* r, const QList<Tuplet*>& tuplets, const QList<Slur*>& slurs)
      {
      while (r->readElement()) {
            MString8 tag = r->tag();
            QString val;

            if (tag == "Note") {
                  Note* note = new Note(score());
                  note->setChord(this);
                  note->read(r);
                  add(note);
                  }
            else if (tag == "appoggiatura") {
                  _noteType = NOTE_APPOGGIATURA;
                  r->read();
                  }
            else if (tag == "acciaccatura") {
                  _noteType = NOTE_ACCIACCATURA;
                  r->read();
                  }
            else if (tag == "grace4") {
                  _noteType = NOTE_GRACE4;
                  r->read();
                  }
            else if (tag == "grace16") {
                  _noteType = NOTE_GRACE16;
                  r->read();
                  }
            else if (tag == "grace32") {
                  _noteType = NOTE_GRACE32;
                  r->read();
                  }
            else if (r->readString("StemDirection", &val)) {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(val.toInt());
                  }
            else if (r->readBool("noStem", &_noStem))
                  ;
            else if (tag == "Arpeggio") {
                  _arpeggio = new Arpeggio(score());
                  _arpeggio->setTrack(track());
                  _arpeggio->read(r);
                  _arpeggio->setParent(this);
                  }
            else if (tag == "Glissando") {
                  _glissando = new Glissando(score());
                  _glissando->setTrack(track());
                  _glissando->read(r);
                  _glissando->setParent(this);
                  }
            else if (tag == "Tremolo") {
                  _tremolo = new Tremolo(score());
                  _tremolo->setTrack(track());
                  _tremolo->read(r);
                  _tremolo->setParent(this);
                  }
            else if (tag == "Stem") {
                  _stem = new Stem(score());
                  _stem->read(r);
                  add(_stem);
                  }
            else if (tag == "Events") {
                  while (r->readElement()) {
                        if (r->tag() == "Event") {
                              NoteEvent* ne = new NoteEvent;
                              ne->read(r);
                              _playEvents.append(ne);
                              }
                        else
                              r->unknown();
                        }
                  }
            else if (tag == "ChordLine") {
                  ChordLine* cl = new ChordLine(score());
                  cl->read(r);
                  add(cl);
                  }
            else if (!ChordRest::readProperties(r, tuplets, slurs))
                  r->unknown();
            }
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
      if (_tremolo && (_tremoloChordType != TremoloSecondNote))
            func(data, _tremolo);
      if (_glissando)
            func(data, _glissando);
      foreach(LedgerLine* h, _ledgerLines)
            func(data, h);
      foreach(Note* n, _notes)
            n->scanElements(data, func);
      foreach(Element* e, _el)
            e->scanElements(data, func);
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
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = chord()->measure()->system();
      int st = track() / VOICES;
      qreal yp = y() + system->staff(st)->y() + system->y();
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

void Chord::setMag(qreal val)
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
///   Layout chord stem and hook.
//
//    called before layout spacing of notes
//    set hook if necessary to get right note width for next
//       pass
//---------------------------------------------------------

void Chord::layoutStem1()
      {
      if (staff()->useTablature())
            return;
      int istaff = staffIdx();

      //-----------------------------------------
      //  process stem
      //-----------------------------------------

      bool hasStem = durationType().hasStem() && !(_noStem || measure()->slashStyle(istaff));
      int hookIdx  = hasStem ? durationType().hooks() : 0;

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
///   Layout chord stem and hook.
//---------------------------------------------------------

void Chord::layoutStem()
      {
      if (staff()->useTablature())
            return;
      System* s = segment()->measure()->system();
      if (s == 0)       //DEBUG
            return;

      if (_stem) {
            Spatium stemLen;

            int hookIdx      = durationType().hooks();
            Note* upnote     = upNote();
            Note* downnote   = downNote();
            qreal ul        = upnote->line() * .5;
            qreal dl        = downnote->line() * .5;
            bool shortenStem = score()->styleB(ST_shortenStem);
            Spatium progression(score()->styleS(ST_shortStemProgression));
            Spatium shortest(score()->styleS(ST_shortestStem));

            qreal normalStemLen = small() ? 2.5 : 3.5;
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
                  stemLen *= -1;
                  }
            else {
                  if (up()) {
                        qreal dy  = dl + downnote->stemYoff(true);
                        qreal sel = ul - normalStemLen;

                        if (shortenStem && (sel < 0.0) && (hookIdx == 0 || !downnote->mirror()))
                              sel -= sel  * progression.val();
                        if (sel > 2.0)
                              sel = 2.0;
                        stemLen = Spatium(sel - dy);
                        if (-stemLen < shortest)
                              stemLen = -shortest;
                        }
                  else {
                        qreal uy  = ul + upnote->stemYoff(false);
                        qreal sel = dl + normalStemLen;

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
                  qreal l = spatium() * 1.0;
                  qreal x = _stem->pos().x() + l * .1;
                  qreal y = _stem->pos().y() + point(stemLen) + l * 1.2;
                  qreal h2 = l * .4;
                  qreal w  = upnote->headWidth() * .7;
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
      if (glissando())
            glissando()->layout();
      qreal _spatium = spatium();

      //
      // Experimental:
      //    look for colliding ledger lines
      //

      const qreal minDist = _spatium * .17;

      Segment* s = segment()->prev(SegChordRest | SegGrace);
      if (s) {
            int strack = staffIdx() * VOICES;
            int etrack = strack + VOICES;
            foreach (LedgerLine* h, _ledgerLines) {
                  Spatium len(h->len());
                  qreal y   = h->y();
                  qreal x   = h->x();
                  bool found = false;
                  qreal cx  = h->canvasPos().x();

                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (!e || e->type() != CHORD)
                              continue;
                        foreach (LedgerLine* ll, *static_cast<Chord*>(e)->ledgerLines()) {
                              if (ll->y() != y)
                                    continue;

                              qreal d = cx - ll->canvasPos().x() - (ll->len().val() * _spatium);
                              if (d < minDist) {
                                    //
                                    // the ledger lines overlap
                                    //
                                    qreal shorten = (minDist - d) * .5;
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

      //---------------------------------------------------
      //    layout fingering
      //---------------------------------------------------

      foreach(Note* note, _notes) {
            foreach(Element* e, *note->el()) {
                  if (e->type() == FINGERING) {
                        e->adjustReadPos();
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

      qreal _spatium  = spatium();

      foreach(const LedgerLine* l, _ledgerLines)
            delete l;
      _ledgerLines.clear();

      if (staff() && staff()->useTablature()) {
            foreach(Note* note, _notes) {
                  note->layout();
                  note->setPos(0.0, _spatium * note->string() * 1.5);
                  }
            delete _stem;
            _stem = 0;
            delete _hook;
            _hook = 0;
            delete _stemSlash;
            _stemSlash = 0;
            return;
            }

      if (!segment()) {
            //
            // hack for use in palette
            //
            foreach(Note* note, _notes) {
                  note->layout();
                  qreal x = 0.0;
                  qreal y = note->line() * _spatium * .5;
                  note->setPos(x, y);
                  }
            return;
            }

      Note* upnote     = upNote();
      qreal headWidth = upnote->headWidth();

      //-----------------------------------------
      //  process notes
      //    - position
      //-----------------------------------------

      qreal lx = 0.0;
      qreal stepDistance = _spatium * .5;

      foreach(Note* note, _notes) {
            note->layout();
            qreal x = 0.0;

            bool stemUp = up();
            if (staffMove() < 0)
                  stemUp = false;
            else if (staffMove() > 0)
                  stemUp = true;

            if (note->mirror())
                  x += stemUp ? headWidth : - headWidth;

            note->setPos(x, note->line() * stepDistance);

            note->adjustReadPos();

            Accidental* accidental = note->accidental();
            if (accidental)
                  x = accidental->x() + note->x();
            if (x < lx)
                  lx = x;
            }
      adjustReadPos();

      //---------------------------------------------------
      //    create ledger lines for notes moved to
      //    upper staff
      //---------------------------------------------------

      qreal x  = upnote->pos().x();
      if ((up() && !upnote->mirror()) || (!up() && upnote->mirror()))
            x += headWidth;

      addLedgerLines(x, staffMove());

      foreach(LedgerLine* l, _ledgerLines)
            l->layout();

      renderPlayback();
      qreal lll = -lx;

      if (_arpeggio) {
            qreal headHeight = upnote->headHeight();
            _arpeggio->layout();
            lll += _arpeggio->width() + _spatium * .5;
            qreal y = upNote()->pos().y() - headHeight * .5;
            qreal h = downNote()->pos().y() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(-lll, y);

            // handle the special case of _arpeggio->span() > 1
            // in layoutArpeggio2() after page layout has done so we
            // know the y position of the next staves
            }

      if (_glissando)
            lll += _spatium * .5;

      _dotPosX   = 0.0;
      qreal rrr = 0.0;
      foreach(const Note* note, _notes) {
            qreal lhw = note->headWidth();
            qreal rr = 0.0;
            if (note->mirror()) {
                  if (up())
                        rr = lhw * 2.0;
                  else {
                        if (lhw > lll)
                              lll = lhw;
                        }
                  }
            else
                  rr = lhw;
            if (rr > rrr)
                  rrr = rr;
            qreal xx = note->pos().x() + headWidth;
            if (xx > _dotPosX)
                  _dotPosX = xx;
            }
      qreal rs = _dotPosX;
      if (dots())
            rs += point(score()->styleS(ST_dotNoteDistance)) + dots() * point(score()->styleS(ST_dotDotDistance));

      rrr += _extraTrailingSpace.val() * _spatium;

      if (_hook) {
            _hook->layout();
            if (up())
                  rrr += _hook->width();
            }
      lll += _extraLeadingSpace.val() * _spatium;

      _space.setLw(lll);
      _space.setRw(rrr);

      foreach(Element* e, _el) {
            e->layout();
            if (e->type() == CHORDLINE) {
                  int x = bbox().translated(e->pos()).right();
                  if (x > _space.rw())
                        _space.setRw(x);
                  }
            }
      }

//---------------------------------------------------------
//   renderArpeggio
//---------------------------------------------------------

static void renderArpeggio(QList<Note*> notes, bool up)
      {
      int minLen = 1000*1000;

      foreach(Note* note, notes) {
            int len = note->playTicks();
            if (len < minLen)
                  minLen = len;
            }
      int arpOffset = minLen / notes.size();

      int start, end, step;
      if (up) {
            start = 0;
            end   = notes.size();
            step  = 1;
            }
      else {
            start = notes.size() - 1;
            end   = -1;
            step  = -1;
            }
      int ctick = 0;
      for (int i = start; i != end; i += step) {
            Note* note = notes[i];
            note->setOnTimeOffset(ctick);
            ctick += arpOffset;
            }
      }

//---------------------------------------------------------
//   layoutArpeggio2
//    called after layout of page
//---------------------------------------------------------

void Chord::layoutArpeggio2()
      {
      if (!_arpeggio)
            return;
      Note* upnote      = upNote();
      qreal headHeight = upnote->headHeight();
      qreal y          = upNote()->canvasPos().y() - headHeight * .5;
      int span          = _arpeggio->span();
      Note* dnote       = downNote();
      int btrack        = track() + (span - 1) * VOICES;
      ChordRest* bchord = static_cast<ChordRest*>(segment()->element(btrack));

      if (bchord && bchord->type() == CHORD)
            dnote = static_cast<Chord*>(bchord)->downNote();
      qreal h = dnote->canvasPos().y() - y;
      _arpeggio->setHeight(h);

      QList<Note*> notes;
      int n = _notes.size();
      for (int j = n - 1; j >= 0; --j) {
            Note* note = _notes[j];
            if (note->tieBack())
                  continue;
            notes.prepend(note);
            }

      for (int i = 1; i < span; ++i) {
            ChordRest* c = static_cast<ChordRest*>(segment()->element(track() + i * VOICES));
            if (c && c->type() == CHORD) {
                  QList<Note*> nl = static_cast<Chord*>(c)->notes();
                  int n = nl.size();
                  for (int j = n - 1; j >= 0; --j) {
                        Note* note = nl[j];
                        if (note->tieBack())
                              continue;
                        notes.prepend(note);
                        }
                  }
            }
      bool up = _arpeggio->subtype() != ARP_DOWN;
      renderArpeggio(notes, up);
      }

//---------------------------------------------------------
//   renderPlayback
//---------------------------------------------------------

void Chord::renderPlayback()
      {
      //-----------------------------------------
      //  Layout acciaccatura and appoggiatura
      //-----------------------------------------

      if (segment()->subtype() == SegChordRest) {
            QList<Chord*> gl;
            Segment* s = segment();
            while (s->prev()) {
                  s = s->prev();
                  if (s->subtype() != SegGrace)
                        break;
                  Element* cr = s->element(track());
                  if (cr && cr->type() == CHORD)
                        gl.prepend(static_cast<Chord*>(cr));
                  }
            if (!gl.isEmpty()) {
                  int nticks = 0;
                  foreach(Chord* c, gl)
                        nticks += c->ticks();
                  int t = nticks;
                  if (gl.front()->noteType() == NOTE_ACCIACCATURA)
                        t /= 2;
                  if (t >= (ticks() / 2))
                        t = ticks() / 2;

                  int rt = 0;
                  foreach(Chord* c, gl) {
                        int len   = c->ticks() * t / nticks;
                        int etick = rt + len - c->ticks();
                        foreach(Note* n, c->notes()) {
                              n->setOnTimeOffset(rt);
                              n->setOffTimeOffset(etick);
                              }
                        rt += len;
                        }
                  foreach(Note* n, notes())
                        n->setOnTimeOffset(rt);
                  }
            }
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

