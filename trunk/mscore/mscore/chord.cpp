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
#include "noteevent.h"
#include "undo.h"
#include "chordline.h"

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(QPainter& p, ScoreView*) const
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
      if ((type == TREMOLO) && (subtype <= TREMOLO_R64)) {
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
                  delete e;
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

void StemSlash::draw(QPainter& p, ScoreView*) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      QPen pen(p.pen());
      pen.setWidthF(lw);
      p.setPen(pen);
      p.drawLine(line);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF StemSlash::bbox() const
      {
      double w = point(score()->styleS(ST_stemWidth)) * .5;
      QRectF r(line.p1().x(), line.p2().y(),
         line.p2().x()-line.p1().x(),
         line.p1().y()-line.p2().y()
         );
      return r.adjusted(-w, -w, 2.0 * w, 2.0 * w);
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
      _glissando     = 0;
      _noteType      = NOTE_NORMAL;
      _stemSlash     = 0;
      _noStem        = false;
      }

Chord::Chord(const Chord& c)
   : ChordRest(c)
      {
      foreach(Note* n, c.notes())
            add(new Note(*n));

#if 0
      foreach(const LedgerLine* ll, c._ledgerLines) {
            LedgerLine* l = new LedgerLine(*ll);
            l->setParent(this);
            l->setTrack(track());
            _ledgerLines.append(new LedgerLine(*ll));
            }
#endif
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
                              Duration d = durationType();
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
                  printf("Chord::add: unknown element\n");
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
                  else
                        printf("Chord::remove() note %p not found!\n", e);
                  }
                  break;

            case ARTICULATION:
                  if (!articulations.removeOne(static_cast<Articulation*>(e)))
                        printf("Chord::remove(): articulation not found\n");
                  break;
            case ARPEGGIO:
                  _arpeggio = 0;
                  break;
            case TREMOLO:
                  {
                  Tremolo* tremolo = static_cast<Tremolo*>(e);
                  if (tremolo->twoNotes()) {
                        Duration d = durationType();
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
                  printf("Chord::remove: unknown element\n");
            }
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

void Chord::addLedgerLine(double x, int staffIdx, int line, int lr)
      {
      double _spatium = spatium();
      double hw       = upNote()->headWidth();
      double hw2      = hw * .5;

      double y = line * _spatium * .5;

      LedgerLine* h   = new LedgerLine(score());
      h->setTrack(staffIdx * VOICES);
      h->setVisible(!staff()->invisible());

      Spatium len(h->len());

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
      if (!_playEvents.isEmpty()) {
            xml.stag("Events");
            foreach(const NoteEvent* e, _playEvents)
                  e->write(xml);
            xml.etag();
            }
      foreach(Element* e, _el)
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(QDomElement e, const QList<Tuplet*>& tuplets, const QList<Slur*>& slurs)
      {
      Note* note = new Note(score());
      int ptch   = e.attribute("pitch", "-1").toInt();
      int ticks  = e.attribute("ticks", "-1").toInt();
      int tpc    = e.attribute("tpc", "-1").toInt();

      if (ticks != -1) {
            Duration d;
            d.setVal(ticks);
            setDurationType(d);
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
            else if (!ChordRest::readProperties(e, tuplets, slurs))
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
      QList<Slur*> sl;
      read(e, tl, sl);
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(QDomElement e, const QList<Tuplet*>& tuplets, const QList<Slur*>& slurs)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "Note") {
                  Note* note = new Note(score());
                  note->setChord(this);
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
            else if (tag == "Events") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "Event") {
                              NoteEvent* ne = new NoteEvent;
                              ne->read(ee);
                              _playEvents.append(ne);
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "ChordLine") {
                  ChordLine* cl = new ChordLine(score());
                  cl->read(e);
                  add(cl);
                  }
            else if (!ChordRest::readProperties(e, tuplets, slurs))
                  domError(e);
            }
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
      setLen(Spatium(2.0));
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
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

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
            double ul        = upnote->line() * .5;
            double dl        = downnote->line() * .5;
            bool shortenStem = score()->styleB(ST_shortenStem);
            Spatium progression(score()->styleS(ST_shortStemProgression));
            Spatium shortest(score()->styleS(ST_shortestStem));

            double normalStemLen = small() ? 2.5 : 3.5;
            if (_noteType != NOTE_NORMAL)
                  normalStemLen *= score()->style(ST_graceNoteMag).toDouble();

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

            QPointF npos(stemPos(_up, false));

            _stem->setLen(point(stemLen));
            _stem->setPos(npos - canvasPos());

            if (_stemSlash) {
                  // TODO: does not work for chords
                  double l = spatium() * 1.0;
                  double x = _stem->pos().x() + l * .1;
                  double y = _stem->pos().y() + point(stemLen) + l * 1.2;
                  double h2 = l * .4;
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
      foreach(LedgerLine* h, _ledgerLines) {
            //
            // Experimental:
            //    look for colliding ledger lines
            //

            double y = h->y();
            double x = h->x();
            Spatium len(h->len());

            double minDist = _spatium * .2;
            bool found = false;
            double cx  = x + canvasPos().x();
            Segment* s = segment()->prev();
            if (s && s->subtype() == SegChordRest) {
                  int strack = staffIdx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (s->element(track)) {
                              Element* e = s->element(track);
                              if (e->type() == CHORD) {
                                    Chord* ch = static_cast<Chord*>(e);
                                    foreach(LedgerLine* ll, *ch->ledgerLines()) {
                                          if (ll->y() == y) {
                                                double d = cx - (ll->canvasPos().x() + ll->len().val()*_spatium) - minDist;
                                                if (d < 0.0) {
                                                      double shorten = -d;
                                                      x   += shorten;
                                                      len -= Spatium(shorten / _spatium);
                                                      ll->setLen(ll->len() - Spatium(shorten / _spatium));
                                                      }
                                                found = true;
                                                break;
                                                }
                                          }
                                    }
                              }
                        if (found)
                              break;
                        }
                  }
            if (found) {
                  h->setLen(len);
                  h->setPos(x, y);
                  }
            }

      //---------------------------------------------------
      //    layout fingering
      //---------------------------------------------------

      foreach(Note* note, _notes) {
            foreach(Element* e, *note->el()) {
                  if ((e->type() == TEXT && e->subtype() == TEXT_FINGERING)
                     || (e->type() == FINGERING)) {
                        if (_notes.size() > 1) {
                              }
                        else {
                              double x = note->headWidth() * .5;
                              x -= e->width() * .5;
                              double y;
                              if (up())
                                    y = _spatium * .4;     // below
                              else
                                    y = -_spatium * 2.4;
                              e->setPos(x, y);
                              }
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

      double extraSpace, minSpace;

      double _spatium  = spatium();

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
      double stepDistance = _spatium * .5;

      foreach(Note* note, _notes) {
            note->layout();

            double x = 0.0;

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
                  x = (accidental->x() + accidental->bbox().x()) * mag();
            if (x < lx)
                  lx = x;
            }

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

      renderPlayback();

      if (_arpeggio) {
            double headHeight = upnote->headHeight();
            _arpeggio->layout();
            lx -= _arpeggio->width() + _spatium * .5;
            double y = upNote()->pos().y() - headHeight * .5;
            double h = downNote()->pos().y() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(lx, y);

            // handle the special case of _arpeggio->span() > 1
            // in layoutArpeggio2() after page layout has done so we
            // know the y position of the next staves
            }

      extraSpace    = -lx + _extraLeadingSpace.val() * _spatium;
      double mirror = 0.0;
      double hw     = 0.0;

      if (_glissando)
            extraSpace += _spatium * .5;

      _dotPosX = 0.0;
      foreach(const Note* note, _notes) {
            double lhw = note->headWidth();
            if (note->mirror() && up() && (mirror < lhw))  // note head on the right side of stem
                  mirror = lhw;
            else if (lhw > hw)
                  hw = lhw;
            double xx = note->pos().x() + headWidth;
            if (xx > _dotPosX)
                  _dotPosX = xx;
            }
      double rs = _dotPosX;
      if (dots())
            rs += point(score()->styleS(ST_dotNoteDistance)) + dots() * point(score()->styleS(ST_dotDotDistance));

      // minSpace = mirror + hw + _extraTrailingSpace.val() * _spatium;
      minSpace = rs + _extraTrailingSpace.val() * _spatium;

      if (up() && _hook)
            minSpace += _hook->width();
      extraSpace += point(_extraLeadingSpace);

      _space.setLw(extraSpace);
      _space.setRw(minSpace);
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
      double headHeight = upnote->headHeight();
      double y          = upNote()->canvasPos().y() - headHeight * .5;
      int span          = _arpeggio->span();
      Note* dnote       = downNote();
      int btrack        = track() + (span - 1) * VOICES;
      ChordRest* bchord = static_cast<ChordRest*>(segment()->element(btrack));

      if (bchord && bchord->type() == CHORD)
            dnote = static_cast<Chord*>(bchord)->downNote();
      double h = dnote->canvasPos().y() - y;
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

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Chord::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
//      Measure* m  = measure();
      switch (e->type()) {
            case ARTICULATION:
                  {
                  Articulation* atr = static_cast<Articulation*>(e);
                  Articulation* oa = hasArticulation(atr);
                  if (oa) {
                        delete atr;
                        atr = 0;
                        // if attribute is already there, remove
                        // score()->cmdRemove(oa); // unexpected behaviour?
                        score()->select(oa, SELECT_SINGLE, 0);
                        }
                  else {
                        atr->setParent(this);
                        atr->setTrack(track());
                        score()->select(atr, SELECT_SINGLE, 0);
                        score()->undoAddElement(atr);
                        }
                  int clef = staff()->clef(segment()->tick());

                  if (atr->subtype() == MordentSym) {
                        QList<NoteEvent*> events;
                        int pitch = upNote()->ppitch();
                        int pitch2 = diatonicUpDown(clef, pitch, -1);
                        events.append(new NoteEvent(0, 0, 128));
                        events.append(new NoteEvent(pitch2 - pitch, 128, 128));
                        events.append(new NoteEvent(0, 256, 744));
                        score()->undo()->push(new ChangeNoteEvents(this, events));
                        }
                  else if (atr->subtype() == PrallSym) {
                        QList<NoteEvent*> events;
                        int pitch = upNote()->ppitch();
                        int pitch2 = diatonicUpDown(clef, pitch, 1);
                        events.append(new NoteEvent(0, 0, 128));
                        events.append(new NoteEvent(pitch2 - pitch, 128, 128));
                        events.append(new NoteEvent(0, 256, 744));
                        score()->undo()->push(new ChangeNoteEvents(this, events));
                        }
                  return atr;
                  }
            case TEMPO_TEXT:
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  break;
            case CHORDLINE:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  break;
            default:
                  delete e;
                  return 0;
            }
      return 0;
      }

